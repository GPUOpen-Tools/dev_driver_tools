/*
 *******************************************************************************
 *
 * Copyright (c) 2016-2018 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include "ddListenerURIService.h"
#include "msgChannel.h"
#include "ddTransferManager.h"
#include "listenerCore.h"
#include "ddURIRequestContext.h"

namespace DevDriver
{
    // =====================================================================================================================
    ListenerURIService::ListenerURIService()
        : m_pListenerCore(nullptr)
    {
    }

    // =====================================================================================================================
    ListenerURIService::~ListenerURIService()
    {
    }

    // =====================================================================================================================
#if DD_VERSION_SUPPORTS(GPUOPEN_URIINTERFACE_CLEANUP_VERSION)
    Result ListenerURIService::HandleRequest(IURIRequestContext* pContext)
    {
        DD_ASSERT(pContext != nullptr);

        Result result = Result::Error;

        // We can only handle requests if a valid listener core has been bound.
        if (m_pListenerCore != nullptr)
        {
            // We currently only handle the "info" command.
            // All other commands will result in an error.
            if (strcmp(pContext->GetRequestArguments(), "clients") == 0)
            {
                // Get a list of all currently connected clients.
                const std::vector<DevDriver::ClientInfo> connectedClients = m_pListenerCore->GetConnectedClientList();

                ITextWriter* pWriter = nullptr;
                result = pContext->BeginTextResponse(&pWriter);

                if (result == Result::Success)
                {
                    pWriter->Write("--- %zu Connected Clients ---", connectedClients.size());

                    for (uint32 clientIndex = 0; clientIndex < static_cast<uint32>(connectedClients.size()); ++clientIndex)
                    {
                        const ClientInfo& clientInfo = connectedClients[clientIndex];

                        pWriter->Write("\n\n--- Client %zu ---", clientIndex);
                        pWriter->Write("\nName: %s", clientInfo.clientName);
                        pWriter->Write("\nDescription: %s", clientInfo.clientDescription);
                        pWriter->Write("\nProcess Id: %zu", clientInfo.clientPid);
                        pWriter->Write("\nClient Id: %zu", clientInfo.clientId);
                        pWriter->Write("\nHas Been Identified: %zu", clientInfo.hasBeenIdentified);
                    }

                    result = pWriter->End();
                }
            }
            else if (strcmp(pContext->GetRequestArguments(), "transports") == 0)
            {
                // Get a list of all currently managed transports.
                const std::vector<std::shared_ptr<IListenerTransport>>& managedTransports = m_pListenerCore->GetManagedTransports();

                ITextWriter* pWriter = nullptr;
                result = pContext->BeginTextResponse(&pWriter);

                if (result == Result::Success)
                {
                    pWriter->Write("--- %zu Transports ---", managedTransports.size());

                    for (uint32 transportIndex = 0; transportIndex < static_cast<uint32>(managedTransports.size()); ++transportIndex)
                    {
                        const std::shared_ptr<IListenerTransport>& transport = managedTransports[transportIndex];

                        pWriter->Write("\n\n--- Transport %u ---", transportIndex);
                        pWriter->Write("\nName: %s", transport->GetTransportName());
                        pWriter->Write("\nHandle: %u", transport->GetHandle());
                        pWriter->Write("\nIs Forwarding Connection: %zu", transport->ForwardingConnection());
                    }

                    result = pWriter->End();
                }
            }
            else if (strcmp(pContext->GetRequestArguments(), "info") == 0)
            {
                const IClientManager* pClientManager = m_pListenerCore->GetClientManager();
                const ListenerCreateInfo& createInfo = m_pListenerCore->GetCreateInfo();

                ITextWriter* pWriter = nullptr;
                result = pContext->BeginTextResponse(&pWriter);

                if (result == Result::Success)
                {
                    pWriter->Write("Listener Description: %s", createInfo.description);
                    pWriter->Write("\nListener UWP Support: %u", static_cast<uint32>(createInfo.flags.enableUWP));
                    pWriter->Write("\nListener Server Support: %u", static_cast<uint32>(createInfo.flags.enableServer));
                    pWriter->Write("\nClient Manager Name: %s", pClientManager->GetClientManagerName());
                    pWriter->Write("\nClient Manager Host Client Id: %u", static_cast<uint32>(pClientManager->GetHostClientId()));

                    result = pWriter->End();
                }
            }
        }

        return result;
    }
#endif
} // DevDriver
