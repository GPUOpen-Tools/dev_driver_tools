//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RDS's XML settings reader.
//=============================================================================

#ifndef _RDS_SETTINGS_READER_H_
#define _RDS_SETTINGS_READER_H_

#include <QIODevice>
#include <QXmlStreamReader>

class RDSSettings;

/// Support for RDS's XML settings reader.
class RDSSettingsReader
{
public:
    explicit RDSSettingsReader(RDSSettings* pRdsSettings);
    bool Read(QIODevice* pDevice);

private:
    void ReadSettings();
    void ReadSetting();

    QXmlStreamReader m_reader;          ///< Qt's XML stream
    RDSSettings*     m_pRdsSettings;    ///< Belongs to the caller, not this class
};

#endif // _RDS_SETTINGS_READER_H_
