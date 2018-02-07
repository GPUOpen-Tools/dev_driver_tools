//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the Application Settings File Writer.
//=============================================================================

#include "ApplicationSettingsFileWriter.h"
#include "../RDPDefinitions.h"
#include "../../Common/ToolUtil.h"
#include "../../DevDriverComponents/inc/gpuopen.h"
#include "../../DevDriverComponents/inc/protocols/settingsClient.h"

//-----------------------------------------------------------------------------
/// Explicit constructor.
/// \param pAppSettingsFile The application settings to write.
//-----------------------------------------------------------------------------
ApplicationSettingsFileWriter::ApplicationSettingsFileWriter(ApplicationSettingsFile* pAppSettingsFile) :
    m_pDriverSettingsFile(pAppSettingsFile)
{
}

//-----------------------------------------------------------------------------
/// Begin writing RDP XML file and make sure it's valid.
/// \param pDevice The XML file represented by a Qt IO device.
//-----------------------------------------------------------------------------
bool ApplicationSettingsFileWriter::Write(QIODevice* pDevice)
{
    m_writer.setDevice(pDevice);
    m_writer.setAutoFormatting(true);

    m_writer.writeStartDocument();

    // The "ApplicationSettings" node is the root element of the document.
    m_writer.writeStartElement(gs_APPLICATION_SETTINGS_ROOT_ELEMENT);

    // Write the flag indicating if this is the global settings page.
    WriteGlobalFlag();

    // Write the target executable filename.
    WriteTargetExecutable();

    // Now write all application-specific driver settings to XML.
    m_writer.writeStartElement(gs_APPLICATION_SETTINGS_DRIVER_SETTINGS);
    WriteDriverSettings();
    m_writer.writeEndElement();

    m_writer.writeEndElement();

    m_writer.writeEndDocument();

    return m_writer.hasError() == false;
}

//-----------------------------------------------------------------------------
/// Write the target executable filename to the XML stream.
//-----------------------------------------------------------------------------
void ApplicationSettingsFileWriter::WriteTargetExecutable()
{
    // Surround the filename with "TargetExecutable" elements.
    m_writer.writeTextElement(gs_APPLICATION_SETTINGS_TARGET_EXECUTABLE, m_pDriverSettingsFile->GetTargetApplicationName());
}

//-----------------------------------------------------------------------------
/// Write all driver settings to the XML stream.
//-----------------------------------------------------------------------------
void ApplicationSettingsFileWriter::WriteDriverSettings()
{
    using namespace DevDriver::SettingsProtocol;

    if (m_pDriverSettingsFile != nullptr)
    {
        const DriverSettingsMap& settingsMap = m_pDriverSettingsFile->GetDriverSettings();

        DriverSettingsMap::const_iterator categoryIter;
        DriverSettingsMap::const_iterator firstCategoryIter = settingsMap.begin();
        DriverSettingsMap::const_iterator lastCategoryIter = settingsMap.end();
        for (categoryIter = firstCategoryIter; categoryIter != lastCategoryIter; ++categoryIter)
        {
            const QString categoryString = categoryIter.key();
            const DriverSettingVector& settingsVector = categoryIter.value();

            m_writer.writeStartElement(gs_APPLICATION_SETTINGS_CATEGORY);
            m_writer.writeTextElement(gs_APPLICATION_SETTINGS_CATEGORY_NAME, categoryString);

            DriverSettingVector::const_iterator settingIter;
            for (settingIter = settingsVector.begin(); settingIter != settingsVector.end(); ++settingIter)
            {
                const DevDriver::SettingsProtocol::Setting& currentSetting = *settingIter;

                WriteDriverSetting(currentSetting);
            }

            // Write the end "Category" element.
            m_writer.writeEndElement();
        }
    }
}

//-----------------------------------------------------------------------------
/// Write a single driver setting line to the XML stream.
/// \param currentSetting The setting to write.
//-----------------------------------------------------------------------------
void ApplicationSettingsFileWriter::WriteDriverSetting(const DevDriver::SettingsProtocol::Setting& currentSetting)
{
    using namespace DevDriver::SettingsProtocol;

    m_writer.writeStartElement(gs_APPLICATION_SETTINGS_SETTING);

    m_writer.writeTextElement(gs_APPLICATION_SETTINGS_SETTING_NAME, currentSetting.name);

    // Need to surround the setting description with a CDATA tag so it doesn't ruin the XML file.
    QString cDataDescription = "<![CDATA[";
    cDataDescription.append(currentSetting.description);
    cDataDescription.append("]]>");
    m_writer.writeTextElement(gs_APPLICATION_SETTINGS_DESCRIPTION, cDataDescription);

    m_writer.writeTextElement(gs_APPLICATION_SETTINGS_TYPE, QString::number(static_cast<DevDriver::uint32>(currentSetting.type)));
    m_writer.writeTextElement(gs_APPLICATION_SETTINGS_CATEGORY_INDEX, QString::number(static_cast<DevDriver::uint32>(currentSetting.categoryIndex)));

    // Write the defaultValue and value members of the setting.
    WriteDriverSettingValue(currentSetting.type, gs_APPLICATION_SETTINGS_VALUE, currentSetting.value);
    WriteDriverSettingValue(currentSetting.type, gs_APPLICATION_SETTINGS_DEFAULT_VALUE, currentSetting.defaultValue);

    // Write the end "Setting" element.
    m_writer.writeEndElement();
}

//-----------------------------------------------------------------------------
/// Write an individual setting value into the XML stream.
/// \param type The type of setting being written.
/// \param tagText The element text to use when writing the setting value.
/// \param value The value of the setting to write to the XML stream.
//-----------------------------------------------------------------------------
void ApplicationSettingsFileWriter::WriteDriverSettingValue(DevDriver::SettingsProtocol::SettingType type, const QString& tagText, const DevDriver::SettingsProtocol::SettingValue& value)
{
    using namespace DevDriver::SettingsProtocol;

    // Format the value based on the type of setting being written.
    switch (type)
    {
    case SettingType::Boolean:
        {
            const QString boolString = QString::number(value.boolValue);
            m_writer.writeTextElement(tagText, boolString);
        }
        break;
    case SettingType::Integer:
        {
            const QString intString = QString::number(value.integerValue);
            m_writer.writeTextElement(tagText, intString);
        }
        break;
    case SettingType::UnsignedInteger:
        {
            const QString uIntString = QString::number(value.unsignedIntegerValue);
            m_writer.writeTextElement(tagText, uIntString);
        }
        break;
    case SettingType::Float:
        {
            const QString floatString = QString::number(value.floatValue);
            m_writer.writeTextElement(tagText, floatString);
        }
        break;
    case SettingType::String:
        {
            const QString stringValue = QString(value.stringValue);
            m_writer.writeTextElement(tagText, stringValue);
        }
        break;
    case SettingType::Hex:
        {
            const QString hexValue = QString(value.unsignedIntegerValue);
            m_writer.writeTextElement(tagText, hexValue);
        }
        break;
    default:
        {
        // If this happens, a new setting type has been added, and RDP must be updated.
        Q_ASSERT(false);
        }
    }
}

//-----------------------------------------------------------------------------
/// Write the global settings page flag.
//-----------------------------------------------------------------------------
void ApplicationSettingsFileWriter::WriteGlobalFlag()
{
    // Surround the filename with "TargetExecutable" elements.
    const QString globalString = QString::number(m_pDriverSettingsFile->IsGlobal());
    m_writer.writeTextElement("IsGlobal", globalString);
}
