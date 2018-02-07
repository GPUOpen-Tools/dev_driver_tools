//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Main entry point.
//=============================================================================

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include "Views/MainWindow.h"
#include "RDSDefinitions.h"
#include "../Common/Util/SingleApplicationInstance.h"
#include "../Common/ToolUtil.h"
#include <ScalingManager.h>

// The application instance (derived from QApplication)
SingleApplicationInstance* s_pAppInstance = nullptr;

#ifndef Q_OS_WIN

// Linux signal handling

#include "../Common/Linux/SignalHandler.h"

//--------------------------------------------------------------
/// Handler that gets called when the SIGTERM signal is caught
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
int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(Service);

    // We only want a single instance of RDS to run on a machine.
    s_pAppInstance = new (std::nothrow)SingleApplicationInstance(argc, argv, gs_RDS_APPLICATION_GUID, true);
    if (s_pAppInstance == nullptr || s_pAppInstance->IsAnotherInstanceRunning())
    {
        // RDS is already running, so don't allow this new instance to proceed any further.
        Cleanup();
        return -1;
    }

    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        // The system tray icon wasn't available to the user, but there's no good reason to stop RDS entirely.
        // Let the user know that the tray icon isn't available, but proceed anyways.
        QMessageBox::critical(0, QObject::tr("Systray"), QObject::tr("Operating in Headless Mode."));
    }

    QApplication::setQuitOnLastWindowClosed(false);

    MainWindow* pWindow = new (std::nothrow)MainWindow();

#ifndef Q_OS_WIN
    // install signal handlers on Linux
    SignalHandler signalHandler;
    signalHandler.AddHandler((__sighandler_t)SigHandler, SIGTERM);
    signalHandler.AddHandler((__sighandler_t)SigHandler, SIGINT);
#endif

    // Scaling manager object registration
    ScalingManager::Get().Initialize(pWindow);
    ScalingManager::Get().RegisterAll();

    int result = s_pAppInstance->exec();

#ifndef Q_OS_WIN
    // remove signal handlers on Linux
    signalHandler.RemoveHandlers();
#endif

    // Destroy the main window and clean up the service.
    SAFE_DELETE(pWindow);
    Cleanup();

    return result;
}
