//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of RDP's XML settings reader.
//=============================================================================

#include "RDPSettingsReader.h"
#include "RDPSettings.h"
#include "../RDPDefinitions.h"
#include "../../DevDriverComponents/inc/protocols/rgpProtocol.h"

//-----------------------------------------------------------------------------
/// Constructor.
/// \param pRdpSettings Output settings class.
//-----------------------------------------------------------------------------
RDPSettingsReader::RDPSettingsReader(RDPSettings* pRdpSettings) :
    m_pRdpSettings(pRdpSettings)
{
}

//-----------------------------------------------------------------------------
/// Begin reading RDP XML file and make sure it's valid.
/// \param pDevice The XML file represented by a Qt IO device.
//-----------------------------------------------------------------------------
bool RDPSettingsReader::Read(QIODevice* pDevice)
{
    m_reader.setDevice(pDevice);

    if (m_reader.readNextStartElement())
    {
        if (m_reader.name() == "RDP")
        {
            ReadSettingsAndRecents();
        }
    }

    return m_reader.error() == false;
}

//-----------------------------------------------------------------------------
/// Detect global settings and recently used files section.
//-----------------------------------------------------------------------------
void RDPSettingsReader::ReadSettingsAndRecents()
{
    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == "GlobalSettings")
        {
            ReadSettings();
        }
        else if (m_reader.name() == "RecentFiles")
        {
            ReadRecentSettingsFiles();
        }
        else if (m_reader.name() == "RecentConnections")
        {
            ReadRecentConnections();
        }
        else if (m_reader.name() == "TargetApplications")
        {
            ReadTargetApplications();
        }
        else if (m_reader.name() == "UserClockMode")
        {
            ReadUserClockMode();
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }
}

//-----------------------------------------------------------------------------
/// Detect setting list.
//-----------------------------------------------------------------------------
void RDPSettingsReader::ReadSettings()
{
    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == "Setting")
        {
            ReadSetting();
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }
}

//-----------------------------------------------------------------------------
/// Read individual settings.
//-----------------------------------------------------------------------------
void RDPSettingsReader::ReadSetting()
{
    RDPSetting setting;

    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == "Name")
        {
            setting.name = m_reader.readElementText();
        }
        else if (m_reader.name() == "Value")
        {
            setting.value = m_reader.readElementText();
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }

    m_pRdpSettings->AddPotentialSetting(setting.name, setting.value);
}

//-----------------------------------------------------------------------------
/// Detect recently opened file list.
//-----------------------------------------------------------------------------
void RDPSettingsReader::ReadRecentSettingsFiles()
{
    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == "RecentFile")
        {
            ReadRecentSettingsFile();
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }
}

//-----------------------------------------------------------------------------
/// Read individual recently opened files.
//-----------------------------------------------------------------------------
void RDPSettingsReader::ReadRecentSettingsFile()
{
    RDPApplicationSettingsFile* recentFile = new RDPApplicationSettingsFile;

    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == "Path")
        {
            recentFile->filepath = m_reader.readElementText();
        }
        else if (m_reader.name() == "Created")
        {
            recentFile->createdTimestamp = m_reader.readElementText();
        }
        else if (m_reader.name() == "Accessed")
        {
            recentFile->lastAccessed = m_reader.readElementText();
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }

    m_pRdpSettings->AddAppSettingsFile(recentFile);
}

//-----------------------------------------------------------------------------
/// Read all recent connection info from the RDP settings file.
//-----------------------------------------------------------------------------
void RDPSettingsReader::ReadRecentConnections()
{
    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == "Connection")
        {
            ReadRecentConnection();
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }
}

//-----------------------------------------------------------------------------
/// Read an individual Recent Connection Info structure from the RDP settings file.
//-----------------------------------------------------------------------------
void RDPSettingsReader::ReadRecentConnection()
{
    RDSConnectionInfo connectionInfo = {};

    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == "Hostname")
        {
            connectionInfo.hostnameString = m_reader.readElementText();
        }
        else if (m_reader.name() == "IP")
        {
            connectionInfo.ipString = m_reader.readElementText();

            // NOTE: This code is added to make the "localhost" string
            // backwards compatible. In a future pass, this will be within
            // an if statement to check the version number of the settings
            // file before executing this part of the code.
            if (connectionInfo.ipString.compare(gs_LOCAL_HOST_IP) == 0)
            {
                connectionInfo.ipString = gs_LOCAL_HOST;
            }
        }
        else if (m_reader.name() == "Port")
        {
            const QString portString = m_reader.readElementText();
            bool converted = false;
            uint portNumber = portString.toUInt(&converted);
            Q_ASSERT(converted == true);
            if (converted)
            {
                connectionInfo.port = portNumber;
            }
        }
        else if (m_reader.name() == "Autoconnect")
        {
            // Read the bool as a string, and then convert the string to an actual bool.
            const QString autoconnectString = m_reader.readElementText();

            bool autoconnect = (autoconnectString.toInt() == 1);
            connectionInfo.autoconnect = autoconnect;
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }

    m_pRdpSettings->AddRecentConnection(connectionInfo);
}

//-----------------------------------------------------------------------------
/// Read all target applications from the settings file XML.
//-----------------------------------------------------------------------------
void RDPSettingsReader::ReadTargetApplications()
{
    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == "Target")
        {
            ReadTargetApplication();
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }
}

//-----------------------------------------------------------------------------
/// Read an individual target application from the settings file XML.
//-----------------------------------------------------------------------------
void RDPSettingsReader::ReadTargetApplication()
{
    RDSTargetApplicationInfo appInfo = {};

    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == "ProcessName")
        {
            appInfo.processName = m_reader.readElementText();
        }
        else if (m_reader.name() == "AppTitle")
        {
            appInfo.titleName = m_reader.readElementText();
        }
        else if (m_reader.name() == "API")
        {
            const QString apiString = m_reader.readElementText();
            appInfo.apiName = apiString;
        }
        else if (m_reader.name() == "ApplySettings")
        {
            // Read the bool as a string, and then convert the string to an actual bool.
            const QString applySettingsString = m_reader.readElementText();
            bool applySettingsBool = (applySettingsString.toInt() == 1);
            appInfo.applySettings = applySettingsBool;
        }
        else if (m_reader.name() == "AllowProfiling")
        {
            // Read the bool as a string, and then convert the string to an actual bool.
            const QString allowProfilingString = m_reader.readElementText();
            bool allowProfilingBool = (allowProfilingString.toInt() == 1);
            appInfo.allowProfiling = allowProfilingBool;
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }

    m_pRdpSettings->AddTargetApplication(appInfo, true);  // Add to bottom of list
}

//-----------------------------------------------------------------------------
/// Read the user's choice of clock mode.
//-----------------------------------------------------------------------------
void RDPSettingsReader::ReadUserClockMode()
{
    DevDriver::DriverControlProtocol::DeviceClockMode clockMode = DevDriver::DriverControlProtocol::DeviceClockMode::Default;

    // Read the clock mode string, and then convert the string to an actual clock mode.
    const QString userClockModeString = m_reader.readElementText();

    if (userClockModeString.compare(gs_CLOCKS_MODE_NAME_TEXT_NORMAL) == 0)
    {
        clockMode = DevDriver::DriverControlProtocol::DeviceClockMode::Default;
    }
    else if (userClockModeString.compare(gs_CLOCKS_MODE_NAME_TEXT_STABLE) == 0)
    {
        clockMode = DevDriver::DriverControlProtocol::DeviceClockMode::Profiling;
    }

    m_pRdpSettings->SetUserClockMode(clockMode);
}
