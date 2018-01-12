//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The model used to store all of RDS's configuration data.
//=============================================================================

#ifndef _CONFIGURATION_WINDOW_MODEL_H_
#define _CONFIGURATION_WINDOW_MODEL_H_

#include "../../Common/ModelViewMapper.h"

/// An enumeration used to represent all controls in the RDS configuration window.
enum RdsConfigurationControls
{
    RDS_CONFIGURATION_CONTROLS_LISTEN_PORT,

    RDS_CONFIGURATION_CONTROLS_COUNT
};

/// The model used to store all of RDS's configuration data.
class ConfigurationWindowModel : public ModelViewMapper
{
    Q_OBJECT
public:
    explicit ConfigurationWindowModel();
    virtual ~ConfigurationWindowModel() {}

    void InitializeDefaults();
    void Update(RdsConfigurationControls modelIndex, const QVariant& value);

private:
    QString m_listenPortString;
};

#endif // _CONFIGURATION_WINDOW_MODEL_H_
