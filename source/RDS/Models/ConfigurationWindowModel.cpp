//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The model used to store all of RDS's configuration data.
//=============================================================================

#include <QVariant>
#include "ConfigurationWindowModel.h"
#include "../Settings/RDSSettings.h"

//-----------------------------------------------------------------------------
/// Constructor for ConfigurationWindowModel.
//-----------------------------------------------------------------------------
ConfigurationWindowModel::ConfigurationWindowModel()
    : ModelViewMapper(RDS_CONFIGURATION_CONTROLS_COUNT)
{
}

//-----------------------------------------------------------------------------
/// Initialize default values for the configuration window model.
//-----------------------------------------------------------------------------
void ConfigurationWindowModel::InitializeDefaults()
{
    RDSSettings& rdsSettings = RDSSettings::Get();

    // Load the default listen port from the user's settings file.
    unsigned int settingsListenPort = rdsSettings.GetListenPort();
    Update(RDS_CONFIGURATION_CONTROLS_LISTEN_PORT, settingsListenPort);
}

//-----------------------------------------------------------------------------
/// Update the model for the incoming model index.
/// \param modelIndex The index of the model whose value is being updated.
/// \param value The new value for the model being updated.
//-----------------------------------------------------------------------------
void ConfigurationWindowModel::Update(RdsConfigurationControls modelIndex, const QVariant& value)
{
    SetModelData(modelIndex, value);

    switch (modelIndex)
    {
    case RDS_CONFIGURATION_CONTROLS_LISTEN_PORT:
        {
            m_listenPortString = value.toString();
            unsigned int listenPort = value.toUInt();

            RDSSettings& rdsSettings = RDSSettings::Get();
            rdsSettings.SetListenPort(listenPort);
        }
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}
