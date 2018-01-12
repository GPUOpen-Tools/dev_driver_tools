//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Driver Settings model class definition.
//=============================================================================

#include <QList>
#include <QVariant>
#include "gpuopen.h"
#include "ApplicationSettingsModel.h"
#include "DriverSettingsModel.h"
#include "DeveloperPanelModel.h"
#include "../RDPDefinitions.h"
#include "../Settings/RDPSettings.h"
#include "../../Common/ToolUtil.h"

using namespace DevDriver;
using namespace DevDriver::SettingsProtocol;

//-----------------------------------------------------------------------------
/// Constructor for DriverSettingsModel.
/// \param pPanelModel The main Panel model used to maintain a connection to RDS.
/// \param modelCount the number of models required.
//-----------------------------------------------------------------------------
DriverSettingsModel::DriverSettingsModel(DeveloperPanelModel* pPanelModel, ApplicationSettingsModel* pApplicationSettingsModel, uint32_t modelCount)
    : DriverProtocolModel(pPanelModel, modelCount)
    , m_pSettingsModel(pApplicationSettingsModel)
{
}

//-----------------------------------------------------------------------------
/// Destructor for DriverSettingsModel.
//-----------------------------------------------------------------------------
DriverSettingsModel::~DriverSettingsModel()
{
}

//-----------------------------------------------------------------------------
/// Initialize default values within the interface.
//-----------------------------------------------------------------------------
void DriverSettingsModel::InitializeDefaults()
{
}

//-----------------------------------------------------------------------------
/// Update the internal model members.
/// \param modelIndex The ID of the UI element that's being updated.
/// \param value The new value of the UI element.
//-----------------------------------------------------------------------------
void DriverSettingsModel::Update(DriverSettingsControls modelIndex, const QVariant& value)
{
    SetModelData(modelIndex, value);
}

//-----------------------------------------------------------------------------
/// Update the existing driver settings with the incoming settings values.
/// \param settingsMap The new driver settings map to apply.
//-----------------------------------------------------------------------------
void DriverSettingsModel::UpdateDriverSettings(const DriverSettingsMap& settingsMap)
{
    ApplicationSettingsModel* pSettingsModel = GetApplicationSettingsModel();
    Q_ASSERT(pSettingsModel != nullptr);

    if (pSettingsModel != nullptr)
    {
        ApplicationSettingsFile* pSettingsFile = pSettingsModel->GetSettingsFile();
        if (pSettingsFile != nullptr)
        {
            DriverSettingsMap::const_iterator settingsStart = settingsMap.begin();
            DriverSettingsMap::const_iterator settingsEnd = settingsMap.end();

            DriverSettingsMap::const_iterator categoryIter;
            for (categoryIter = settingsStart; categoryIter != settingsEnd; ++categoryIter)
            {
                const QString& categoryName = categoryIter.key();
                const DriverSettingVector& settings = categoryIter.value();

                for (int settingIndex = 0; settingIndex < settings.size(); ++settingIndex)
                {
                    const DevDriver::SettingsProtocol::Setting& currentSetting = settings.at(settingIndex);

                    const QString settingName(currentSetting.name);
                    pSettingsFile->UpdateSetting(categoryName, currentSetting);
                }
            }

            // Write the setting file after the change
            RDPSettings::Get().WriteApplicationSettingsFile(pSettingsFile);
        }
    }
}

//-----------------------------------------------------------------------------
/// Update a single driver setting.
/// \param categoryName The name of the setting category for the setting
/// to update.
/// \param Setting struct to update.
//-----------------------------------------------------------------------------
void DriverSettingsModel::UpdateDriverSetting(QString categoryName, const DevDriver::SettingsProtocol::Setting& setting)
{
    ApplicationSettingsModel* pSettingsModel = GetApplicationSettingsModel();
    Q_ASSERT(pSettingsModel != nullptr);

    if (pSettingsModel != nullptr)
    {
        ApplicationSettingsFile* pSettingsFile = pSettingsModel->GetSettingsFile();
        if (pSettingsFile != nullptr)
        {
            // Update the setting
            pSettingsFile->UpdateSetting(categoryName, setting);

            // Write the setting file after each change
            RDPSettings::Get().WriteApplicationSettingsFile(pSettingsFile);
        }
    }
}
