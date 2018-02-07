//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of RDP's Application Settings file reader.
//=============================================================================

#include "ApplicationSettingsFileReader.h"
#include "ApplicationSettingsFile.h"
#include "../RDPDefinitions.h"
#include "../../Common/ToolUtil.h"
#include "../../DevDriverComponents/inc/ddPlatform.h"
#include "../../DevDriverComponents/inc/protocols/settingsClient.h"

using namespace DevDriver;
using namespace DevDriver::SettingsProtocol;

//-----------------------------------------------------------------------------
/// Constructor.
/// \param pApplicationSettingsFile Output settings class.
//-----------------------------------------------------------------------------
ApplicationSettingsFileReader::ApplicationSettingsFileReader(ApplicationSettingsFile* pApplicationSettingsFile) :
    m_pApplicationSettingsFile(pApplicationSettingsFile)
{
}

//-----------------------------------------------------------------------------
/// Begin reading application settings file and make sure it's valid.
/// \param pDevice The XML file represented by a Qt IO device.
//-----------------------------------------------------------------------------
bool ApplicationSettingsFileReader::Read(QIODevice* pDevice)
{
    m_reader.setDevice(pDevice);

    if (m_reader.readNextStartElement())
    {
        if (m_reader.name() == gs_APPLICATION_SETTINGS_ROOT_ELEMENT)
        {
            ReadSettingsFile();
        }
    }

    return m_reader.error() == false;
}

//-----------------------------------------------------------------------------
/// Read the entire settings file from the XML stream.
//-----------------------------------------------------------------------------
void ApplicationSettingsFileReader::ReadSettingsFile()
{
    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == gs_APPLICATION_SETTINGS_IS_GLOBAL)
        {
            const QString isGlobalString = m_reader.readElementText();

            bool isGlobal = (isGlobalString.toInt() == 1);
            m_pApplicationSettingsFile->SetIsGlobal(isGlobal);
        }
        else if (m_reader.name() == gs_APPLICATION_SETTINGS_TARGET_EXECUTABLE)
        {
            const QString executableName = m_reader.readElementText();
            m_pApplicationSettingsFile->SetTargetExecutableName(executableName);
        }
        else if (m_reader.name() == gs_APPLICATION_SETTINGS_DRIVER_SETTINGS)
        {
            ReadDriverSettings();
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }
}

//-----------------------------------------------------------------------------
/// Read all Driver Settings elements to populate the settings file.
//-----------------------------------------------------------------------------
void ApplicationSettingsFileReader::ReadDriverSettings()
{
    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == gs_APPLICATION_SETTINGS_CATEGORY)
        {
            ReadSettingsCategory();
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }
}

//-----------------------------------------------------------------------------
/// Read a category and the settings within it.
//-----------------------------------------------------------------------------
void ApplicationSettingsFileReader::ReadSettingsCategory()
{
    QString currentCategory = "";
    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == gs_APPLICATION_SETTINGS_CATEGORY_NAME)
        {
            currentCategory = m_reader.readElementText();
        }
        else if (m_reader.name() == gs_APPLICATION_SETTINGS_SETTING)
        {
            // Parse each new setting and add it to the setting file.
            DevDriver::SettingsProtocol::Setting newSetting = {};
            ReadDriverSetting(newSetting);

            Q_ASSERT(currentCategory.isEmpty() != true);
            m_pApplicationSettingsFile->AddSetting(currentCategory, newSetting);
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }
}

//-----------------------------------------------------------------------------
/// Read an individual driver setting from the XML stream.
//-----------------------------------------------------------------------------
void ApplicationSettingsFileReader::ReadDriverSetting(DevDriver::SettingsProtocol::Setting& setting)
{
    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == gs_APPLICATION_SETTINGS_SETTING_NAME)
        {
            const QString settingName = m_reader.readElementText();
            std::string settingNameStdString = settingName.toStdString();
            Platform::Strncpy(setting.name, settingNameStdString.c_str(), kSmallStringSize);
        }
        else if (m_reader.name() == gs_APPLICATION_SETTINGS_CATEGORY_INDEX)
        {
            const QString categoryIndex = m_reader.readElementText();
            unsigned int indexTypeInt = categoryIndex.toUInt();
            setting.categoryIndex = indexTypeInt;
        }
        else if (m_reader.name() == gs_APPLICATION_SETTINGS_DESCRIPTION)
        {
            // Need to extract the description string from the CDATA tag.
            QString settingDescription = m_reader.readElementText();
            settingDescription = settingDescription.replace("<![CDATA[", "");
            settingDescription = settingDescription.replace("]]>", "");

            std::string settingDescriptionStdString = settingDescription.toStdString();
            Platform::Strncpy(setting.description, settingDescriptionStdString.c_str(), kLargeStringSize);
        }
        else if (m_reader.name() == gs_APPLICATION_SETTINGS_TYPE)
        {
            const QString settingType = m_reader.readElementText();
            unsigned int settingTypeInt = settingType.toUInt();
            setting.type = static_cast<SettingType>(settingTypeInt);
        }
        else if (m_reader.name() == gs_APPLICATION_SETTINGS_VALUE)
        {
            const QString settingValue = m_reader.readElementText();
            ReadDriverSettingValue(settingValue, setting.type, setting.value);
        }
        else if (m_reader.name() == gs_APPLICATION_SETTINGS_DEFAULT_VALUE)
        {
            const QString defaultValue = m_reader.readElementText();
            ReadDriverSettingValue(defaultValue, setting.type, setting.defaultValue);
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }
}

//-----------------------------------------------------------------------------
/// Parse the incoming value string and store the converted result in the given SettingValue instance.
/// \param settingValueString The string containing the setting value.
/// \param value The instance to store the value in.
/// \returns True if conversion was successful, and false if parsing failed.
//-----------------------------------------------------------------------------
bool ApplicationSettingsFileReader::ReadDriverSettingValue(const QString& settingValueString, DevDriver::SettingsProtocol::SettingType settingType, DevDriver::SettingsProtocol::SettingValue& value)
{
    bool convertedValue = true;

    using namespace DevDriver::SettingsProtocol;

    switch (static_cast<SettingType>(settingType))
    {
    case SettingType::Boolean:
        {
            value.boolValue = (settingValueString.toInt() == 1);
        }
        break;
    case SettingType::Integer:
        {
            value.integerValue = settingValueString.toInt();
        }
        break;
    case SettingType::UnsignedInteger:
        {
            value.unsignedIntegerValue = settingValueString.toUInt();
        }
        break;
    case SettingType::Float:
        {
        value.floatValue = settingValueString.toFloat();
        }
        break;
    case SettingType::String:
        {
            value.floatValue = settingValueString.toFloat();
        }
        break;
    case SettingType::Hex:
        {
            value.unsignedIntegerValue = settingValueString.toUInt();
        }
        break;
    default:
        // If this fails, a new type of setting has probably been added, and RDP needs to be updated.
        convertedValue = false;
        Q_ASSERT(false);
    }

    return convertedValue;
}
