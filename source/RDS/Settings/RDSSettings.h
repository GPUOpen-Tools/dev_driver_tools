//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Define all settings that apply to the Radeon Developer Service.
//=============================================================================

#ifndef _RDS_SETTINGS_H_
#define _RDS_SETTINGS_H_

#include <QColor>
#include <QMap>

/// A name/value pair structure used to save and load RDS settings.
struct RDSSetting
{
    QString name;       ///< The key name for the setting.
    QString value;      ///< The value for the setting.
};

/// An enumeration that contains Ids for all items that need to be saved in the RDS Settings file.
enum RDSSettingID
{
    RDS_LISTEN_PORT,

    RDS_SETTING_COUNT,
};

/// A map type used to associate each setting Id with the corresponding value.
typedef QMap< RDSSettingID, RDSSetting > RDSSettingsMap;

/// A class used to hold all of RDS's settings.
class RDSSettings
{
public:
    static RDSSettings& Get();

    RDSSettings();

    bool LoadSettings();
    void SaveSettings();

    void AddPotentialSetting(const QString& name, const QString& value);
    QMap<RDSSettingID, RDSSetting>& Settings() { return m_activeSettings; }

    unsigned int GetListenPort() const;
    void SetListenPort(unsigned int listenPort);

private:
    void InitDefaultSettings();
    void AddActiveSetting(RDSSettingID id, const RDSSetting& setting);
    bool GetBoolValue(RDSSettingID settingId) const;
    int GetIntValue(RDSSettingID settingId) const;
    unsigned int GetUnsignedIntValue(RDSSettingID settingId) const;
    QColor GetColorValue(RDSSettingID settingId) const;
    QString GetStringValue(RDSSettingID settingId) const;
    void SetBoolValue(RDSSettingID settingId, const bool value);
    void SetIntValue(RDSSettingID settingId, const int value);
    void SetColorValue(RDSSettingID settingId, const QColor& value);
    void SetStringValue(RDSSettingID settingId, const QString& value);

    RDSSettingsMap              m_activeSettings;               ///< Map containing active settings.
    RDSSettingsMap              m_defaultSettings;              ///< Map containing default settings.
};

#endif // _RDS_SETTINGS_H_
