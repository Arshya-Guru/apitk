#ifndef LABELPAINTINTERACTIONMODE_H
#define LABELPAINTINTERACTIONMODE_H

#include "PaintbrushInteractionMode.h"

/**
 * Interaction mode for the Label Paint tool. Identical to the paintbrush
 * interaction except that the right mouse button is not handled (events fall
 * through to the crosshairs/zoom behavior).
 */
class LabelPaintInteractionMode : public PaintbrushInteractionMode
{
  Q_OBJECT

public:
  LabelPaintInteractionMode(QWidget *parent, QWidget *canvasWidget);
  ~LabelPaintInteractionMode();

  virtual void mousePressEvent(QMouseEvent *ev) override;
};

#endif // LABELPAINTINTERACTIONMODE_H
