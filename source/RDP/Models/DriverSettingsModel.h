//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Driver Settings model class definition.
//=============================================================================

#ifndef _DRIVER_SETTINGS_MODEL_H_
#define _DRIVER_SETTINGS_MODEL_H_

#include <QVector>
#include "DeveloperPanelModel.h"
#include "DriverProtocolModel.h"
#include "ApplicationSettingsModel.h"
#include "../AppSettings/ApplicationSettingsFile.h"
#include "../../DevDriverComponents/inc/protocols/settingsClient.h"

class QVariant;

namespace DevDriver
{
    namespace SettingsProtocol
    {
        class SettingsClient;
    }
}

class ApplicationSettingsModel;

/// An enum used to represent the controls in the driver settings interface.
enum DriverSettingsControls
{
    DRIVER_SETTINGS_CONTROLS_COUNT              ///< The number of controls in the interface.
};

/// Model class for the Driver Settings tab.
class DriverSettingsModel : public DriverProtocolModel
{
public:
    explicit DriverSettingsModel(DeveloperPanelModel* pPanelModel, ApplicationSettingsModel* pApplicationSettingsModel, uint32_t modelCount);
    virtual ~DriverSettingsModel();

    void InitializeDefaults();
    void Update(DriverSettingsControls modelIndex, const QVariant& value);
    void UpdateDriverSettings(const DriverSettingsMap& settingsMap);
    void UpdateDriverSetting(QString categoryName, const DevDriver::SettingsProtocol::Setting& setting);

    ApplicationSettingsModel* GetApplicationSettingsModel() const { return m_pSettingsModel; }
    const DriverSettingsMap& GetSettingsMap() const { return m_pSettingsModel->GetSettingsFile()->GetDriverSettings(); }

private:
    QString m_settingDescription;                       ///< The model's setting description string.
    ApplicationSettingsModel* m_pSettingsModel;         ///< The settings file where the chosen settings are serialized.
};

#endif // _DRIVER_SETTINGS_MODEL_H_
