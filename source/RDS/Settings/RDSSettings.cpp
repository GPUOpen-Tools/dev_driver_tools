//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation for RDS settings.
//=============================================================================

#include <QDir>
#include <QFile>
#include <QTextStream>

#include "RDSSettings.h"
#include "RDSSettingsReader.h"
#include "RDSSettingsWriter.h"
#include "../RDSDefinitions.h"
#include "../../Common/DriverToolsDefinitions.h"
#include "../../Common/ToolUtil.h"

/// Single instance of the RDSSettings.
static RDSSettings s_rdsSettings;

//-----------------------------------------------------------------------------
/// \return a reference to the RDSSettings.
//-----------------------------------------------------------------------------
RDSSettings& RDSSettings::Get()
{
    return s_rdsSettings;
}

//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
RDSSettings::RDSSettings()
{
    InitDefaultSettings();
}

//-----------------------------------------------------------------------------
/// Add a setting to our active map if it is recognized.
/// \param setting The setting to add.
//-----------------------------------------------------------------------------
void RDSSettings::AddPotentialSetting(const QString& name, const QString& value)
{
    for (RDSSettingsMap::iterator i = m_defaultSettings.begin(); i != m_defaultSettings.end(); ++i)
    {
        if (i.value().name.compare(name) == 0)
        {
            AddActiveSetting(i.key(), { name, value });
            break;
        }
    }
}

//-----------------------------------------------------------------------------
/// Apply default settings and then override them if found on disk.
/// \return Returns true if settings were read from file, and false otherwise.
//-----------------------------------------------------------------------------
bool RDSSettings::LoadSettings()
{
    // Begin by applying the defaults.
    for (RDSSettingsMap::iterator i = m_defaultSettings.begin(); i != m_defaultSettings.end(); ++i)
    {
        AddPotentialSetting(i.value().name, i.value().value);
    }

    QString settingsFilepath = ToolUtil::GetDriverToolsXmlFileLocation();
    settingsFilepath.append(QDir::separator());
    settingsFilepath.append(gs_RDS_PRODUCT_SETTINGS_FILENAME);

    QFile file(settingsFilepath);
    bool readSettingsFile = file.open(QFile::ReadOnly | QFile::Text);

    // Override the defaults.
    if (readSettingsFile)
    {
        RDSSettingsReader xmlReader(&RDSSettings::Get());

        readSettingsFile = xmlReader.Read(&file);

        // Make sure the XML parse worked.
        Q_ASSERT(readSettingsFile == true);
    }

    // Save the file.
    else
    {
        SaveSettings();

        readSettingsFile = false;
    }

    return readSettingsFile;
}

//-----------------------------------------------------------------------------
/// Save the RDS settings to disk.
//-----------------------------------------------------------------------------
void RDSSettings::SaveSettings()
{
    QString settingsFilepath = ToolUtil::GetDriverToolsXmlFileLocation();
    settingsFilepath.append(QDir::separator());
    settingsFilepath.append(gs_RDS_PRODUCT_SETTINGS_FILENAME);

    QFile file(settingsFilepath);
    bool success = file.open(QFile::WriteOnly | QFile::Text);

    if (success == true)
    {
        RDSSettingsWriter xmlWriter(&RDSSettings::Get());
        success = xmlWriter.Write(&file);

        Q_ASSERT(success == true);
    }
}

//-----------------------------------------------------------------------------
/// Initialize our table with default settings.
//-----------------------------------------------------------------------------
void RDSSettings::InitDefaultSettings()
{
    m_defaultSettings[RDS_LISTEN_PORT] =  { "ListenPort",  QString::number(gs_DEFAULT_CONNECTION_PORT) };
}

//-----------------------------------------------------------------------------
/// Store an active setting.
/// \param settingId The identifier for this setting.
/// \param setting The setting containing name and value.
//-----------------------------------------------------------------------------
void RDSSettings::AddActiveSetting(RDSSettingID settingId, const RDSSetting& setting)
{
    m_activeSettings[settingId] = setting;
}

//-----------------------------------------------------------------------------
/// Get a setting as a boolean value.
/// \param settingId The identifier for this setting.
/// \return The boolean value for the setting specified.
//-----------------------------------------------------------------------------
bool RDSSettings::GetBoolValue(RDSSettingID settingId) const
{
    return (m_activeSettings[settingId].value.compare("True") == 0) ? true : false;
}

//-----------------------------------------------------------------------------
/// Get a setting as an unsigned integer value.
/// \param settingId The identifier for this setting.
/// \returns The unsigned integer for the setting specified.
//-----------------------------------------------------------------------------
unsigned int RDSSettings::GetUnsignedIntValue(RDSSettingID settingId) const
{
    return m_activeSettings[settingId].value.toUInt();
}

//-----------------------------------------------------------------------------
/// Get a setting as an integer value.
/// \param settingId The identifier for this setting.
/// \return The integer value for the setting specified.
//-----------------------------------------------------------------------------
int RDSSettings::GetIntValue(RDSSettingID settingId) const
{
    return m_activeSettings[settingId].value.toInt();
}

//-----------------------------------------------------------------------------
/// Get a setting as a QColor object.
/// \param settingId The identifier for this setting.
/// \return The color value for the setting specified.
//-----------------------------------------------------------------------------
QColor RDSSettings::GetColorValue(RDSSettingID settingId) const
{
    QStringList c = m_activeSettings[settingId].value.split(",");

    Q_ASSERT(c.size() == 4);

    int r = c.at(0).toInt();
    int g = c.at(1).toInt();
    int b = c.at(2).toInt();
    int a = c.at(3).toInt();

    return QColor(r, g, b, a);
}

//-----------------------------------------------------------------------------
/// Get a setting as a QString object.
/// \param settingId The identifier for this setting.
/// \return The string value for the setting specified.
//-----------------------------------------------------------------------------
QString RDSSettings::GetStringValue(RDSSettingID settingId) const
{
    return m_activeSettings[settingId].value;
}

//-----------------------------------------------------------------------------
/// Set a setting as a boolean value.
/// \param settingId The identifier for this setting.
/// \param value The new value of the setting.
//-----------------------------------------------------------------------------
void RDSSettings::SetBoolValue(RDSSettingID settingId, const bool value)
{
    if (value == true)
    {
        AddPotentialSetting(m_defaultSettings[settingId].name, "True");
    }
    else
    {
        AddPotentialSetting(m_defaultSettings[settingId].name, "False");
    }
}

//-----------------------------------------------------------------------------
/// Set a setting as an integer value.
/// \param settingId The identifier for this setting.
/// \param value The new value of the setting.
//-----------------------------------------------------------------------------
void RDSSettings::SetIntValue(RDSSettingID settingId, const int value)
{
    AddPotentialSetting(m_defaultSettings[settingId].name, QString::number(value));
}

//-----------------------------------------------------------------------------
/// Set a setting as a QColor value.
/// \param settingId The identifier for this setting.
/// \param value The new value of the setting.
//-----------------------------------------------------------------------------
void RDSSettings::SetColorValue(RDSSettingID settingId, const QColor& value)
{
    QString str;
    QTextStream(&str) << value.red() << ", " << value.green() << ", " << value.blue() << ", " << value.alpha();
    AddPotentialSetting(m_defaultSettings[settingId].name, str);
}

//-----------------------------------------------------------------------------
/// Set a setting as a QString value.
/// \param settingId The identifier for this setting.
/// \param value The new value for the setting.
//-----------------------------------------------------------------------------
void RDSSettings::SetStringValue(RDSSettingID settingId, const QString& value)
{
    AddPotentialSetting(m_defaultSettings[settingId].name, value);
}

//-----------------------------------------------------------------------------
/// Get current the RDS listen port.
/// \returns The current the RDS listen port.
//-----------------------------------------------------------------------------
unsigned int RDSSettings::GetListenPort() const
{
    return GetIntValue(RDS_LISTEN_PORT);
}

//-----------------------------------------------------------------------------
/// Sets the port that RDS will use to listen for incoming connections.
/// \param listenPort The port used to listen for incoming connections.
//-----------------------------------------------------------------------------
void RDSSettings::SetListenPort(unsigned int listenPort)
{
    AddPotentialSetting(m_defaultSettings[RDS_LISTEN_PORT].name, QString::number(listenPort));
    SaveSettings();
}
