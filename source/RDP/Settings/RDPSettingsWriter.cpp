//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of RDP's XML settings writer.
//=============================================================================

#include "RDPSettingsWriter.h"
#include "../RDPDefinitions.h"
#include "../Util/RDPUtil.h"

//-----------------------------------------------------------------------------
/// Explicit constructor.
/// \param pRdpSettings Output settings class.
//-----------------------------------------------------------------------------
RDPSettingsWriter::RDPSettingsWriter(RDPSettings* pRdpSettings) :
    m_pRdpSettings(pRdpSettings)
{
}

//-----------------------------------------------------------------------------
/// Begin writing RDP XML file and make sure it's valid.
/// \param pDevice The XML file represented by a Qt IO device.
//-----------------------------------------------------------------------------
bool RDPSettingsWriter::Write(QIODevice* pDevice)
{
    m_writer.setDevice(pDevice);
    m_writer.setAutoFormatting(true);

    m_writer.writeStartDocument();
    m_writer.writeStartElement("RDP");

    WriteSettingsAndRecents();

    m_writer.writeEndElement();
    m_writer.writeEndDocument();

    return m_writer.hasError() == false;
}

//-----------------------------------------------------------------------------
/// Detect global settings and recently used files section.
//-----------------------------------------------------------------------------
void RDPSettingsWriter::WriteSettingsAndRecents()
{
    m_writer.writeStartElement("GlobalSettings");
    WriteSettings();
    m_writer.writeEndElement();

    m_writer.writeStartElement("RecentFiles");
    WriteRecentSettingsFiles();
    m_writer.writeEndElement();

    m_writer.writeStartElement("RecentConnections");
    WriteRecentConnections();
    m_writer.writeEndElement();

    m_writer.writeStartElement("TargetApplications");
    WriteTargetApplications();
    m_writer.writeEndElement();

    WriteUserClockMode();
}

//-----------------------------------------------------------------------------
/// Detect setting list.
//-----------------------------------------------------------------------------
void RDPSettingsWriter::WriteSettings()
{
    RDPSettingsMap& settings = m_pRdpSettings->Settings();

    for (RDPSettingsMap::iterator i = settings.begin(); i != settings.end(); ++i)
    {
        m_writer.writeStartElement("Setting");
        WriteSetting(i.value());
        m_writer.writeEndElement();
    }
}

//-----------------------------------------------------------------------------
/// Write individual settings.
/// \param setting The RDPSetting structure to write out
//-----------------------------------------------------------------------------
void RDPSettingsWriter::WriteSetting(const RDPSetting& setting)
{
    m_writer.writeTextElement("Name", setting.name);
    m_writer.writeTextElement("Value", setting.value);
}

//-----------------------------------------------------------------------------
/// Detect recently opened file list.
//-----------------------------------------------------------------------------
void RDPSettingsWriter::WriteRecentSettingsFiles()
{
    const QVector<RDPApplicationSettingsFile*>& recentFiles = m_pRdpSettings->RecentFiles();

    for (int loop = 0; loop < recentFiles.size(); loop++)
    {
        m_writer.writeStartElement("RecentFile");
        RDPApplicationSettingsFile* pFile = recentFiles.at(loop);
        WriteRecentAppSettingsFile(*pFile);
        m_writer.writeEndElement();
    }
}

//-----------------------------------------------------------------------------
/// Write individual recently opened files.
/// \param recentFile The name of the file to write
//-----------------------------------------------------------------------------
void RDPSettingsWriter::WriteRecentAppSettingsFile(const RDPApplicationSettingsFile& recentFile)
{
    m_writer.writeTextElement("Path", recentFile.filepath);
    m_writer.writeTextElement("Created", recentFile.createdTimestamp);
    m_writer.writeTextElement("Accessed", recentFile.lastAccessed);
}

//-----------------------------------------------------------------------------
/// Write all recent connection info to the RDP settings file.
//-----------------------------------------------------------------------------
void RDPSettingsWriter::WriteRecentConnections()
{
    // Retrieve the vector of all recent connections and write each to the settings file.
    const RecentConnectionVector& recentConnections = m_pRdpSettings->GetRecentConnections();

    RecentConnectionVector::const_iterator connectionIter;
    for (connectionIter = recentConnections.begin(); connectionIter != recentConnections.end(); ++connectionIter)
    {
        m_writer.writeStartElement("Connection");
        WriteRecentConnection(*connectionIter);
        m_writer.writeEndElement();
    }
}

//-----------------------------------------------------------------------------
/// Write and individual Recent Connection Info structure to the RDP settings file.
/// \param connectionInfo The connection info to serialize to the settings file.
//-----------------------------------------------------------------------------
void RDPSettingsWriter::WriteRecentConnection(const RDSConnectionInfo& connectionInfo)
{
    m_writer.writeTextElement("Hostname",       connectionInfo.hostnameString);

    // NOTE: This code is added to make the "localhost" string
    // backwards compatible. In a future pass, this will be within
    // an if statement to check the version number of the settings
    // file before executing this part of the code.
    if (connectionInfo.ipString.compare(gs_LOCAL_HOST_IP) == 0)
    {
        m_writer.writeTextElement("IP", gs_LOCAL_HOST);
    }
    else
    {
        m_writer.writeTextElement("IP", connectionInfo.ipString);
    }
    m_writer.writeTextElement("Port",           QString::number(connectionInfo.port));
    m_writer.writeTextElement("Autoconnect",    QString::number(connectionInfo.autoconnect));
}

//-----------------------------------------------------------------------------
/// Write the list of Target Applications to the RDP settings file.
//-----------------------------------------------------------------------------
void RDPSettingsWriter::WriteTargetApplications()
{
    // Retrieve the vector of all target applications and write each to the settings file.
    const TargetApplicationVector& targetApps = m_pRdpSettings->GetTargetApplications();

    TargetApplicationVector::const_iterator targetAppIter;
    for (targetAppIter = targetApps.begin(); targetAppIter != targetApps.end(); ++targetAppIter)
    {
        m_writer.writeStartElement("Target");
        WriteTargetApplication(*targetAppIter);
        m_writer.writeEndElement();
    }
}

//-----------------------------------------------------------------------------
/// Write a single Target Application to the RDP settings file.
/// \param applicationInfo The Target Application info to write to the settings file.
//-----------------------------------------------------------------------------
void RDPSettingsWriter::WriteTargetApplication(const RDSTargetApplicationInfo& applicationInfo)
{
    m_writer.writeTextElement("ProcessName",    applicationInfo.processName);
    m_writer.writeTextElement("AppTitle",       applicationInfo.titleName);
    m_writer.writeTextElement("API",            applicationInfo.apiName);
    m_writer.writeTextElement("ApplySettings",  QString::number(applicationInfo.applySettings));
    m_writer.writeTextElement("AllowProfiling", QString::number(applicationInfo.allowProfiling));
}

//-----------------------------------------------------------------------------
/// Write the user's choice of clock mode as a string.
//-----------------------------------------------------------------------------
void RDPSettingsWriter::WriteUserClockMode()
{
    // Default the clock mode to normal if there's an unrecognized mode about to be written.
    QString clockModeString = gs_CLOCKS_MODE_NAME_TEXT_NORMAL;

    DevDriver::DriverControlProtocol::DeviceClockMode clockMode = RDPSettings::Get().GetUserClockMode();
    switch (clockMode)
    {
    case DevDriver::DriverControlProtocol::DeviceClockMode::Default:
        {
            clockModeString = gs_CLOCKS_MODE_NAME_TEXT_NORMAL;
        }
        break;
    case DevDriver::DriverControlProtocol::DeviceClockMode::Profiling:
        {
            clockModeString = gs_CLOCKS_MODE_NAME_TEXT_STABLE;
        }
        break;
    case DevDriver::DriverControlProtocol::DeviceClockMode::Peak:
        {
        }
        break;
    default:
        // If this happens, clock enumerations are outdated and must be updated in RDP.
        clockMode = DevDriver::DriverControlProtocol::DeviceClockMode::Default;
    }

    m_writer.writeTextElement("UserClockMode", clockModeString);
}
