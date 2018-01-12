//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A background worker responsible for reading driver log messages.
//=============================================================================

#include "DriverMessageProcessorThread.h"
#include "DeveloperPanelModel.h"
#include "../Util/RDPUtil.h"
#include "gpuopen.h"
#include "msgChannel.h"

//-----------------------------------------------------------------------------
/// The static instance of the connection settings model. Used to handle the message callback.
//-----------------------------------------------------------------------------
static DeveloperPanelModel* sDeveloperPanelModel = nullptr;

//-----------------------------------------------------------------------------
/// Constructor for DriverMessageProcessorThread.
//-----------------------------------------------------------------------------
DriverMessageProcessorThread::DriverMessageProcessorThread(ChannelContext* pChannelContext, DeveloperPanelModel* pPanelModel)
    : m_pContext(pChannelContext)
    , m_pDeveloperPanelModel(pPanelModel)
{
    sDeveloperPanelModel = m_pDeveloperPanelModel;
}

//-----------------------------------------------------------------------------
/// Destructor for DriverMessageProcessorThread.
//-----------------------------------------------------------------------------
DriverMessageProcessorThread::~DriverMessageProcessorThread()
{
}

//-----------------------------------------------------------------------------
/// Process new incoming client messages.
/// \param pContext The ChannelContext that holds the connection information.
/// \param pMessage The message buffer.
//-----------------------------------------------------------------------------
static void ProcessClientMessage(const DevDriver::MessageBuffer* pMessage)
{
    using namespace DevDriver;
    if (pMessage != nullptr && sDeveloperPanelModel != nullptr)
    {
        if (pMessage->header.protocolId == Protocol::System)
        {
            using namespace DevDriver::SystemProtocol;

            SystemMessage msg = static_cast<SystemMessage>(pMessage->header.messageId);
            switch (msg)
            {
            case SystemMessage::ClientConnected:
                {
                    RDPUtil::DbgMsg("[RDP] Received client connected from unknown client with id %u.", pMessage->header.srcClientId);
                    sDeveloperPanelModel->AddClientId(pMessage->header.srcClientId);
                }
                break;
            case SystemMessage::ClientDisconnected:
                {
                    RDPUtil::DbgMsg("[RDP] Client with Id %u has disconnected.", pMessage->header.srcClientId);
                    sDeveloperPanelModel->ClientDisconnected(pMessage->header.srcClientId);
                }
                break;
            case SystemMessage::Halted:
                {
                    RDPUtil::DbgMsg("[RDP] Received client halted from unknown client with id %u.", pMessage->header.srcClientId);
                    const ClientInfoStruct *pClientInfo = reinterpret_cast<const ClientInfoStruct *>(&(pMessage->payload[0]));

                    const QString clientNameString = QString::fromStdString(pClientInfo->clientName);
                    const QString clientDescriptionString = QString::fromStdString(pClientInfo->clientDescription);

                    sDeveloperPanelModel->AddClientInfo(pMessage->header.srcClientId,
                        clientNameString,
                        pClientInfo->processId,
                        clientDescriptionString);
                }
                break;
            case SystemMessage::Pong:
                {
                    RDPUtil::DbgMsg("[RDP] Received pong from unknown client with id %u.", pMessage->header.srcClientId);
                }
                break;
            default:
                Q_ASSERT(false);
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// Start the driver message processing thread loop.
//-----------------------------------------------------------------------------
void DriverMessageProcessorThread::StartMessageProcessingLoop()
{
    using namespace DevDriver;
    const uint32 kLogDelayInMs = 100;

    MessageBuffer message = {};
    IMsgChannel* pMsgChannel = m_pContext->pClient->GetMessageChannel();

    while (m_pContext->exitRequested == false)
    {
        if (pMsgChannel->Receive(message, kLogDelayInMs) == Result::Success)
        {
            ProcessClientMessage(&message);
        }
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when the thread is finished processing messages.
//-----------------------------------------------------------------------------
void DriverMessageProcessorThread::ThreadFinished()
{
    RDPUtil::DbgMsg("[RDP] Message Processor is finished!");
}
