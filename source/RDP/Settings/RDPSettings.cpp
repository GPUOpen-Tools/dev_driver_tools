//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation for RDP settings.
//=============================================================================

#include <chrono>
#include <ctime>

#include <QDir>
#include <QFile>
#include <QTextStream>

#include "RDPSettings.h"
#include "RDPSettingsReader.h"
#include "RDPSettingsWriter.h"
#include "../RDPDefinitions.h"
#include "../AppSettings/ApplicationSettingsFileWriter.h"
#include "../AppSettings/ApplicationSettingsFileReader.h"
#include "../Util/RDPUtil.h"

/// Single instance of the RDPSettings
static RDPSettings s_rdpSettings;

/// The maximum number of recent files allowed
static const int MAX_RECENT_FILES = 10;

/// The executables in this list won't be processed by RDP when started.
static const char* kBlacklistedExecutables[] =
{
    "svchost.exe",
    "RadeonSettings.exe",
    "taskhost.exe",
    "taskhostw.exe"
};

//-----------------------------------------------------------------------------
/// \return a reference to the RDPSettings
//-----------------------------------------------------------------------------
RDPSettings& RDPSettings::Get()
{
    return s_rdpSettings;
}

//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
RDPSettings::RDPSettings()
{
    InitDefaultSettings();
}

//-----------------------------------------------------------------------------
/// Add a setting to our active map if it is recognized
/// \param setting The setting to add
//-----------------------------------------------------------------------------
void RDPSettings::AddPotentialSetting(const QString& name, const QString& value)
{
    for (RDPSettingsMap::iterator i = m_defaultSettings.begin(); i != m_defaultSettings.end(); ++i)
    {
        if (i.value().name.compare(name) == 0)
        {
            AddActiveSetting(i.key(), { name, value });
            break;
        }
    }
}

//-----------------------------------------------------------------------------
/// Create a new Application Settings file.
/// \returns A new RDPRecentAppSettings instance with the file information.
//-----------------------------------------------------------------------------
RDPApplicationSettingsFile* RDPSettings::CreateAppSettingsFile()
{
    static std::time_t lastTimestamp;

    RDPApplicationSettingsFile* pAppSettings = new RDPApplicationSettingsFile;

    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    std::time_t now = std::chrono::system_clock::to_time_t(tp);
    pAppSettings->lastAccessed = QString::number(now);
    pAppSettings->createdTimestamp = QString::number(now);

    // Generate a suitable path for the settings file.
    QString appSettingsFilepath = ToolUtil::GetDriverToolsXmlFileLocation();
    appSettingsFilepath.append(QDir::separator());
    appSettingsFilepath.append(gs_APPLICATION_SETTINGS_DIRECTORY);
    appSettingsFilepath.append(QDir::separator());

    // Guarantee that the name is unique - generated names may be the same if this function is
    // called multiple times before one clock tick passes
    if (now <= lastTimestamp)
    {
        now = lastTimestamp + 1;
    }

    // Since we don't know the app name yet, generate a unique filename for it.
    appSettingsFilepath.append("App");
    const QString timestamp = QString::number(now);
    appSettingsFilepath.append(timestamp);
    appSettingsFilepath.append(".rds");

    pAppSettings->filepath = appSettingsFilepath;

    // Add this to the "recent list" so we know to re-open it later.
    AddAppSettingsFile(pAppSettings);

    lastTimestamp = now;

    return pAppSettings;
}

//-----------------------------------------------------------------------------
/// Write an application-specific settings file.
/// \param pFileInfo The file info for the settings file to write.
/// \param pSettingsFile The application settings file data.
//-----------------------------------------------------------------------------
void RDPSettings::WriteApplicationSettingsFile(ApplicationSettingsFile* pSettingsFile)
{
    bool success = false;

    if (pSettingsFile != nullptr)
    {
        RDPApplicationSettingsFile* pFileInfo = pSettingsFile->GetFileInfo();
        if (pFileInfo != nullptr)
        {
            // Make sure that the directory for the file is created before writing.
            QFileInfo fileInfo(pFileInfo->filepath);
            QDir filepathDir = fileInfo.absoluteDir();

            bool pathCreated = true;
            if (!filepathDir.exists())
            {
                pathCreated = filepathDir.mkdir(filepathDir.absolutePath());
            }

            if (pathCreated)
            {
                QFile file(pFileInfo->filepath);
                success = file.open(QFile::WriteOnly | QFile::Text);
                if (success == true)
                {
                    ApplicationSettingsFileWriter fileWriter(pSettingsFile);
                    success = fileWriter.Write(&file);
                }
            }
        }
    }

    if (!success)
    {
        RDPUtil::DbgMsg("[RDP] Failed to write application settings file.");
    }
}

//-----------------------------------------------------------------------------
/// Write an application specific settings file.
/// \param pFileInfo The info for the application settings file to read.
/// \returns A new ApplicationSettingsFile instance, or nullptr if reading the file failed.
//-----------------------------------------------------------------------------
ApplicationSettingsFile* RDPSettings::ReadApplicationSettingsFile(RDPApplicationSettingsFile* pFileInfo)
{
    ApplicationSettingsFile* pApplicationSettingsFile = nullptr;

    if (pFileInfo != nullptr)
    {
        // Attempt to load the recent file.
        QFile file(pFileInfo->filepath);
        bool readSettingsFile = file.open(QFile::ReadOnly | QFile::Text);

        if (readSettingsFile)
        {
            pApplicationSettingsFile = new ApplicationSettingsFile;
            pApplicationSettingsFile->SetFileInfo(pFileInfo);

            // Deserialize the XML settings.
            ApplicationSettingsFileReader appSettingsFileReader(pApplicationSettingsFile);
            readSettingsFile = appSettingsFileReader.Read(&file);

            if (!readSettingsFile)
            {
                // Failed to read the file, so return null.
                SAFE_DELETE(pApplicationSettingsFile);
            }
        }
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Failed to read application settings file.");
    }

    return pApplicationSettingsFile;
}

//-----------------------------------------------------------------------------
/// Remove an application settings file. It won't be reloaded automatically.
/// \param settingFilename The filename of the application settings file to close.
//-----------------------------------------------------------------------------
void RDPSettings::CloseAppSettingsFile(const QString& settingFilename)
{
    RemoveRecentFile(settingFilename.toStdString().c_str());
}

//-----------------------------------------------------------------------------
/// Add new connection information to be serialized in the recent connections list.
/// \param connectionInfo A structure containing all the info required to connect to RDS.
/// \return true if connection added successfully, false otherwise (connection
/// already listed)
//-----------------------------------------------------------------------------
bool RDPSettings::AddRecentConnection(const RDSConnectionInfo& connectionInfo)
{
    for (auto& item : m_recentConnections)
    {
        if (item.ipString.compare(connectionInfo.ipString) == 0 && item.port == connectionInfo.port)
        {
            return false;
        }
    }

    m_recentConnections.push_back(connectionInfo);
    SaveSettings();
    return true;
}

//-----------------------------------------------------------------------------
/// Remove a connection from the recent connections list.
/// \param connectionIndex The index of the connection to remove.
/// \returns True when the row was removed successfully, and false if it wasn't.
//-----------------------------------------------------------------------------
bool RDPSettings::RemoveRecentConnection(int connectionIndex)
{
    bool validIndex = connectionIndex >= 0 && connectionIndex < m_recentConnections.size();
    Q_ASSERT(validIndex == true);
    if (validIndex)
    {
        RecentConnectionVector::iterator eraseIter = m_recentConnections.begin() + connectionIndex;
        m_recentConnections.erase(eraseIter);
        SaveSettings();

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
/// Add new target application information to be serialized in the target
/// applications list.
/// \param applicationInfo A structure containing all the target application
/// info.
/// \param bAddToEnd True adds the item to the end of the list.
///  Otherwise item is added to front of list.
//-----------------------------------------------------------------------------
void RDPSettings::AddTargetApplication(const RDSTargetApplicationInfo& applicationInfo, bool addToEnd)
{
    if (addToEnd)
    {
        m_targetApplications.push_back(applicationInfo);
    }
    else
    {
        m_targetApplications.push_front(applicationInfo);
    }
}

//-----------------------------------------------------------------------------
/// Remove an existing target application from the user's application table.
/// \param rowIndex The index of the row to be removed from the table.
//-----------------------------------------------------------------------------
void RDPSettings::RemoveTargetApplication(int rowIndex)
{
    TargetApplicationVector::iterator eraseIter = m_targetApplications.begin() + rowIndex;
    m_targetApplications.erase(eraseIter);
}

//-----------------------------------------------------------------------------
/// Allow the application at the given index to be profiled. Disable profiling
/// on all other apps (only 1 app can be profiled at a time)
/// \param index The array index to be profiled
/// \param checked Whether the checkbox at index is checked or not
//-----------------------------------------------------------------------------
void RDPSettings::AllowTargetApplicationProfiling(int index, bool checked)
{
    Q_ASSERT(index < m_targetApplications.size());

    // go through the array and clear out the old profile state
    for (auto& item : m_targetApplications)
    {
        item.allowProfiling = false;
    }

    // apply this profile state to the array using the row as the index
    m_targetApplications[index].allowProfiling = checked;
}

//-----------------------------------------------------------------------------
/// Set the 'apply settings' value when the checkbox state is changed
/// \param index The array index that was changed
/// \param checked Whether the checkbox at index is checked or not
//-----------------------------------------------------------------------------
void RDPSettings::ApplyDriverSettingsState(int index, bool checked)
{
    Q_ASSERT(index < m_targetApplications.size());

    m_targetApplications[index].applySettings = checked;
}

// ---------------------------------------------------------------------------- -
/// Return the application profiling setting
/// \param index the index of the row in application table
/// \return the value for this setting
//-----------------------------------------------------------------------------
bool RDPSettings::isAllowTargetApplicationProfiling(int index)
{
    Q_ASSERT(index < m_targetApplications.size());

    // apply this profile state to the array using the row as the index
    return m_targetApplications[index].allowProfiling;
}

//-----------------------------------------------------------------------------
/// Return the driver settings setting
/// \param index the index of the row in application table
/// \return the value for this setting
//-----------------------------------------------------------------------------
bool RDPSettings::isApplyDriverSettingsState(int index)
{
    Q_ASSERT(index < m_targetApplications.size());

    return m_targetApplications[index].applySettings;
}

//-----------------------------------------------------------------------------
/// Check if the given process name is on the process name blacklist.
/// \param processName The name of the process to check.
/// \return True if the process name is on the blacklist, false otherwise.
//-----------------------------------------------------------------------------
bool RDPSettings::CheckBlacklistMatch(const QString& processName)
{
    return m_processBlacklist.contains(processName);
}

//-----------------------------------------------------------------------------
/// Apply default settings and then override them if found on disk.
/// \return Returns true if settings were read from file, and false otherwise.
//-----------------------------------------------------------------------------
bool RDPSettings::LoadSettings()
{
    // Begin by applying the defaults
    for (RDPSettingsMap::iterator i = m_defaultSettings.begin(); i != m_defaultSettings.end(); ++i)
    {
        AddPotentialSetting(i.value().name, i.value().value);
    }

    QString settingsFilepath = ToolUtil::GetDriverToolsXmlFileLocation();
    settingsFilepath.append(QDir::separator());
    settingsFilepath.append(gs_PRODUCT_SETTINGS_FILENAME);

    QFile file(settingsFilepath);
    bool readSettingsFile = file.open(QFile::ReadOnly | QFile::Text);

    // Override the defaults
    if (readSettingsFile)
    {
        RDPSettingsReader xmlReader(&RDPSettings::Get());

        readSettingsFile = xmlReader.Read(&file);

        // Make sure the XML parse worked
        Q_ASSERT(readSettingsFile == true);
        if (!readSettingsFile)
        {
            RDPUtil::DbgMsg("[RDP] Detected malformed settings file.");
        }
    }

    // Save the file
    else
    {
        SaveSettings();

        readSettingsFile = false;
    }

    LoadProcessBlacklist();

    return readSettingsFile;
}

//-----------------------------------------------------------------------------
/// Load the process blacklist file.
//-----------------------------------------------------------------------------
void RDPSettings::LoadProcessBlacklist()
{
    // Get filepath
    QString blacklistFilepath = ToolUtil::GetDriverToolsXmlFileLocation();
    blacklistFilepath.append(QDir::separator());
    blacklistFilepath.append(gs_PROCESS_NAME_BLACKLIST_FILENAME);

    // Blacklist file object
    QFile blacklistFile(blacklistFilepath);
    QTextStream fileStream(&blacklistFile);

    bool fileOpened = false;
    if (blacklistFile.exists())
    {
        // File exists - open for reading
        fileOpened = blacklistFile.open(QFile::ReadOnly | QFile::Text);
    }
    else
    {
        // File doesn't exist - open for read/write
        fileOpened = blacklistFile.open(QFile::ReadWrite | QFile::Text);

        // Write default blacklist data
        if (fileOpened)
        {
            int numBlacklistedExecutables = sizeof(kBlacklistedExecutables) / sizeof(kBlacklistedExecutables[0]);

            for (int exeIndex = 0; exeIndex < numBlacklistedExecutables; ++exeIndex)
            {
                fileStream << kBlacklistedExecutables[exeIndex];

                // Append a newline after each executable.
                if (exeIndex < (numBlacklistedExecutables - 1))
                {
                    fileStream << "\n";
                }
            }

            // Rewind the filestream to the beginning after writing all of the blacklisted executables.
            fileStream.seek(0);
        }
    }

    if (fileOpened)
    {
        // Read and store all process names
        QString line = fileStream.readLine();
        while (!(line.isNull() || line.isEmpty()))
        {
            m_processBlacklist.push_back(line);
            line = fileStream.readLine();
        }
    }
}

//-----------------------------------------------------------------------------
/// Save the settings (and list of recent files) to disk
//-----------------------------------------------------------------------------
void RDPSettings::SaveSettings()
{
    QString settingsFilepath = ToolUtil::GetDriverToolsXmlFileLocation();
    settingsFilepath.append(QDir::separator());
    settingsFilepath.append(gs_PRODUCT_SETTINGS_FILENAME);

    QFile file(settingsFilepath);
    bool success = file.open(QFile::WriteOnly | QFile::Text);

    if (success == true)
    {
        RDPSettingsWriter xmlWriter(&RDPSettings::Get());
        success = xmlWriter.Write(&file);
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Failed to open settings file for saving.");
    }

    Q_ASSERT(success == true);
}

//-----------------------------------------------------------------------------
/// Initialize our table with default settings.
//-----------------------------------------------------------------------------
void RDPSettings::InitDefaultSettings()
{
    // Window size and position.
    m_defaultSettings[RDP_SETTING_MAIN_WINDOW_WIDTH] =  { "WindowWidth", "0" };
    m_defaultSettings[RDP_SETTING_MAIN_WINDOW_HEIGHT] = { "WindowHeight", "0" };
    m_defaultSettings[RDP_SETTING_MAIN_WINDOW_X_POS] =  { "WindowXPos", "100" };
    m_defaultSettings[RDP_SETTING_MAIN_WINDOW_Y_POS] =  { "WindowYPos", "100" };

    // Connection Settings interface.
    m_defaultSettings[RDP_SETTING_CONNECTION_HOST_STRING] = { "RDSHost", QString(gs_DEFAULT_SERVER_HOST) };
    m_defaultSettings[RDP_SETTING_CONNECTION_PORT] = { "RDSPort", QString::number(gs_DEFAULT_CONNECTION_PORT) };

    // A setting to track if the user checked the "Don't show me this notification again" checkbox when confirming a disconnect.
    m_defaultSettings[RDP_SETTING_CONNECTION_SHOW_CONFIRMATION_WHEN_DISCONNECTING] = { "ShowDisconnectConfirmation", QString(gs_TRUE_TEXT) };

    QString appSettingsFilepath = ToolUtil::GetDriverToolsXmlFileLocation();
    m_defaultSettings[RDP_SETTING_LAST_APPLICATION_PATH] = { "LastApplicationPath",  appSettingsFilepath };

    const QString& traceOutputPath = GetDefaultTraceOutputPath();

    // RGP's instruction-level tracing is disabled by default.
    m_defaultSettings[RDP_SETTING_RGP_DETAILED_INSTRUCTION_DATA] = { "RGPDetailedData", QString(gs_FALSE_TEXT) };
    m_defaultSettings[RDP_SETTING_RGP_ALLOW_COMPUTE_PRESENTS] = { "RGPAllowComputePresents", QString(gs_FALSE_TEXT) };
    m_defaultSettings[RDP_SETTING_RGP_TRACE_OUTPUT_PATH_STRING] = { "RGPOutputPath", traceOutputPath };

    // Build a path to the default RGP executable location. Start in "This directory" where RDP is running.
    QString defaultRGPPath = RDPUtil::GetDefaultRGPPath();

    m_defaultSettings[RDP_SETTING_RGP_PATH_STRING] = { "PathToRGP", defaultRGPPath };

    m_userClockMode = DevDriver::DriverControlProtocol::DeviceClockMode::Default;
}

//-----------------------------------------------------------------------------
/// Store an active setting
/// \param settingId The identifier for this setting
/// \param setting The setting containing name and value
//-----------------------------------------------------------------------------
void RDPSettings::AddActiveSetting(RDPSettingID settingId, const RDPSetting& setting)
{
    m_activeSettings[settingId] = setting;
}

//-----------------------------------------------------------------------------
/// Get a setting as a boolean value
/// \param settingId The identifier for this setting
/// \return The boolean value for the setting specified
//-----------------------------------------------------------------------------
bool RDPSettings::GetBoolValue(RDPSettingID settingId) const
{
    return (m_activeSettings[settingId].value.compare("True") == 0) ? true : false;
}

//-----------------------------------------------------------------------------
/// Get a setting as an integer value
/// \param settingId The identifier for this setting
/// \return The integer value for the setting specified
//-----------------------------------------------------------------------------
int RDPSettings::GetIntValue(RDPSettingID settingId) const
{
    return m_activeSettings[settingId].value.toInt();
}

//-----------------------------------------------------------------------------
/// Get a setting as a QColor object
/// \param settingId The identifier for this setting
/// \return The color value for the setting specified
//-----------------------------------------------------------------------------
QColor RDPSettings::GetColorValue(RDPSettingID settingId) const
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
QString RDPSettings::GetStringValue(RDPSettingID settingId) const
{
    return m_activeSettings[settingId].value;
}

//-----------------------------------------------------------------------------
/// Set a setting as a boolean value
/// \param settingId The identifier for this setting
/// \param value The new value of the setting
//-----------------------------------------------------------------------------
void RDPSettings::SetBoolValue(RDPSettingID settingId, const bool value)
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
/// Set a setting as an integer value
/// \param settingId The identifier for this setting
/// \param value The new value of the setting
//-----------------------------------------------------------------------------
void RDPSettings::SetIntValue(RDPSettingID settingId, const int value)
{
    AddPotentialSetting(m_defaultSettings[settingId].name, QString::number(value));
}

//-----------------------------------------------------------------------------
/// Set a setting as a QColor value
/// \param settingId The identifier for this setting
/// \param value The new value of the setting
//-----------------------------------------------------------------------------
void RDPSettings::SetColorValue(RDPSettingID settingId, const QColor& value)
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
void RDPSettings::SetStringValue(RDPSettingID settingId, const QString& value)
{
    AddPotentialSetting(m_defaultSettings[settingId].name, value);
}

//-----------------------------------------------------------------------------
/// Remove a file from the recent files list
/// \param pFileName The name of the file to remove
//-----------------------------------------------------------------------------
void RDPSettings::RemoveRecentFile(const QString& fileName)
{
    const int numRecentFiles = this->m_recentAppSettingsFiles.size();
    for (int loop = 0; loop < numRecentFiles; loop++)
    {
        if (m_recentAppSettingsFiles[loop]->filepath.compare(fileName) == 0)
        {
            m_recentAppSettingsFiles.remove(loop);
            break;
        }
    }

    SaveSettings();
}

//-----------------------------------------------------------------------------
/// Get window width from the settings
/// \return The main window width.
//-----------------------------------------------------------------------------
int RDPSettings::GetWindowWidth()
{
    return GetIntValue(RDP_SETTING_MAIN_WINDOW_WIDTH);
}

//-----------------------------------------------------------------------------
/// Get window height from the settings
/// \return The main window height.
//-----------------------------------------------------------------------------
int RDPSettings::GetWindowHeight()
{
    return GetIntValue(RDP_SETTING_MAIN_WINDOW_HEIGHT);
}

//-----------------------------------------------------------------------------
/// Get window's X screen position from the settings
/// \return The window's X screen position.
//-----------------------------------------------------------------------------
int RDPSettings::GetWindowXPos()
{
    return GetIntValue(RDP_SETTING_MAIN_WINDOW_X_POS);
}

//-----------------------------------------------------------------------------
/// Get window's Y screen position from the settings
/// \return The window's Y screen position.
//-----------------------------------------------------------------------------
int RDPSettings::GetWindowYPos() const
{
    return GetIntValue(RDP_SETTING_MAIN_WINDOW_Y_POS);
}

//-----------------------------------------------------------------------------
/// Get the port to use when connecting to RDS.
/// \return The port used in the last connection to RDS.
//-----------------------------------------------------------------------------
unsigned int RDPSettings::GetConnectionPort() const
{
    return GetIntValue(RDP_SETTING_CONNECTION_PORT);
}

//-----------------------------------------------------------------------------
/// Get the user's choice to show/hide the disconnect confirmation interface.
/// \return A flag that indicates if the user wishes to see the "Are you sure?" dialog before disconnecting.
//-----------------------------------------------------------------------------
bool RDPSettings::ShowConfirmationWhenDisconnecting() const
{
    return GetBoolValue(RDP_SETTING_CONNECTION_SHOW_CONFIRMATION_WHEN_DISCONNECTING);
}

//-----------------------------------------------------------------------------
/// Get the last saved Host string to connect with RDS.
/// \return A string containing the hostname to connect to.
//-----------------------------------------------------------------------------
QString RDPSettings::GetConnectionHost() const
{
    return GetStringValue(RDP_SETTING_CONNECTION_HOST_STRING);
}

//-----------------------------------------------------------------------------
/// Get the flag that indicates the state of collecting instruction level trace data.
/// \return A true/false that indicates the state of collecting instruction level trace data.
//-----------------------------------------------------------------------------
bool RDPSettings::GetRGPDetailedInstructionData() const
{
    return GetBoolValue(RDP_SETTING_RGP_DETAILED_INSTRUCTION_DATA);
}

//-----------------------------------------------------------------------------
/// Get the flag that indicates if Compute Queues are allowed to present frames.
/// \return A true/false that indicates if compute queues are allowed to present frames.
//-----------------------------------------------------------------------------
bool RDPSettings::GetRGPAllowComputePresents() const
{
    return GetBoolValue(RDP_SETTING_RGP_ALLOW_COMPUTE_PRESENTS);
}

//-----------------------------------------------------------------------------
/// Get the last saved RGP Trace output filepath.
/// \return A string containing the user's selected trace output path.
//-----------------------------------------------------------------------------
QString RDPSettings::GetRGPTraceOutputPath() const
{
    return QDir::toNativeSeparators(GetStringValue(RDP_SETTING_RGP_TRACE_OUTPUT_PATH_STRING));
}

//-----------------------------------------------------------------------------
/// Generate the path to where RGP trace files will be output to by default.
/// \returns The default RGP trace output directory.
//-----------------------------------------------------------------------------
QString RDPSettings::GetDefaultTraceOutputPath() const
{
    // Default traces default to being dumped to a named folder alongside RDP.
    const QString rdpPath = QDir::currentPath();

    // Create the "Traces" folder alongside the executable.
    QDir traceFilepath(rdpPath);
    traceFilepath.mkdir(gs_DEFAULT_TRACE_DIRECTORY);
    traceFilepath.cd(gs_DEFAULT_TRACE_DIRECTORY);

    QString fullOutputPath = traceFilepath.toNativeSeparators(traceFilepath.absolutePath());

    return fullOutputPath;
}

//-----------------------------------------------------------------------------
/// Get the last saved RGP Trace output filepath.
/// \return A string containing the last target executable directory the user browsed to.
//-----------------------------------------------------------------------------
QString RDPSettings::GetLastTargetExecutableDirectory() const
{
    return GetStringValue(RDP_SETTING_LAST_APPLICATION_PATH);
}

//-----------------------------------------------------------------------------
/// Get the path to the local RGP install executable.
/// \return A string containing the path to the local RGP executable.
//-----------------------------------------------------------------------------
QString RDPSettings::GetPathToRGP() const
{
    return QDir::toNativeSeparators(GetStringValue(RDP_SETTING_RGP_PATH_STRING));
}

//-----------------------------------------------------------------------------
/// Get the clock mode chosen by the user in the clocks interface.
/// \return The clock mode that RDP will set for each new running target application.
//-----------------------------------------------------------------------------
DevDriver::DriverControlProtocol::DeviceClockMode RDPSettings::GetUserClockMode() const
{
    // Verify that the given clock mode is valid.
    Q_ASSERT(static_cast<int>(m_userClockMode) < static_cast<int>(DevDriver::DriverControlProtocol::DeviceClockMode::Count));
    return m_userClockMode;
}

//-----------------------------------------------------------------------------
/// Sets the size of the window (width and height) in the settings
/// \param width The new width
/// \param height The new height
//-----------------------------------------------------------------------------
void RDPSettings::SetWindowSize(const int width, const int height)
{
    AddPotentialSetting(m_defaultSettings[RDP_SETTING_MAIN_WINDOW_WIDTH].name, QString::number(width));
    AddPotentialSetting(m_defaultSettings[RDP_SETTING_MAIN_WINDOW_HEIGHT].name, QString::number(height));
    SaveSettings();
}

//-----------------------------------------------------------------------------
/// Sets the position of the window on the screen in the settings
/// \param xPos The new X position
/// \param yPos The new Y Position
//-----------------------------------------------------------------------------
void RDPSettings::SetWindowPos(const int xPos, const int yPos)
{
    AddPotentialSetting(m_defaultSettings[RDP_SETTING_MAIN_WINDOW_X_POS].name, QString::number(xPos));
    AddPotentialSetting(m_defaultSettings[RDP_SETTING_MAIN_WINDOW_Y_POS].name, QString::number(yPos));
    SaveSettings();
}

//-----------------------------------------------------------------------------
/// Sets the new default connection port that will be used to connect to RDS.
/// \param port The port used to connect to RDS.
//-----------------------------------------------------------------------------
void RDPSettings::SetConnectionPort(const QString& portString)
{
    AddPotentialSetting(m_defaultSettings[RDP_SETTING_CONNECTION_PORT].name, portString);
    SaveSettings();
}

//-----------------------------------------------------------------------------
/// Sets the value of the "Show disconnect confirmation" flag.
/// \param showConfirmation The flag that determines if the "Are you sure you want to disconnect" confirmation will appear.
//-----------------------------------------------------------------------------
void RDPSettings::SetShowDisconnectConfirmation(bool showConfirmation)
{
    const QString valString = showConfirmation ? gs_TRUE_TEXT : gs_FALSE_TEXT;
    AddPotentialSetting(m_defaultSettings[RDP_SETTING_CONNECTION_SHOW_CONFIRMATION_WHEN_DISCONNECTING].name, valString);
    SaveSettings();
}

//-----------------------------------------------------------------------------
/// Sets the new default connection port that will be used to connect to RDS.
/// \param hostString The host string used to connect to RDS.
//-----------------------------------------------------------------------------
void RDPSettings::SetConnectionHost(const QString& hostString)
{
    AddPotentialSetting(m_defaultSettings[RDP_SETTING_CONNECTION_HOST_STRING].name, hostString);
    SaveSettings();
}

//-----------------------------------------------------------------------------
/// Sets the "detailed instruction data" flag used when collecting an RGP trace.
/// \param detailedData A true/false indicating the user's profiling detail level.
//-----------------------------------------------------------------------------
void RDPSettings::SetRGPDetailedInstructionData(bool detailedData)
{
    const QString valString = detailedData ? gs_TRUE_TEXT : gs_FALSE_TEXT;
    AddPotentialSetting(m_defaultSettings[RDP_SETTING_RGP_DETAILED_INSTRUCTION_DATA].name, valString);
    SaveSettings();
}

//-----------------------------------------------------------------------------
/// Sets the "allow compute presents" flag used when collecting an RGP trace.
/// \param allowComputePresents A true/false indicating if frames can be presented with a compute queue.
//-----------------------------------------------------------------------------
void RDPSettings::SetRGPAllowComputePresents(bool allowComputePresents)
{
    const QString valString = allowComputePresents ? gs_TRUE_TEXT : gs_FALSE_TEXT;
    AddPotentialSetting(m_defaultSettings[RDP_SETTING_RGP_ALLOW_COMPUTE_PRESENTS].name, valString);
    SaveSettings();
}

//-----------------------------------------------------------------------------
/// Sets the new default RGP Trace output path where new traces will be dumped.
/// \param tracePath The filepath where all new RGP traces will be dumped.
//-----------------------------------------------------------------------------
void RDPSettings::SetRGPTraceOutputPath(const QString& tracePath)
{
    AddPotentialSetting(m_defaultSettings[RDP_SETTING_RGP_TRACE_OUTPUT_PATH_STRING].name, tracePath);
    SaveSettings();
}

//-----------------------------------------------------------------------------
/// Sets the new default RGP Trace output path where new traces will be dumped.
/// \param tracePath The filepath where all new RGP traces will be dumped.
//-----------------------------------------------------------------------------
void RDPSettings::SetLastTargetExecutableDirectory(const QString& executableDirectory)
{
    AddPotentialSetting(m_defaultSettings[RDP_SETTING_LAST_APPLICATION_PATH].name, executableDirectory);
    SaveSettings();
}

//-----------------------------------------------------------------------------
/// Sets the path to the RGP executable filename on the local machine.
/// \param tracePath The full filepath to the local RGP install's executable.
//-----------------------------------------------------------------------------
void RDPSettings::SetPathToRGP(const QString& rgpPathString)
{
    AddPotentialSetting(m_defaultSettings[RDP_SETTING_RGP_PATH_STRING].name, rgpPathString);
    SaveSettings();
}

//-----------------------------------------------------------------------------
/// Sets the clock mode to be used during normal operation while RDP is connected.
/// Value is chosen by the user within the clocks interface, and is overridden while collecting a trace.
/// \param clockMode The clock mode to set for the GPU.
//-----------------------------------------------------------------------------
void RDPSettings::SetUserClockMode(DevDriver::DriverControlProtocol::DeviceClockMode clockMode)
{
    m_userClockMode = clockMode;
    SaveSettings();
}
