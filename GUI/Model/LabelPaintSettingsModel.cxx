#include "LabelPaintSettingsModel.h"
#include "GlobalUIModel.h"
#include "GlobalState.h"

LabelPaintSettingsModel::LabelPaintSettingsModel()
{
  m_FullyConnectedModel = wrapGetterSetterPairAsProperty(
    this, &Self::GetFullyConnectedValue, &Self::SetFullyConnectedValue);
}

LabelPaintSettingsModel::~LabelPaintSettingsModel() {}

PaintbrushSettings
LabelPaintSettingsModel::GetPaintbrushSettings()
{
  return m_ParentModel->GetGlobalState()->GetLabelPaintSettings().brush;
}

void
LabelPaintSettingsModel::SetPaintbrushSettings(PaintbrushSettings ps)
{
  // The label paint brush is always a plain (manual) brush
  ps.smart_mode = PAINTBRUSH_MANUAL;

  LabelPaintSettings lps = m_ParentModel->GetGlobalState()->GetLabelPaintSettings();
  lps.brush = ps;
  m_ParentModel->GetGlobalState()->SetLabelPaintSettings(lps);
  InvokeEvent(ModelUpdateEvent());
}

bool
LabelPaintSettingsModel::GetFullyConnectedValue(bool &value)
{
  value = m_ParentModel->GetGlobalState()->GetLabelPaintSettings().fully_connected;
  return true;
}

void
LabelPaintSettingsModel::SetFullyConnectedValue(bool value)
{
  LabelPaintSettings lps = m_ParentModel->GetGlobalState()->GetLabelPaintSettings();
  lps.fully_connected = value;
  m_ParentModel->GetGlobalState()->SetLabelPaintSettings(lps);
  InvokeEvent(ModelUpdateEvent());
}
