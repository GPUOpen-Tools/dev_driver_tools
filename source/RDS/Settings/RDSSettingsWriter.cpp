//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of RDS's XML settings writer.
//=============================================================================

#include "RDSSettingsWriter.h"

//-----------------------------------------------------------------------------
/// Explicit constructor.
/// \param pRdsSettings Output settings class.
//-----------------------------------------------------------------------------
RDSSettingsWriter::RDSSettingsWriter(RDSSettings* pRdsSettings) :
    m_pRdsSettings(pRdsSettings)
{
}

//-----------------------------------------------------------------------------
/// Begin writing RDS XML file and make sure it's valid.
/// \param pDevice The XML file represented by a Qt IO device.
//-----------------------------------------------------------------------------
bool RDSSettingsWriter::Write(QIODevice* pDevice)
{
    m_writer.setDevice(pDevice);
    m_writer.setAutoFormatting(true);

    m_writer.writeStartDocument();
    m_writer.writeStartElement("RDS");

    WriteSettings();

    m_writer.writeEndElement();
    m_writer.writeEndDocument();

    return m_writer.hasError() == false;
}

//-----------------------------------------------------------------------------
/// Detect setting list.
//-----------------------------------------------------------------------------
void RDSSettingsWriter::WriteSettings()
{
    RDSSettingsMap& settings = m_pRdsSettings->Settings();

    for (RDSSettingsMap::iterator i = settings.begin(); i != settings.end(); ++i)
    {
        m_writer.writeStartElement("Setting");
        WriteSetting(i.value());
        m_writer.writeEndElement();
    }
}

//-----------------------------------------------------------------------------
/// Write individual settings.
/// \param setting The RDSSetting structure to write out
//-----------------------------------------------------------------------------
void RDSSettingsWriter::WriteSetting(const RDSSetting& setting)
{
    m_writer.writeTextElement("Name", setting.name);
    m_writer.writeTextElement("Value", setting.value);
}
