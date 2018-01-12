//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A model responsible for enabling calls to a developer driver protocol client.
//=============================================================================

#include "DriverProtocolModel.h"
#include "DeveloperPanelModel.h"
#include "../Util/RDPUtil.h"
#include "../../DevDriverComponents/inc/protocols/driverControlClient.h"
#include "../../DevDriverComponents/inc/protocols/loggingClient.h"
#include "../../DevDriverComponents/inc/protocols/rgpClient.h"
#include "../../DevDriverComponents/inc/protocols/settingsClient.h"
#include "../../DevDriverComponents/inc/protocolClient.h"
#ifdef WIN32
#include "../../DevDriverComponents/inc/protocols/etwClient.h"
#endif

//-----------------------------------------------------------------------------
/// Default constructor for DriverProtocolModel.
/// \param pPanelModel The main Panel model used to maintain a connection to RDS.
/// \param modelCount the number of models required.
//-----------------------------------------------------------------------------
DriverProtocolModel::DriverProtocolModel(DeveloperPanelModel* pPanelModel, uint32_t modelCount)
    : ModelViewMapper(modelCount)
    , m_pPanelModel(pPanelModel)
    , m_clientId(0)
{
}

//-----------------------------------------------------------------------------
/// Default destructor for DriverProtocolModel.
//-----------------------------------------------------------------------------
DriverProtocolModel::~DriverProtocolModel()
{
}

//-----------------------------------------------------------------------------
/// Retrieve a driver protocol client by type.
/// \param protocol The type of protocol client to retrieve.
/// \param ppProtocolClient The driver protocol client
/// \param clientId The ClientId of the Developer Mode process to connect to.
/// \returns True if the client was retrieved successfully.
//-----------------------------------------------------------------------------
bool DriverProtocolModel::GetClientByType(DevDriver::Protocol protocol, DevDriver::IProtocolClient** ppProtocolClient)
{
    if ((m_pPanelModel != nullptr) && (ppProtocolClient != nullptr))
    {
        ChannelContext& channelContext = m_pPanelModel->GetChannelContext();
        if(channelContext.pClient != nullptr)
        {
            DevDriver::BaseProtocolClient* pClient = nullptr;

            switch (protocol)
            {
            case DevDriver::Protocol::DriverControl:
                {
                    DevDriver::DriverControlProtocol::DriverControlClient* pDriverControlClient = channelContext.pClient->AcquireProtocolClient<DevDriver::Protocol::DriverControl>();
                    pClient = reinterpret_cast<DevDriver::BaseProtocolClient*>(pDriverControlClient);
                }
                break;
            case DevDriver::Protocol::Logging:
                {
                    DevDriver::LoggingProtocol::LoggingClient* pLoggingClient = channelContext.pClient->AcquireProtocolClient<DevDriver::Protocol::Logging>();
                    pClient = reinterpret_cast<DevDriver::BaseProtocolClient*>(pLoggingClient);
                }
                break;
            case DevDriver::Protocol::Settings:
                {
                    DevDriver::SettingsProtocol::SettingsClient* pSettingsClient = channelContext.pClient->AcquireProtocolClient<DevDriver::Protocol::Settings>();
                    pClient = reinterpret_cast<DevDriver::BaseProtocolClient*>(pSettingsClient);
                }
                break;
            case DevDriver::Protocol::RGP:
                {
                    DevDriver::RGPProtocol::RGPClient* pRgpClient = channelContext.pClient->AcquireProtocolClient<DevDriver::Protocol::RGP>();
                    pClient = reinterpret_cast<DevDriver::BaseProtocolClient*>(pRgpClient);
                }
                break;
#ifdef WIN32
            case DevDriver::Protocol::ETW:
                {
                    DevDriver::ETWProtocol::ETWClient* pEtwClient = channelContext.pClient->AcquireProtocolClient<DevDriver::Protocol::ETW>();
                    pClient = reinterpret_cast<DevDriver::BaseProtocolClient*>(pEtwClient);
                }
                break;
#endif
            default:
                {
                    RDPUtil::DbgMsg("[RDP] Failed to create DevDriverClient with type %d", protocol);
                }
            }

            if (pClient != nullptr)
            {
                Q_ASSERT(m_clientId != 0);
                bool connected = (pClient->Connect(m_clientId) == DevDriver::Result::Success);

                const QString protocolString = ToolUtil::GetProtocolTypeString(protocol);

                if (connected)
                {
                    *ppProtocolClient = pClient;
                    return true;
                }
                else
                {
                    RDPUtil::DbgMsg("[RDP] Failed to connect %s client using ClientId '%d'", protocolString.toStdString().c_str(), m_clientId);
                }
            }
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
/// Release the given protocol client.
/// \param pProtocolClient The client to be released.
//-----------------------------------------------------------------------------
void DriverProtocolModel::ReleaseClient(DevDriver::IProtocolClient* pProtocolClient)
{
    bool clientNull = pProtocolClient == nullptr;
    if (clientNull)
    {
        RDPUtil::DbgMsg("[RDP] - Failed to release protocol client because it was null.");
    }

    ChannelContext& channelContext = m_pPanelModel->GetChannelContext();
    bool channelClientIsNull = channelContext.pClient == nullptr;
    if (channelClientIsNull)
    {
        RDPUtil::DbgMsg("[RDP] - Failed to release protocol client because DevDriverClient was null.");
    }

    Q_ASSERT((clientNull == false && channelClientIsNull == false));

    channelContext.pClient->ReleaseProtocolClient(pProtocolClient);
}
