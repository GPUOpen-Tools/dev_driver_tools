//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RDP's Application Settings file reader.
//=============================================================================

#ifndef _APP_SETTINGS_FILE_READER_H_
#define _APP_SETTINGS_FILE_READER_H_

#include <QIODevice>
#include <QXmlStreamReader>
#include "ApplicationSettingsFile.h"

/// Support for RDP's XML settings reader.
class ApplicationSettingsFileReader
{
public:
    explicit ApplicationSettingsFileReader(ApplicationSettingsFile* pDriverSettingsFile);
    bool Read(QIODevice* pDevice);

private:
    void ReadSettingsFile();
    void ReadDriverSettings();
    void ReadSettingsCategory();
    void ReadDriverSetting(DevDriver::SettingsProtocol::Setting& setting);
    bool ReadDriverSettingValue(const QString& settingValueString,
        DevDriver::SettingsProtocol::SettingType settingType,
        DevDriver::SettingsProtocol::SettingValue& value);

    QXmlStreamReader m_reader;      ///< Qt's XML stream
    ApplicationSettingsFile* m_pApplicationSettingsFile;     ///< Belongs to the caller, not this class
};

#endif // _APP_SETTINGS_FILE_READER_H_
