//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the Application Settings File Writer.
//=============================================================================

#ifndef _APP_SETTINGS_FILE_WRITER_H_
#define _APP_SETTINGS_FILE_WRITER_H_

#include <QIODevice>
#include <QXmlStreamWriter>

#include "ApplicationSettingsFile.h"

/// The class responsible for reading an application-specific settings file.
class ApplicationSettingsFileWriter
{
public:
    explicit ApplicationSettingsFileWriter(ApplicationSettingsFile* pAppSettingsFile);
    bool Write(QIODevice* pDevice);

private:
    void WriteTargetExecutable();
    void WriteDriverSettings();
    void WriteDriverSetting(const DevDriver::SettingsProtocol::Setting& currentSetting);
    void WriteDriverSettingValue(DevDriver::SettingsProtocol::SettingType type, const QString& tagText, const DevDriver::SettingsProtocol::SettingValue& value);
    void WriteGlobalFlag();

    QXmlStreamWriter m_writer;          ///< Qt's XML stream
    ApplicationSettingsFile*     m_pDriverSettingsFile;    ///< Belongs to the caller, not this class
};

#endif // _APP_SETTINGS_FILE_WRITER_H_
