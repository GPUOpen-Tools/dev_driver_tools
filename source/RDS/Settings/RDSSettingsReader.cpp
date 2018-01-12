//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of RDS's XML settings reader.
//=============================================================================

#include "RDSSettingsReader.h"
#include "RDSSettings.h"

//-----------------------------------------------------------------------------
/// Constructor.
/// \param pRdsSettings Output settings class.
//-----------------------------------------------------------------------------
RDSSettingsReader::RDSSettingsReader(RDSSettings* pRdsSettings) :
    m_pRdsSettings(pRdsSettings)
{
}

//-----------------------------------------------------------------------------
/// Begin reading RDS XML file and make sure it's valid.
/// \param pDevice The XML file represented by a Qt IO device.
//-----------------------------------------------------------------------------
bool RDSSettingsReader::Read(QIODevice* pDevice)
{
    m_reader.setDevice(pDevice);

    if (m_reader.readNextStartElement())
    {
        if (m_reader.name() == "RDS")
        {
            ReadSettings();
        }
    }

    return m_reader.error() == false;
}

//-----------------------------------------------------------------------------
/// Detect setting list.
//-----------------------------------------------------------------------------
void RDSSettingsReader::ReadSettings()
{
    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == "Setting")
        {
            ReadSetting();
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }
}

//-----------------------------------------------------------------------------
/// Read individual settings.
//-----------------------------------------------------------------------------
void RDSSettingsReader::ReadSetting()
{
    RDSSetting setting;

    while (m_reader.readNextStartElement())
    {
        if (m_reader.name() == "Name")
        {
            setting.name = m_reader.readElementText();
        }
        else if (m_reader.name() == "Value")
        {
            setting.value = m_reader.readElementText();
        }
        else
        {
            m_reader.skipCurrentElement();
        }
    }

    m_pRdsSettings->AddPotentialSetting(setting.name, setting.value);
}
