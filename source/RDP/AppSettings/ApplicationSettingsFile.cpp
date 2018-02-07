//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation for an application-specific settings file.
//=============================================================================

#include "ApplicationSettingsFile.h"
#include "../../Common/ToolUtil.h"

//-----------------------------------------------------------------------------
/// Constructor for the ApplicationSettingsFile type.
//-----------------------------------------------------------------------------
ApplicationSettingsFile::ApplicationSettingsFile()
    : m_targetApplicationName("")
    , m_isGlobal(false)
    , m_pFileInfo(nullptr)
{
}

//-----------------------------------------------------------------------------
/// Add a new setting to the ApplicationSettingsFile instance.
/// \param category The category to add the setting to.
/// \param newSetting The new setting data to add to the file.
//-----------------------------------------------------------------------------
void ApplicationSettingsFile::AddSetting(const QString& category, const DevDriver::SettingsProtocol::Setting& newSetting)
{
    DriverSettingsMap::iterator categoryIter = m_driverSettings.find(category);
    if (categoryIter != m_driverSettings.end())
    {
        DriverSettingVector& categorySettings = (*categoryIter);
        categorySettings.push_back(newSetting);
    }
    else
    {
        DriverSettingVector& categorySettings = m_driverSettings[category];
        categorySettings.push_back(newSetting);
    }
}

//-----------------------------------------------------------------------------
/// Find the differences in driver settings between the internal map of settings vs the given settings file.
/// \param otherSettingsMap The other set of settings to compare with.
/// \param differences An output vector that will be filled with the difference between settings maps.
/// \returns True if the given settings maps differ, and false if they are the same.
//-----------------------------------------------------------------------------
bool ApplicationSettingsFile::GetSettingsMapDelta(const DriverSettingsMap& otherSettingsMap, DriverSettingsMap& differences) const
{
    bool hasDifference = false;

    // Compare the entire global settings map and application settings map.
    DriverSettingsMap::const_iterator settingsStart = m_driverSettings.begin();
    DriverSettingsMap::const_iterator settingsEnd = m_driverSettings.end();

    // Step through the driver settings and find the matching category in the other settings file.
    DriverSettingsMap::const_iterator settingsIter;
    for (settingsIter = settingsStart; settingsIter != settingsEnd; ++settingsIter)
    {
        const QString& category = settingsIter.key();

        // Search the other file's settings map for the same category.
        const DriverSettingsMap::const_iterator otherSettingsIter = otherSettingsMap.find(category);
        if (otherSettingsIter != otherSettingsMap.end())
        {
            // The other file has a matching category. Do the settings in the category match?
            const DriverSettingVector& settings = settingsIter.value();
            const DriverSettingVector& otherValues = otherSettingsIter.value();

            DriverSettingVector valuesDelta;
            if (GetSettingsVectorDelta(settings, otherValues, valuesDelta))
            {
                differences.insert(category, valuesDelta);
            }
        }
    }

    if (!differences.isEmpty())
    {
        hasDifference = true;
    }

    return hasDifference;
}

//-----------------------------------------------------------------------------
/// Find the differences in driver settings between the internal map of settings vs the given settings file.
/// \param pOtherSettingsFile The other set of settings file to compare settings with.
/// \param differences An output vector that will be filled with the difference between settings maps.
/// \returns True if the given settings maps differ, and false if they are the same.
//-----------------------------------------------------------------------------
bool ApplicationSettingsFile::GetSettingsDelta(ApplicationSettingsFile* pOtherSettingsFile, DriverSettingsMap& differences) const
{
    const DriverSettingsMap& otherSettings = pOtherSettingsFile->GetDriverSettings();

    return GetSettingsMapDelta(otherSettings, differences);
}

//-----------------------------------------------------------------------------
/// Find the difference between the initial settings vs another given settings vector. Put the differences in an output vector.
/// \param initialSettings The initial settings to compare.
/// \param otherSettings The other set of settings to compare with.
/// \param differences An output vector that will be filled with the difference between settings maps.
/// \returns True if the given settings maps differ, and false if they are the same.
//-----------------------------------------------------------------------------
bool ApplicationSettingsFile::GetSettingsVectorDelta(const DriverSettingVector& initialSettings, const DriverSettingVector& otherSettings, DriverSettingVector& differences) const
{
    bool settingsDiffer = false;

    // We should assume that that incoming vectors aren't empty.
    Q_ASSERT((initialSettings.isEmpty() || otherSettings.isEmpty()) == false);

    for (int settingIndex = 0; settingIndex < initialSettings.size(); ++settingIndex)
    {
        const DriverSettingVector::const_iterator initialSettingIter = initialSettings.begin() + settingIndex;
        const DriverSettingVector::const_iterator otherSettingIter = otherSettings.begin() + settingIndex;

        // First ensure we're comparing the same setting values, so the names must match.
        const QString& initialSettingName = initialSettingIter->name;
        const QString& otherSettingName = otherSettingIter->name;
        if (initialSettingName.compare(otherSettingName) == 0)
        {
            const DevDriver::SettingsProtocol::SettingValue* pInitialValue = &(initialSettingIter->value);
            const DevDriver::SettingsProtocol::SettingValue* pOtherValue = &(otherSettingIter->value);

            // Do the values differ?
            int valueMatches = memcmp(pInitialValue, pOtherValue, sizeof(DevDriver::SettingsProtocol::SettingValue));
            if (valueMatches != 0)
            {
                // The values were different, so add it to the output map.
                differences.push_back(*initialSettingIter);

                settingsDiffer = true;
            }
        }
    }

    return settingsDiffer;
}

//-----------------------------------------------------------------------------
/// Add a new setting to the ApplicationSettingsFile instance.
/// \param category The category to add the setting to.
/// \param newSetting The new setting data to add to the file.
//-----------------------------------------------------------------------------
bool ApplicationSettingsFile::UpdateSetting(const QString& category, const DevDriver::SettingsProtocol::Setting& newSetting)
{
    bool settingUpdated = false;

    DriverSettingsMap::iterator categoryIter = m_driverSettings.find(category);
    if (categoryIter != m_driverSettings.end())
    {
        DriverSettingVector& categorySettings = categoryIter.value();

        // Step through the list of settings to look for the one that matches the incoming change.
        for (int settingIndex = 0; settingIndex < categorySettings.size(); ++settingIndex)
        {
            const QString name(categorySettings[settingIndex].name);

            const QString settingName(newSetting.name);
            if (name.compare(settingName) == 0)
            {
                DevDriver::SettingsProtocol::Setting& setting = categorySettings[settingIndex];

                memcpy(&setting.value, &newSetting.value, sizeof(DevDriver::SettingsProtocol::SettingValue));
                settingUpdated = true;
                break;
            }
        }
    }

    // The category already exists, but we didn't find the setting in it. Add it here.
    if (!settingUpdated)
    {
        // The setting didn't exist yet. Add it as if it's a new setting.
        m_driverSettings[category].push_back(newSetting);
    }

    return settingUpdated;
}

//-----------------------------------------------------------------------------
/// Copy this file data from another application settings file.
/// \param pOtherFile The settings file containing the data to be copied.
//-----------------------------------------------------------------------------
void ApplicationSettingsFile::CopyFrom(ApplicationSettingsFile* pOtherFile)
{
    // Do nothing if other file is null
    if (pOtherFile == nullptr)
    {
        return;
    }

    m_targetApplicationName = pOtherFile->GetTargetApplicationName();
    m_driverSettings = pOtherFile->GetDriverSettings();
}

//-----------------------------------------------------------------------------
/// Restore all the settings in this file to their default state.
//-----------------------------------------------------------------------------
void ApplicationSettingsFile::RestoreToDefaultSettings()
{
    // Step through the driver settings and find the matching category in the other settings file.
    for (DriverSettingVector& settingsVector : m_driverSettings)
    {
        for (DevDriver::SettingsProtocol::Setting& setting : settingsVector)
        {
            setting.value = setting.defaultValue;
        }
    }
}
