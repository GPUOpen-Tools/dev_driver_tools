//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Main entry point for headless Radeon Developer Service (i.e. command line mode).
//=============================================================================

#include "../Common/DriverToolsDefinitions.h"
#include "RDSDefinitions.h"
#include "../Common/CommandlineParser.h"
#include "../Common/ddMemAlloc.h"
#include "../Common/SingleInstance.h"
#include "../../DevDriverComponents/listener/listenerCore.h"
#include "../../DevDriverComponents/inc/protocols/loggingClient.h"

using namespace DevDriver;
static ListenerCore* s_pListenerCore = nullptr;
static SingleInstance* s_pSingleInstance = nullptr;

// needs to be volatile as it's value is changed in an event handler. This should
// instruct the compiler not to optimize out the while loop
static volatile bool s_exit = false;

//--------------------------------------------------------------
/// Clean up before terminating. Delete any allocated memory and
/// shutdown the service
//--------------------------------------------------------------
static void Cleanup()
{
    DD_PRINT(LogLevel::Info, "[RDS] Cleanup");
    if (s_pListenerCore != nullptr)
    {
        s_pListenerCore->Destroy();
        delete s_pListenerCore;
        s_pListenerCore = nullptr;
    }

    if (s_pSingleInstance != nullptr)
    {
        delete s_pSingleInstance;
        s_pSingleInstance = nullptr;
    }
}

#ifdef WIN32
//--------------------------------------------------------------
/// Registered as a callback when the user closes the server console window
/// \param dwCtrlType CTRL type
/// \return Always returns true
//--------------------------------------------------------------
BOOL ConsoleCloseHandler(DWORD dwCtrlType)
{
    DD_UNUSED(dwCtrlType);
    s_exit = true;

    // return true to indicate that we handled the event
    return TRUE;
}

#else
// Linux signal handling

#include "../Common/Linux/SignalHandler.h"

//--------------------------------------------------------------
/// Handler that gets called when the SIGTERM signal is caught
//--------------------------------------------------------------

void SigHandler()
{
    s_exit = true;
}

#endif // WIN32

//-----------------------------------------------------------------------------
/// Main entry point.
/// \param argc The number of arguments.
/// \param argv An array containing arguments.
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    CommandlineParser commandLine(argc, argv);
    commandLine.SetHelpOption("--help", gs_RDS_CLI_HELP_OPTION_DESCRIPTION);
    Int16CommandlineParameter portParameter("--port", gs_RDS_CLI_PORT_OPTION_DESCRIPTION, false, gs_DEFAULT_CONNECTION_PORT);
    commandLine.AddParameter(&portParameter);
    CommandlineParameter uwpParameter("--enableUWP", gs_RDS_CLI_UWPENABLE_OPTION_DESCRIPTION, false, true);
#ifdef WIN32
    commandLine.AddParameter(&uwpParameter);
#endif

    bool parseSucceeded = commandLine.Parse();
    int retVal = 0;
    if (!parseSucceeded)
    {
        DD_PRINT(LogLevel::Error, "[RDS] Error parsing commandline arguments.");
        DD_PRINT(LogLevel::Error, commandLine.ErrorString().c_str());
        retVal = -1;
    }

    if (!parseSucceeded || commandLine.IsHelpRequested())
    {
        printf("Usage: %s %s", gs_RDS_EXECUTABLE_FILENAME, gs_RDS_CLI_USAGE_DESCRIPTION);
        printf("%s", commandLine.HelpString().c_str());
        return retVal;
    }

    s_pSingleInstance = new SingleInstance(gs_RDS_APPLICATION_GUID);
    if (s_pSingleInstance == nullptr || s_pSingleInstance->IsProgramAlreadyRunning())
    {
        DD_PRINT(LogLevel::Error, "[RDS] Error - An instance of RDS is already running.");
        return -1;
    }

#ifdef WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleCloseHandler, TRUE);
#else
    // install signal handlers on Linux
    SignalHandler signalHandler;
    signalHandler.AddHandler((__sighandler_t)SigHandler, SIGTERM);
    signalHandler.AddHandler((__sighandler_t)SigHandler, SIGINT);
#endif // WIN32

    ListenerBindAddress address = {};
    Platform::Strncpy(address.hostAddress, gs_DEFAULT_HOST_ADDRESS, sizeof(char) * kMaxStringLength);
    address.port = portParameter.ValueAsInt();
    s_pListenerCore = new ListenerCore();

    ListenerCreateInfo createInfo = {};
    const std::string& listenerNameString = gs_PRODUCT_NAME_STRING;
    Platform::Strncpy(createInfo.description, listenerNameString.c_str(), sizeof(char) * kMaxStringLength);
    createInfo.pAddressesToBind = &address;
    createInfo.numAddresses = 1;

    createInfo.flags.enableServer = true;
    createInfo.flags.enableUWP = uwpParameter.IsParameterPresent();

    DevDriver::AllocCb GenericAllocCb =
    {
        nullptr,
        &ddMemAlloc::GenericAlloc,
        &ddMemAlloc::GenericFree
    };

    createInfo.allocCb = GenericAllocCb;
    createInfo.serverCreateInfo.enabledProtocols.logging = true;
    createInfo.serverCreateInfo.enabledProtocols.etw = true;

    Result initResult = s_pListenerCore->Initialize(createInfo);

    if (initResult == Result::Success)
    {
        std::string initCompleteMessage = "[RDS] Initialized successfully.Now listening for RDP connection.";
#ifdef WIN32
        if (uwpParameter.IsParameterPresent())
        {
            initCompleteMessage += " (UWP Enabled)";
        }
        else
        {
            initCompleteMessage += " (UWP Disabled)";
        }
#endif // WIN32
        DD_PRINT(LogLevel::Info, initCompleteMessage.c_str());
    }
    else
    {
        DD_PRINT(LogLevel::Error, "[RDS] Failed to initialize listener.");
        return -1;
    }

    // Process loop. Check messages on windows, wait for Ctrl-C on linux
    while (s_exit == false)
    {
#ifdef WIN32
        MSG msg = {};

        // Process any messages in the queue.
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                s_exit = true;
            }
        }
#endif // WIN32
    }

    Cleanup();

#ifndef WIN32
    // remove signal handlers on Linux
    signalHandler.RemoveHandlers();
#endif // WIN32

}
