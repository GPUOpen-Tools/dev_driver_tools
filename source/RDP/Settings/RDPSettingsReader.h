//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RDP's XML settings reader.
//=============================================================================

#ifndef _RDP_SETTINGS_READER_H_
#define _RDP_SETTINGS_READER_H_

#include <QIODevice>
#include <QXmlStreamReader>

class RDPSettings;

/// Support for RDP's XML settings reader.
class RDPSettingsReader
{
public:
    explicit RDPSettingsReader(RDPSettings* pRdpSettings);
    bool Read(QIODevice* pDevice);

private:
    void ReadSettingsAndRecents();

    // Read global settings.
    void ReadSettings();
    void ReadSetting();

    // Read recently used files.
    void ReadRecentSettingsFiles();
    void ReadRecentSettingsFile();

    // Read recently used connections to RDS.
    void ReadRecentConnections();
    void ReadRecentConnection();

    // Read target applications list.
    void ReadTargetApplications();
    void ReadTargetApplication();

    void ReadUserClockMode();

    QXmlStreamReader m_reader;          ///< Qt's XML stream
    RDPSettings*     m_pRdpSettings;    ///< Belongs to the caller, not this class
};

#endif // _RDP_SETTINGS_READER_H_
