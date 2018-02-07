//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RDS's XML settings writer.
//=============================================================================

#ifndef _RDS_SETTINGS_WRITER_H_
#define _RDS_SETTINGS_WRITER_H_

#include <QIODevice>
#include <QXmlStreamWriter>

#include "RDSSettings.h"

/// Support for RDS's XML settings writer.
class RDSSettingsWriter
{
public:
    explicit RDSSettingsWriter(RDSSettings* pRdsSettings);
    bool Write(QIODevice* pDevice);

private:
    void WriteSettings();
    void WriteSetting(const RDSSetting& setting);

    QXmlStreamWriter m_writer;          ///< Qt's XML stream
    RDSSettings*     m_pRdsSettings;    ///< Belongs to the caller, not this class
};

#endif // _RDS_SETTINGS_WRITER_H_
