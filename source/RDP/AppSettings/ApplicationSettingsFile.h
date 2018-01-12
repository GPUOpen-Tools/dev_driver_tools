//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Declaration of an application-specific settings file.
//=============================================================================

#ifndef _APP_SETTINGS_FILE_H_
#define _APP_SETTINGS_FILE_H_

#include <QVector>
#include <QMap>
#include "../../DevDriverComponents/inc/protocols/settingsClient.h"

struct RDPApplicationSettingsFile;

namespace DevDriver
{
    namespace SettingsProtocol
    {
        struct SettingValue;
    }
}

/// A list of DriverSettings to serialize to and from file.
typedef QVector<DevDriver::SettingsProtocol::Setting> DriverSettingVector;

/// A map of category name to a vector of driver settings.
typedef QMap<QString, DriverSettingVector > DriverSettingsMap;

/// Support RGP's settings and information about recently opened traces.
class ApplicationSettingsFile
{
public:
    ApplicationSettingsFile();
    ~ApplicationSettingsFile() {}
    void SetTargetExecutableName(const QString& targetName) { m_targetApplicationName = targetName; }
    void AddSetting(const QString& category, const DevDriver::SettingsProtocol::Setting& newSetting);
    bool GetSettingsDelta(ApplicationSettingsFile* pOtherSettingsFile, DriverSettingsMap& differences) const;
    bool GetSettingsMapDelta(const DriverSettingsMap& otherSettingsMap, DriverSettingsMap& differences) const;
    bool UpdateSetting(const QString& category, const DevDriver::SettingsProtocol::Setting& newSetting);
    void SetIsGlobal(bool isGlobal) { m_isGlobal = isGlobal; }
    void CopyFrom(ApplicationSettingsFile* pOtherFile);
    void RestoreToDefaultSettings();

    const QString& GetTargetApplicationName() const { return m_targetApplicationName; }
    const DriverSettingsMap& GetDriverSettings() const { return m_driverSettings; }
    bool IsGlobal() const { return m_isGlobal; }

    void SetFileInfo(RDPApplicationSettingsFile* pFileInfo) { m_pFileInfo = pFileInfo; }
    RDPApplicationSettingsFile* GetFileInfo() const { return m_pFileInfo; }

private:
    bool GetSettingsVectorDelta(const DriverSettingVector& initialSettings, const DriverSettingVector& otherSettings, DriverSettingVector& differences) const;

    QString m_targetApplicationName;            ///< The target application name.
    DriverSettingsMap m_driverSettings;         ///< A vector of all driver settings.
    bool m_isGlobal;                            ///< A flag to indicate if this is the global settings file.
    RDPApplicationSettingsFile* m_pFileInfo;    ///< A pointer to an info structure for this settings file.
};

#endif // _APP_SETTINGS_FILE_H_
