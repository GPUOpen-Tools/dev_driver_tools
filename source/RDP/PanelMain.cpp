//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Main entry point.
//=============================================================================

#include "Views/MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <QLayout>

#include <ScalingManager.h>
#include "../Common/Util/SingleApplicationInstance.h"
#include "RDPDefinitions.h"

// The application instance (derived from QApplication)
SingleApplicationInstance* s_pAppInstance = nullptr;

#ifndef Q_OS_WIN

// Linux signal handling

#include "../Common/Linux/SignalHandler.h"

//--------------------------------------------------------------
/// Handler that gets called when the SIGINT signal is caught
//--------------------------------------------------------------
void SigHandler()
{
    if (s_pAppInstance != nullptr)
    {
        s_pAppInstance->exit();
    }
}

#endif

//-----------------------------------------------------------------------------
/// Clean up any resources before exiting the program
//-----------------------------------------------------------------------------
static void Cleanup()
{
    SAFE_DELETE(s_pAppInstance);
}

//-----------------------------------------------------------------------------
/// Main entry point.
/// \param argc The number of arguments.
/// \param argv An array containing arguments.
//-----------------------------------------------------------------------------
int main(int argc, char* argv[]) try
{
    s_pAppInstance = new (std::nothrow)SingleApplicationInstance(argc, argv, gs_RDP_APPLICATION_GUID);
    if (s_pAppInstance == nullptr || s_pAppInstance->IsAnotherInstanceRunning())
    {
        // RDP is already running, so don't allow this new instance to proceed any further.
        Cleanup();
        return -1;
    }

    MainWindow* pMainWindow = new (std::nothrow)MainWindow();
    if (pMainWindow == nullptr)
    {
        Cleanup();
        return -1;
    }

#ifndef Q_OS_WIN
    // install signal handlers on Linux. For now, just Ctrl-C (SIGINT)
    SignalHandler signalHandler;
    signalHandler.AddHandler((__sighandler_t)SigHandler, SIGINT);
#endif

    pMainWindow->show();

    // Connect the slot that brings the application window to the foreground if another instance is started.
    s_pAppInstance->connect(s_pAppInstance, &SingleApplicationInstance::AppInstanceStarted, pMainWindow, &MainWindow::OnAppInstanceStarted);

    // Scaling manager object registration
    ScalingManager::Get().Initialize(pMainWindow);
    ScalingManager::Get().RegisterAll();

    int result = s_pAppInstance->exec();

#ifndef Q_OS_WIN
    // remove signal handlers on Linux
    signalHandler.RemoveHandlers();
#endif

    SAFE_DELETE(pMainWindow);
    Cleanup();

    return result;
}

//-----------------------------------------------------------------------------
/// Catch any exceptions of type std::exception.
/// \param e The exception caught.
//-----------------------------------------------------------------------------
catch (std::exception& e)
{
    qCritical() << "Exception thrown: " << e.what();
}

//-----------------------------------------------------------------------------
/// Catch any unknown exceptions.
//-----------------------------------------------------------------------------
catch (...)
{
    qCritical() << "Unknown exception thrown: ";
}
