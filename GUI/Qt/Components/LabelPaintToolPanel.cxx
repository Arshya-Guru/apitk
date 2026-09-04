#include "LabelPaintToolPanel.h"
#include "ui_LabelPaintToolPanel.h"

#include "LabelPaintSettingsModel.h"
#include "SNAPQtCommon.h"
#include "QtRadioButtonCoupling.h"
#include "QtCheckBoxCoupling.h"
#include "QtSpinBoxCoupling.h"
#include "QtSliderCoupling.h"

LabelPaintToolPanel::LabelPaintToolPanel(QWidget *parent)
  : SNAPComponent(parent)
  , ui(new Ui::LabelPaintToolPanel)
{
  ui->setupUi(this);

  m_Model = nullptr;

  // Adjust the shortcuts for increase/decrease behavior
  ui->actionBrushIncrease->setShortcuts(ui->actionBrushIncrease->shortcuts() << QKeySequence('='));
  ui->actionBrushDecrease->setShortcuts(ui->actionBrushDecrease->shortcuts() << QKeySequence('_'));

  addAction(ui->actionBrushIncrease);
  addAction(ui->actionBrushDecrease);
  addAction(ui->actionBrushStyle);
}

LabelPaintToolPanel::~LabelPaintToolPanel()
{
  delete ui;
}

void
LabelPaintToolPanel::SetModel(LabelPaintSettingsModel *model)
{
  m_Model = model;

  // Couple the shape radio buttons
  std::map<PaintbrushShape, QAbstractButton *> rmap_shape{ { PAINTBRUSH_RECTANGULAR, ui->btnSquare },
                                                           { PAINTBRUSH_ROUND, ui->btnRound } };
  makeRadioGroupCoupling(ui->grpBrushShape, rmap_shape, m_Model->GetPaintbrushShapeModel());

  // Couple the other controls
  makeCoupling(ui->chkVolumetric, model->GetVolumetricBrushModel());
  makeCoupling(ui->chkIsotropic, model->GetIsotropicBrushModel());
  makeCoupling(ui->chkChase, model->GetChaseCursorModel());
  makeCoupling(ui->chkFullyConnected, model->GetFullyConnectedModel());

  makeCoupling(ui->inBrushSizeSlider, model->GetBrushSizeModel());
  makeCoupling(ui->inBrushSizeSpinbox, model->GetBrushSizeModel());

  activateOnFlag(ui->chkVolumetric, m_Model, PaintbrushSettingsModel::UIF_VOLUMETRIC_OK);
}

void
LabelPaintToolPanel::on_actionBrushStyle_triggered()
{
  if (ui->btnSquare->isChecked())
    ui->btnRound->setChecked(true);
  else
    ui->btnSquare->setChecked(true);
}
