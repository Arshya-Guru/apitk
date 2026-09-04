#ifndef LABELPAINTSETTINGSMODEL_H
#define LABELPAINTSETTINGSMODEL_H

#include "PaintbrushSettingsModel.h"

/**
 * \class LabelPaintSettingsModel
 * \brief Settings model for the Label Paint (connected component relabel) tool.
 *
 * Reuses the brush shape / size / 3D / isotropic / chase property models from
 * PaintbrushSettingsModel, but reads and writes the LabelPaintSettings in
 * GlobalState instead of the PaintbrushSettings. Adds a connectivity toggle.
 */
class LabelPaintSettingsModel : public PaintbrushSettingsModel
{
public:
  irisITKObjectMacro(LabelPaintSettingsModel, PaintbrushSettingsModel)

  // Whether components are 26-connected (true) or 6-connected (false)
  irisSimplePropertyAccessMacro(FullyConnected, bool)

protected:
  LabelPaintSettingsModel();
  virtual ~LabelPaintSettingsModel();

  virtual PaintbrushSettings GetPaintbrushSettings() override;
  virtual void SetPaintbrushSettings(PaintbrushSettings ps) override;

  SmartPtr<AbstractSimpleBooleanProperty> m_FullyConnectedModel;
  bool GetFullyConnectedValue(bool &value);
  void SetFullyConnectedValue(bool value);
};

#endif // LABELPAINTSETTINGSMODEL_H
