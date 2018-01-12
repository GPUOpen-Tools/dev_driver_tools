//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RDP's XML settings writer.
//=============================================================================

#ifndef _RDP_SETTINGS_WRITER_H_
#define _RDP_SETTINGS_WRITER_H_

#include <QIODevice>
#include <QXmlStreamWriter>

#include "RDPSettings.h"

/// Support for RDP's XML settings writer.
class RDPSettingsWriter
{
public:
    explicit RDPSettingsWriter(RDPSettings* pRdpSettings);
    bool Write(QIODevice* pDevice);

private:
    void WriteSettingsAndRecents();

    // Write global settings.
    void WriteSettings();
    void WriteSetting(const RDPSetting& setting);

    // Write recently used files.
    void WriteRecentSettingsFiles();
    void WriteRecentAppSettingsFile(const RDPApplicationSettingsFile& recentFile);

    // Write recent connections.
    void WriteRecentConnections();
    void WriteRecentConnection(const RDSConnectionInfo& connectionInfo);

    // Write Target Applications list.
    void WriteTargetApplications();
    void WriteTargetApplication(const RDSTargetApplicationInfo& applicationInfo);

    void WriteUserClockMode();

    QXmlStreamWriter m_writer;          ///< Qt's XML stream
    RDPSettings*     m_pRdpSettings;    ///< Belongs to the caller, not this class
};

#endif // _RDP_SETTINGS_WRITER_H_
