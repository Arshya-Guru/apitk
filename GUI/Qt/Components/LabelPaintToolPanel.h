#ifndef LABELPAINTTOOLPANEL_H
#define LABELPAINTTOOLPANEL_H

#include <QWidget>
#include "SNAPComponent.h"

class LabelPaintSettingsModel;

namespace Ui {
class LabelPaintToolPanel;
}

class LabelPaintToolPanel : public SNAPComponent
{
  Q_OBJECT

public:
  explicit LabelPaintToolPanel(QWidget *parent = 0);
  ~LabelPaintToolPanel();

  void SetModel(LabelPaintSettingsModel *model);

private slots:
  void on_actionBrushStyle_triggered();

private:
  Ui::LabelPaintToolPanel *ui;
  LabelPaintSettingsModel *m_Model;
};

#endif // LABELPAINTTOOLPANEL_H
