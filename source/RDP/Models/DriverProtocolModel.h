//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A model responsible for enabling calls to a developer driver protocol client.
//=============================================================================

#ifndef _DRIVER_PROTOCOL_MODEL_H_
#define _DRIVER_PROTOCOL_MODEL_H_

#include "../../Common/ModelViewMapper.h"
#include "../RDPDefinitions.h"

namespace DevDriver
{
    class IProtocolClient;
}

class DeveloperPanelModel;

/// A model responsible for enabling calls to a developer driver protocol client.
class DriverProtocolModel : public ModelViewMapper
{
    Q_OBJECT
public:
    explicit DriverProtocolModel(DeveloperPanelModel* pPanelModel, uint32_t modelCount);
    virtual ~DriverProtocolModel();

    bool GetClientByType(DevDriver::Protocol protocol, DevDriver::IProtocolClient** ppProtocolClient);
    void ReleaseClient(DevDriver::IProtocolClient* pProtocolClient);
    DeveloperPanelModel* GetPanelModel() const { return m_pPanelModel; }
    void SetConnectedClientId(DevDriver::ClientId clientId) { m_clientId = clientId; }
    DevDriver::ClientId GetConnectedClientId() const { return m_clientId; }

private:
    DeveloperPanelModel* m_pPanelModel;     ///< The main model for the panel, which maintains a connection to RDS.
    DevDriver::ClientId m_clientId;
};

#endif // _DRIVER_PROTOCOL_MODEL_H_
