//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  a class to provide service to initialize dev driver protocols to
///          capture rgp trace from within the profiled application.
//=============================================================================
#ifndef _RGP_CLIENT_IN_PROCESS_MODEL_H_
#define _RGP_CLIENT_IN_PROCESS_MODEL_H_

#include "../listener/listenerCore.h"
#include "../inc/devDriverServer.h"
#include "../inc/devDriverClient.h"
#include "../inc/protocols/driverControlClient.h"

class RGPClientInProcessModel;
typedef struct
{
    RGPClientInProcessModel*    m_pContext;
    DevDriver::DevDriverClient* m_pClient;
} RGPWorkerThreadContext;

class RGPClientInProcessModel
{
public:
    RGPClientInProcessModel();
    ~RGPClientInProcessModel();

    bool Init();
    void Finish();

    bool IsProfileCaptured() const { return m_profileCaptured; }
    const char* GetProfileName() const { return m_profileName.c_str(); }

    bool TriggerCapture(const char* captureFileName);
    void CollectTrace();

    bool ProcessHaltedMessage(DevDriver::ClientId clientId);

private:
    RGPClientInProcessModel(const RGPClientInProcessModel& c);              // disable
    RGPClientInProcessModel& operator=(const RGPClientInProcessModel& c);   // disable

    bool InitDriverProtocols();
    void DeInitDriverProtocols();

    bool CreateWorkerThreadToResumeDriverAndCollectRgpTrace();

    void GenerateProfileName(std::string& profileName);
    void SetProfileCaptured(bool bCaptured) { m_profileCaptured = bCaptured; }

    bool ConnectProtocolClients(
        DevDriver::DevDriverClient*                             pClient,
        DevDriver::ClientId                                     clientId,
        DevDriver::RGPProtocol::RGPClient*&                     pRgpClientOut,
        DevDriver::DriverControlProtocol::DriverControlClient*& pDriverControlClientOut);

    void DisconnectProtocolClients(
        DevDriver::DevDriverClient*                            pClient,
        DevDriver::RGPProtocol::RGPClient*                     pRgpClient,
        DevDriver::DriverControlProtocol::DriverControlClient* pDriverControlClient);

    DevDriver::Result SetGPUClockMode(
        DevDriver::DriverControlProtocol::DriverControlClient* pDriverControlClient,
        DevDriver::DriverControlProtocol::DeviceClockMode      kTraceClockMode);

    bool EnableRgpProfiling(DevDriver::RGPProtocol::RGPClient* pRgpClient);

    bool ResumeDriverAndWaitForDriverInitilization(DevDriver::DriverControlProtocol::DriverControlClient* pDriverControlClient);

    bool CollectRgpTrace(
        DevDriver::RGPProtocol::RGPClient*                     pRgpClient,
        DevDriver::DriverControlProtocol::DriverControlClient* pDriverControlClient);

    DevDriver::ListenerCore                                 m_listenerCore;
    DevDriver::DevDriverClient*                             m_pClient;

    DevDriver::Platform::Thread                             m_thread;
    RGPWorkerThreadContext                                  m_threadContext;

    std::string                                             m_profileName;          ///< The name of the last saved profile

    DevDriver::ClientId                                     m_clientId;             ///< The current client Id
    bool                                                    m_profileCaptured;      ///< Has a profile been captured
    bool                                                    m_finished;             ///< Has Finished() been called. Ensure it's only called once
};

#endif // _RGP_CLIENT_IN_PROCESS_MODEL_H_
