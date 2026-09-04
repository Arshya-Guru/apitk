#include "LabelPaintModel.h"
#include "GenericSliceModel.h"
#include "GlobalState.h"
#include "GlobalUIModel.h"
#include "IRISApplication.h"
#include "GenericImageData.h"
#include "LabelImageWrapper.h"
#include "UndoDataManager.h"
#include "ImageCoordinateTransform.h"

#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <cstdint>
#include <algorithm>
#include <vector>

LabelPaintModel::LabelPaintModel()
{
  m_StrokeDirty = false;
}

LabelPaintModel::~LabelPaintModel() {}

PaintbrushSettings
LabelPaintModel::GetEffectivePaintbrushSettings()
{
  PaintbrushSettings pbs = m_Parent->GetDriver()->GetGlobalState()->GetLabelPaintSettings().brush;
  pbs.smart_mode = PAINTBRUSH_MANUAL;
  return pbs;
}

bool
LabelPaintModel::IsSeedLabel(LabelType label, LabelType drawing_color, const DrawOverFilter &drawover)
{
  // Relabeling a component with its own label is a no-op
  if (label == drawing_color)
    return false;

  switch (drawover.CoverageMode)
  {
    case PAINT_OVER_ONE:
      // The only way to select the background (clear label) as a component is
      // to explicitly pick it as the paint-over label
      return label == drawover.DrawOverLabel;
    case PAINT_OVER_VISIBLE:
    case PAINT_OVER_ALL:
    default:
      // Never treat the background as a connected component: that would
      // relabel the whole image with a single click
      return label != 0;
  }
}

bool
LabelPaintModel::ApplyBrush(bool reverse_mode, bool dragging, bool release)
{
  // Right-click has no behavior in this mode
  if (reverse_mode)
    return false;

  // Get the global objects
  IRISApplication *driver = m_Parent->GetDriver();
  GlobalState     *gs = driver->GetGlobalState();

  // Get the segmentation image
  LabelImageWrapper *seg = driver->GetSelectedSegmentationLayer();
  if (!seg)
    return false;

  typedef LabelImageWrapper::ImageType ImageType;
  typedef ImageType::IndexType         IndexType;
  typedef ImageType::RegionType        RegionType;

  ImageType *img = seg->GetModifiableImage();

  // Paint properties
  LabelType          drawing_color = gs->GetDrawingColorLabel();
  DrawOverFilter     drawover = gs->GetDrawOverFilter();
  LabelPaintSettings lps = gs->GetLabelPaintSettings();
  PaintbrushSettings pbs = GetEffectivePaintbrushSettings();

  // Define the brush footprint region, exactly as the paintbrush does
  RegionType xTestRegion;
  for (unsigned int i = 0; i < 3; i++)
  {
    if (i != seg->GetDisplaySliceImageAxis(m_Parent->GetId()) || pbs.volumetric)
    {
      xTestRegion.SetIndex(i, (long)(m_MousePosition(i) - pbs.radius));
      xTestRegion.SetSize(i, (long)(2 * pbs.radius + 1));
    }
    else
    {
      xTestRegion.SetIndex(i, m_MousePosition(i));
      xTestRegion.SetSize(i, 1);
    }
  }

  // Crop the region by the buffered region
  const RegionType &bufRegion = img->GetBufferedRegion();
  if (!xTestRegion.Crop(bufRegion))
    return false;

  // Shift vector (different depending on whether the brush has odd/even diameter)
  Vector3d offset = ComputeOffset();

  // Collect the seed voxels: voxels inside the brush whose label is eligible
  std::vector<IndexType> seeds;
  itk::ImageRegionConstIteratorWithIndex<ImageType> it(img, xTestRegion);
  for (; !it.IsAtEnd(); ++it)
  {
    IndexType idx = it.GetIndex();

    Vector3d xDelta = offset + to_double(idx) - to_double(m_MousePosition);
    Vector3d xDeltaSliceSpace =
      to_double(m_Parent->GetImageToDisplayTransform()->TransformVector(xDelta));

    if (!TestInside(xDeltaSliceSpace, pbs))
      continue;

    if (IsSeedLabel(it.Get(), drawing_color, drawover))
      seeds.push_back(idx);
  }

  if (seeds.empty())
    return false;

  // Geometry of the buffered region. The label image is run-length encoded,
  // so voxels are accessed through GetPixel/SetPixel rather than a raw buffer.
  // Linear offsets (x + y*W + z*W*H relative to the buffered region start) are
  // used to key the visited bitmap and the change list.
  const IndexType bufStart = bufRegion.GetIndex();
  const long   W = (long)bufRegion.GetSize(0);
  const long   H = (long)bufRegion.GetSize(1);
  const long   D = (long)bufRegion.GetSize(2);
  const size_t strideY = (size_t)W;
  const size_t strideZ = (size_t)W * (size_t)H;
  const size_t nVoxels = strideZ * (size_t)D;

  // Visited bitmap over the whole image (1 bit per voxel)
  std::vector<uint64_t> visited((nVoxels + 63) / 64, 0);
  auto isVisited = [&visited](size_t o) { return (visited[o >> 6] >> (o & 63)) & 1; };
  auto markVisited = [&visited](size_t o) { visited[o >> 6] |= (uint64_t(1) << (o & 63)); };

  // Neighborhood offsets (6- or 26-connectivity)
  struct NbrOffset { int dx, dy, dz; };
  std::vector<NbrOffset> nbrs;
  if (lps.fully_connected)
  {
    for (int dz = -1; dz <= 1; dz++)
      for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++)
          if (dx || dy || dz)
            nbrs.push_back({ dx, dy, dz });
  }
  else
  {
    nbrs = { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };
  }

  // Record of voxels to change (linear offset, old label) and their bounding box
  std::vector<std::pair<size_t, LabelType>> changed;
  long bbmin[3] = { W, H, D }, bbmax[3] = { -1, -1, -1 };

  std::vector<size_t> stack;
  IndexType           q;

  for (const IndexType &seed : seeds)
  {
    long   sx = seed[0] - bufStart[0], sy = seed[1] - bufStart[1], sz = seed[2] - bufStart[2];
    size_t soff = (size_t)sx + sy * strideY + sz * strideZ;

    // The seed may belong to a component already collected by an earlier seed
    if (isVisited(soff))
      continue;

    // Label of the component to collect (seeds were already checked for eligibility)
    LabelType L = img->GetPixel(seed);

    // Flood fill (read-only pass) the connected component of label L
    markVisited(soff);
    changed.push_back(std::make_pair(soff, L));
    stack.push_back(soff);

    while (!stack.empty())
    {
      size_t o = stack.back();
      stack.pop_back();

      long z = (long)(o / strideZ);
      long r = (long)(o % strideZ);
      long y = r / W;
      long x = r % W;

      if (x < bbmin[0]) bbmin[0] = x;
      if (y < bbmin[1]) bbmin[1] = y;
      if (z < bbmin[2]) bbmin[2] = z;
      if (x > bbmax[0]) bbmax[0] = x;
      if (y > bbmax[1]) bbmax[1] = y;
      if (z > bbmax[2]) bbmax[2] = z;

      for (const NbrOffset &n : nbrs)
      {
        long nx = x + n.dx, ny = y + n.dy, nz = z + n.dz;
        if (nx < 0 || ny < 0 || nz < 0 || nx >= W || ny >= H || nz >= D)
          continue;
        size_t no = (size_t)nx + ny * strideY + nz * strideZ;
        if (isVisited(no))
          continue;
        q[0] = nx + bufStart[0];
        q[1] = ny + bufStart[1];
        q[2] = nz + bufStart[2];
        if (img->GetPixel(q) == L)
        {
          markVisited(no);
          changed.push_back(std::make_pair(no, L));
          stack.push_back(no);
        }
      }
    }
  }

  if (changed.empty())
    return false;

  // Apply the changes and build the undo delta over the bounding box of the
  // changes. Both the delta encoding and the ITK region iterator walk the
  // region in raster order, which matches increasing linear offset, so sort
  // the change list by offset and walk everything together.
  std::sort(changed.begin(), changed.end());

  RegionType deltaRegion;
  for (unsigned int i = 0; i < 3; i++)
  {
    deltaRegion.SetIndex(i, bufStart[i] + bbmin[i]);
    deltaRegion.SetSize(i, bbmax[i] - bbmin[i] + 1);
  }

  typedef UndoDataManager<LabelType>::Delta UndoDelta;
  UndoDelta *delta = new UndoDelta();
  delta->SetRegion(deltaRegion);

  size_t k = 0;
  itk::ImageRegionIterator<ImageType> wit(img, deltaRegion);
  for (long z = bbmin[2]; z <= bbmax[2]; z++)
  {
    for (long y = bbmin[1]; y <= bbmax[1]; y++)
    {
      size_t rowOff = (size_t)bbmin[0] + y * strideY + z * strideZ;
      for (long x = bbmin[0]; x <= bbmax[0]; x++, rowOff++, ++wit)
      {
        if (k < changed.size() && changed[k].first == rowOff)
        {
          wit.Set(drawing_color);
          delta->Encode((LabelType)(drawing_color - changed[k].second));
          k++;
        }
        else
        {
          delta->Encode((LabelType)0);
        }
      }
    }
  }
  delta->FinishEncoding();

  // Mark the image as modified and stage the delta for undo
  seg->PixelsModified();
  seg->StoreIntermediateUndoDelta(delta);
  m_StrokeDirty = true;

  return true;
}

void
LabelPaintModel::CommitDrawing()
{
  IRISApplication *driver = m_Parent->GetDriver();
  LabelImageWrapper *seg = driver->GetSelectedSegmentationLayer();

  if (!m_StrokeDirty || !seg)
    return;

  m_StrokeDirty = false;

  // Commit the staged deltas as a single undo point
  seg->StoreUndoPoint("Label paint");
  driver->RecordCurrentLabelUse();

  // Notify the rest of the application that the segmentation changed
  driver->InvokeEvent(SegmentationChangeEvent());
}
