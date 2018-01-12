//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The Main Window interface for the Radeon Developer Service.
//=============================================================================

#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "../../DevDriverComponents/listener/listenerCore.h"
#include "../../DevDriverComponents/inc/ddTransferManager.h"

class QAction;
class QMenu;
class QThread;

class ConfigurationWindow;
class DebugWindow;

namespace Ui
{
class MainWindow;
}

/// A URI service used to command RDS remotely.
class CommandService : public DevDriver::URIProtocol::URIService
{
public:
    CommandService() : URIService("command") {}
    virtual ~CommandService() {}

    virtual DevDriver::Result HandleRequest(DevDriver::URIProtocol::URIRequestContext* pContext) override;
};

/// The main window implementation for RDS.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* pParent = nullptr);
    virtual ~MainWindow();

    void CreateActions();
    void CreateTrayIcon();
    void DestroyTrayIcon();
    void InitializeDebugWindow();
    void InitializeService();
    void ShutdownService();
    void EmitTerminateProcess();
    void ToggleConfigWindowVisibility();

signals:
    void TerminateProcess();

private slots:
    void OnTerminateProcessEmitted();

private:
    Ui::MainWindow* ui;

    DebugWindow* m_pDebugWindow;                    ///< The window used to display the Panel's debug output.
    QAction* m_pEnableUWPAction;                    ///< Action to enable UWP support in the listener.
    QAction* m_pQuitAction;                         ///< Action to terminate the listener application.
    QAction* m_pConfigureAction;                    ///< Action to open the configuration window.
    QMenu* m_pTrayIconMenu;                         ///< Context Menu opened on right-clicking tray icon.
    QSystemTrayIcon* m_pTrayIcon;                   ///< The listener's system tray icon.
    ConfigurationWindow* m_pConfigurationWindow;    ///< The RDS settings configuration window.

    DevDriver::ListenerCore m_listenerCore;         ///< The listener's communication object.
    DevDriver::uint32 m_listenPort;                 ///< The port that the listener has opened for incoming connections.
    bool m_listening;                               ///< A flag to indicate if the listener is listening for connections.
    bool m_enableUWP;                               ///< A flag to indicate if UWP support should be enabled.

signals:
    void ListenPortUpdated(unsigned int port);

private slots:
    void OnListenEndpointUpdated();
    void OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void OnUWPChanged(bool enableUWP);
    void OnConfigureTriggered();

private:
    CommandService m_commandService;
};

#endif // _MAIN_WINDOW_H_
