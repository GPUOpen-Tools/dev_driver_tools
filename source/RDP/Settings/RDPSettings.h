//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Define all settings that apply to the Radeon Developer Panel.
//=============================================================================

#ifndef _RDP_SETTINGS_H_
#define _RDP_SETTINGS_H_

#include <QColor>
#include <QVector>
#include <QMap>
#include "../../DevDriverComponents/inc/protocols/driverControlProtocol.h"
#include "../../DevDriverComponents/inc/devDriverClient.h"

class ApplicationSettingsFile;

/// A name/value pair structure used to save and load RDP settings.
struct RDPSetting
{
    QString name;       ///< The key name for the setting.
    QString value;      ///< The value for the setting.
};

/// Metadata used to describe an RDP application settings file.
struct RDPApplicationSettingsFile
{
    QString filepath;           ///< Full path to the file
    QString createdTimestamp;   ///< When was this trace created?
    QString lastAccessed;       ///< When was this trace read?
};

/// Structure containing all data serialized for each row in the Recent Connections table.
struct RDSConnectionInfo
{
    DevDriver::DevDriverClientCreateInfo rdsInfo;  ///< The connection info structure used to connect to RDS.
    QString hostnameString; ///< The host name
    QString ipString;       ///< The IP address or hostname, as a string.
    uint16_t port;          ///< The port number
    bool autoconnect;       ///< Should RDP try to autoconnect to this host
    bool available;         ///< Is this host available
};

/// Structure containing all data serialized for each row in the Setup Target Application table.
struct RDSTargetApplicationInfo
{
    QString processName;    ///< Process name (name of the executable).
    QString titleName;      ///< Title name (the internal full name of the application, if it exists).
    QString apiName;        ///< The API that this application uses.
    bool applySettings;     ///< Are settings to be applied next time round to this application?
    bool allowProfiling;    ///< Is profiling allowed for this application?
};

/// An enumeration that contains Ids for all items that need to be saved in the RDP Settings file.
enum RDPSettingID
{
    // Main RDP window dimensions and position.
    RDP_SETTING_MAIN_WINDOW_WIDTH,
    RDP_SETTING_MAIN_WINDOW_HEIGHT,
    RDP_SETTING_MAIN_WINDOW_X_POS,
    RDP_SETTING_MAIN_WINDOW_Y_POS,

    // RDP connection info to RDS
    RDP_SETTING_CONNECTION_HOST_STRING,
    RDP_SETTING_CONNECTION_PORT,
    RDP_SETTING_CONNECTION_SHOW_CONFIRMATION_WHEN_DISCONNECTING,

    // The last directory the user browsed to for an application.
    RDP_SETTING_LAST_APPLICATION_PATH,

    // RGP tracing settings.
    RDP_SETTING_RGP_DETAILED_INSTRUCTION_DATA,
    RDP_SETTING_RGP_ALLOW_COMPUTE_PRESENTS,
    RDP_SETTING_RGP_TRACE_OUTPUT_PATH_STRING,
    RDP_SETTING_RGP_PATH_STRING,

    RDP_SETTING_COUNT,
};

/// A map type used to associate each setting Id with the corresponding value.
typedef QMap< RDPSettingID, RDPSetting > RDPSettingsMap;

/// A vector containing all of the user's RDS connection target info.
typedef QVector<RDSConnectionInfo> RecentConnectionVector;

/// A vector containing all of the user's target application info.
typedef QVector<RDSTargetApplicationInfo> TargetApplicationVector;

/// A vector containing each recent RDPApplicationSettingsFile metadata structure.
typedef QVector<RDPApplicationSettingsFile*> AppSettingMetadataVector;

/// A class used to hold all of RDP setting names and values.
class RDPSettings
{
public:
    static RDPSettings& Get();

    RDPSettings();

    bool LoadSettings();
    void SaveSettings();

    void AddPotentialSetting(const QString& name, const QString& value);
    RDPApplicationSettingsFile* CreateAppSettingsFile();
    void WriteApplicationSettingsFile(ApplicationSettingsFile* pSettingsFile);
    ApplicationSettingsFile* ReadApplicationSettingsFile(RDPApplicationSettingsFile* pFileInfo);
    void AddAppSettingsFile(RDPApplicationSettingsFile* recentFile) { m_recentAppSettingsFiles.push_back(recentFile); }
    void CloseAppSettingsFile(const QString& settingFilename);

    bool AddRecentConnection(const RDSConnectionInfo& connectionInfo);
    bool RemoveRecentConnection(int connectionIndex);
    const RecentConnectionVector& GetRecentConnections() const { return m_recentConnections; }

    void AddTargetApplication(const RDSTargetApplicationInfo& applicationInfo, bool addToEnd = false);
    void RemoveTargetApplication(int rowIndex);
    void AllowTargetApplicationProfiling(int row, bool checked);
    void ApplyDriverSettingsState(int index, bool checked);
    bool isAllowTargetApplicationProfiling(int index);
    bool isApplyDriverSettingsState(int index);
    TargetApplicationVector GetTargetApplications() const { return m_targetApplications; }

    void LoadProcessBlacklist();
    bool CheckBlacklistMatch(const QString& processName);

    QMap<RDPSettingID, RDPSetting>& Settings() { return m_activeSettings; }
    const QVector<RDPApplicationSettingsFile*>& RecentFiles() { return m_recentAppSettingsFiles; }

    int GetWindowWidth();
    int GetWindowHeight();
    int GetWindowXPos();
    int GetWindowYPos() const;
    unsigned int GetConnectionPort() const;
    bool ShowConfirmationWhenDisconnecting() const;
    QString GetConnectionHost() const;
    bool GetRGPDetailedInstructionData() const;
    bool GetRGPAllowComputePresents() const;
    QString GetRGPTraceOutputPath() const;
    QString GetDefaultTraceOutputPath() const;
    QString GetLastTargetExecutableDirectory() const;
    QString GetPathToRGP() const;
    DevDriver::DriverControlProtocol::DeviceClockMode GetUserClockMode() const;

    void SetWindowSize(const int width, const int height);
    void SetWindowPos(const int xPos, const int yPos);
    void SetConnectionPort(const QString& portString);
    void SetShowDisconnectConfirmation(bool showConfirmation);
    void SetConnectionHost(const QString& hostString);
    void SetRGPDetailedInstructionData(bool detailedData);
    void SetRGPAllowComputePresents(bool allowComputePresents);
    void SetRGPTraceOutputPath(const QString& tracePath);
    void SetLastTargetExecutableDirectory(const QString& executableDirectory);
    void SetPathToRGP(const QString& rgpPathString);
    void SetUserClockMode(DevDriver::DriverControlProtocol::DeviceClockMode clockMode);

private:
    void InitDefaultSettings();
    void AddActiveSetting(RDPSettingID id, const RDPSetting& setting);
    bool GetBoolValue(RDPSettingID settingId) const;
    int GetIntValue(RDPSettingID settingId) const;
    QColor GetColorValue(RDPSettingID settingId) const;
    QString GetStringValue(RDPSettingID settingId) const;
    void SetBoolValue(RDPSettingID settingId, const bool value);
    void SetIntValue(RDPSettingID settingId, const int value);
    void SetColorValue(RDPSettingID settingId, const QColor& value);
    void SetStringValue(RDPSettingID settingId, const QString& value);
    void RemoveRecentFile(const QString& fileName);

    AppSettingMetadataVector    m_recentAppSettingsFiles;       ///< Vector of recently opened files.
    RecentConnectionVector      m_recentConnections;            ///< Vector of recently used connections.
    TargetApplicationVector     m_targetApplications;           ///< Vector of target applications.
    RDPSettingsMap              m_activeSettings;               ///< Map containing active settings.
    RDPSettingsMap              m_defaultSettings;              ///< Map containing default settings.
    QVector<QString>            m_processBlacklist;             ///< Process name blacklist.
    DevDriver::DriverControlProtocol::DeviceClockMode m_userClockMode;  ///< The user's chosen clock mode.
};

#endif // _RDP_SETTINGS_H_
