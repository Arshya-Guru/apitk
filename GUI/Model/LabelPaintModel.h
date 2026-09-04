#ifndef LABELPAINTMODEL_H
#define LABELPAINTMODEL_H

#include "PaintbrushModel.h"

/**
 * \class LabelPaintModel
 * \brief Connected-component relabeling brush ("Label Paint" mode).
 *
 * This model reuses the paintbrush footprint, cursor tracking and outline
 * rendering from PaintbrushModel, but replaces the painting behavior. Every
 * voxel under the brush whose label passes the current "paint over" filter
 * acts as a seed: the entire 3D connected component of that label is
 * relabeled with the active drawing label. Right-click (reverse mode) does
 * nothing. Undo/redo works through the standard delta mechanism.
 */
class LabelPaintModel : public PaintbrushModel
{
public:
  irisITKObjectMacro(LabelPaintModel, PaintbrushModel)

  // The effective brush settings come from the label paint settings in
  // GlobalState rather than the paintbrush settings
  virtual PaintbrushSettings GetEffectivePaintbrushSettings() override;

protected:
  LabelPaintModel();
  virtual ~LabelPaintModel();

  virtual bool ApplyBrush(bool reverse_mode, bool dragging, bool release) override;
  virtual void CommitDrawing() override;

  // Whether a label is eligible to be used as a seed under the draw-over filter
  bool IsSeedLabel(LabelType label, LabelType drawing_color, const DrawOverFilter &drawover);

  // Whether anything was changed since the last commit
  bool m_StrokeDirty;
};

#endif // LABELPAINTMODEL_H
