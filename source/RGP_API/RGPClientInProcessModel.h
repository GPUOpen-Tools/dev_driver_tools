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

    bool IsProfileCaptured() const { return m_bProfileCaptured; }
    void SetProfileCaptured(bool bCaptured) { m_bProfileCaptured = bCaptured; }

    void GetProfileName(std::string& profileName);

private:
    RGPClientInProcessModel(const RGPClientInProcessModel& c);              // disable
    RGPClientInProcessModel& operator=(const RGPClientInProcessModel& c);   // disable

    bool InitDriverProtocols();
    void DeInitDriverProtocols();

    bool CreateWorkerThreadToResumeDriverAndCollectRgpTrace();

    DevDriver::Result JoinThread() { return m_thread.Join(); }

    DevDriver::ListenerCore     m_listenerCore;
    DevDriver::DevDriverClient* m_pClient;

    DevDriver::Platform::Thread m_thread;
    RGPWorkerThreadContext      m_threadContext;
    std::string                 m_profileName;

    bool                        m_bProfileCaptured;
};

#endif // _RGP_CLIENT_IN_PROCESS_MODEL_H_
