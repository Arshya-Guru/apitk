#include "LabelPaintInteractionMode.h"
#include "PaintbrushModel.h"
#include "QtWarningDialog.h"

LabelPaintInteractionMode::LabelPaintInteractionMode(QWidget *parent, QWidget *canvasWidget)
  : PaintbrushInteractionMode(parent, canvasWidget)
{
}

LabelPaintInteractionMode::~LabelPaintInteractionMode() {}

void
LabelPaintInteractionMode::mousePressEvent(QMouseEvent *ev)
{
  // Only the left button does anything in label paint mode
  if (ev->button() != Qt::LeftButton)
    return;

  try
  {
    if (m_Model->ProcessPushEvent(m_XSlice, this->m_LastPressLayoutCell, false))
      ev->accept();
  }
  catch (IRISException &exc)
  {
    QtWarningDialog::show({ IRISWarning(exc.what()) });
  }
}
