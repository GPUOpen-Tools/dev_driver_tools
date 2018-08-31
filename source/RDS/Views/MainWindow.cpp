//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The Main Window interface for the Radeon Developer Service.
//=============================================================================

#include <QApplication>
#include <QDesktopWidget>
#include <QIcon>
#include <QMessageBox>
#include <QProcess>
#include <QThread>
#include <QCommandLineParser>
#include <QWindow>
#include <QScreen>

#include "ConfigurationWindow.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "../RDSDefinitions.h"
#include "../Settings/RDSSettings.h"
#include "../../Common/ToolUtil.h"
#include "../../Common/ddMemAlloc.h"
#include "../../Common/DriverToolsDefinitions.h"
#include "../../Common/Views/DebugWindow.h"
#include "../../DevDriverComponents/listener/listenerCore.h"

using namespace DevDriver;

MainWindow* g_MainWindow = nullptr;

//-----------------------------------------------------------------------------
/// Handle incoming requsts for the RDSCommand service.
/// \param pContext The incoming command request context.
/// \returns The result of the handled request.
//-----------------------------------------------------------------------------
DevDriver::Result CommandService::HandleRequest(DevDriver::IURIRequestContext* pContext)
{
    DevDriver::Result result = DevDriver::Result::Error;

    if (strcmp(pContext->GetRequestArguments(), "terminate") == 0)
    {
        result = DevDriver::Result::Success;

        g_MainWindow->EmitTerminateProcess();
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Constructor for the Service's Main Window.
/// \param pParent The parent widget for this window.
//-----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* pParent) :
    QMainWindow(pParent),
    ui(new Ui::MainWindow),
    m_pEnableUWPAction(nullptr),
    m_pQuitAction(nullptr),
    m_pTrayIconMenu(nullptr),
    m_pTrayIcon(nullptr),
    m_listenPort(gs_DEFAULT_CONNECTION_PORT),
    m_listening(false),
    m_enableUWP(false)
{
    ui->setupUi(this);

    g_MainWindow = this;

#if SHOW_DEBUG_WINDOW
    InitializeDebugWindow();
#endif

    // Load the application's default settings and any user modifications to them.
    static RDSSettings& rdsSettings = RDSSettings::Get();
    if (rdsSettings.LoadSettings())
    {
        ToolUtil::DbgMsg("[RDS] Loaded RDS settings file.");
    }
    else
    {
        ToolUtil::DbgMsg("[RDS] Failed to load RDS settings file. Will use default settings.");
    }

    // Create the configuration window, but don't show it until the user wants to see it.
    m_pConfigurationWindow = new ConfigurationWindow(nullptr);
    m_pConfigurationWindow->hide();

    connect(m_pConfigurationWindow, &ConfigurationWindow::ListenEndpointUpdated, this, &MainWindow::OnListenEndpointUpdated);
    connect(this, &MainWindow::ListenPortUpdated, m_pConfigurationWindow, &ConfigurationWindow::OnListenPortUpdated);
    connect(this, &MainWindow::TerminateProcess, this, &MainWindow::OnTerminateProcessEmitted);

    CreateActions();
    CreateTrayIcon();

    InitializeService();
}

//-----------------------------------------------------------------------------
/// Initialize the debug window.
//-----------------------------------------------------------------------------
void MainWindow::InitializeDebugWindow()
{
    const QRect desktopRes = QApplication::desktop()->screenGeometry();

    const int desktopWidth = (gs_DESKTOP_AVAIL_WIDTH_PCT / 100.0f) * desktopRes.width();
    const int desktopHeight = (gs_DESKTOP_AVAIL_HEIGHT_PCT / 100.0f) * desktopRes.height();

    int mainHeight = (gs_MAIN_WINDOW_DESKTOP_HEIGHT_PCT / 100.0f) * desktopHeight - gs_DESKTOP_MARGIN - gs_OS_TITLE_BAR_HEIGHT - gs_DESKTOP_MARGIN;

    const int dbgWidth = desktopWidth * (gs_DBG_WINDOW_DESKTOP_WIDTH_PCT / 100.0f);
    const int dbgHeight = desktopHeight - gs_DESKTOP_MARGIN - gs_OS_TITLE_BAR_HEIGHT - mainHeight - gs_OS_TITLE_BAR_HEIGHT;

    m_pDebugWindow = new DebugWindow(this);
    m_pDebugWindow->resize(dbgWidth, dbgHeight);
    m_pDebugWindow->show();

    ToolUtil::RegisterDbgWindow(m_pDebugWindow);
}

//-----------------------------------------------------------------------------
/// Destructor for the Service's Main Window.
//-----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    DestroyTrayIcon();
    ShutdownService();

    delete ui;
}

//-----------------------------------------------------------------------------
/// Initialize the Service's system tray icon.
//-----------------------------------------------------------------------------
void MainWindow::CreateTrayIcon()
{
    m_pTrayIconMenu = new QMenu(this);

    m_pTrayIconMenu->addAction(m_pConfigureAction);
    m_pTrayIconMenu->addAction(m_pQuitAction);

    m_pTrayIcon = new QSystemTrayIcon(this);
    m_pTrayIcon->setContextMenu(m_pTrayIconMenu);

    connect(m_pTrayIcon, &QSystemTrayIcon::activated, this, &MainWindow::OnTrayIconActivated);

    // The tray icon tooltip is used to display the product name and version.
    QString productTooltip = gs_PRODUCT_NAME_STRING;
    productTooltip.append(" - ");

    productTooltip.append(ToolUtil::GetFormattedVersionString());
    m_pTrayIcon->setToolTip(productTooltip);

    QIcon* icon = new QIcon(":/assets/RDS_Icon.png");
    m_pTrayIcon->setIcon(*icon);
    setWindowIcon(*icon);
    m_pTrayIcon->show();
}

//-----------------------------------------------------------------------------
/// Destroy the Service's system tray icon.
//-----------------------------------------------------------------------------
void MainWindow::DestroyTrayIcon()
{
    SAFE_DELETE(m_pConfigureAction);
    SAFE_DELETE(m_pQuitAction);
    SAFE_DELETE(m_pEnableUWPAction);

    disconnect(m_pTrayIcon, nullptr, this, nullptr);
    SAFE_DELETE(m_pTrayIcon);
}

//-----------------------------------------------------------------------------
/// Create the actions used to signal the Service UI's slots.
//-----------------------------------------------------------------------------
void MainWindow::CreateActions()
{
    // Action for toggling UWP support
    m_pEnableUWPAction = new QAction(gs_ENABLE_UWP_CONTEXT_MENU, this);
    m_pEnableUWPAction->setCheckable(true);
    connect(m_pEnableUWPAction, &QAction::toggled, this, &MainWindow::OnUWPChanged);

    // Action for quitting the application
    m_pQuitAction = new QAction(gs_QUIT_CONTEXT_MENU, this);
    connect(m_pQuitAction, &QAction::triggered, qApp, &QApplication::quit);

    // Action for opening the configuration window
    m_pConfigureAction = new QAction(gs_CONFIGURE_CONTEXT_MENU, this);
    connect(m_pConfigureAction, &QAction::triggered, this, &MainWindow::OnConfigureTriggered);
}

//-----------------------------------------------------------------------------
/// Initialize the listener to start communicating with the driver.
//-----------------------------------------------------------------------------
void MainWindow::InitializeService()
{
    QCommandLineParser parser;

   // Add options
    parser.addHelpOption();
    parser.addOption(QCommandLineOption("port", "RDS listen port", "portnumber"));

    // Process the actual command line arguments given by the user
    parser.process(*qApp);
    unsigned int commandLinePortNum = parser.value("port").toUInt();

    ListenerBindAddress address = {};
    Platform::Strncpy(address.hostAddress, gs_DEFAULT_HOST_ADDRESS, sizeof(char) * kMaxStringLength);

    // Determine port number
    if (commandLinePortNum > 0 && commandLinePortNum <= gs_MAX_LISTEN_PORT)
    {
        // Valid command line option - use command line port
        address.port = commandLinePortNum;
        m_pConfigurationWindow->EnableChangingPort(false);
    }
    else
    {
        // No valid command line option. Use the port specified in user settings file.
        RDSSettings& rdsSettings = RDSSettings::Get();
        address.port = rdsSettings.GetListenPort();
    }

    m_listenPort = address.port;
    emit ListenPortUpdated(m_listenPort);

    ListenerCreateInfo createInfo = {};
    const std::string& listenerNameString = gs_PRODUCT_NAME_STRING.toStdString();
    Platform::Strncpy(createInfo.description, listenerNameString.c_str(), sizeof(char) * kMaxStringLength);
    createInfo.pAddressesToBind = &address;
    createInfo.numAddresses = 1;

    createInfo.flags.enableServer = true;
    createInfo.flags.enableUWP = m_enableUWP;

    DevDriver::AllocCb GenericAllocCb =
    {
        nullptr,
        &ddMemAlloc::GenericAlloc,
        &ddMemAlloc::GenericFree
    };

    createInfo.allocCb = GenericAllocCb;
    createInfo.serverCreateInfo.enabledProtocols.logging = true;
    createInfo.serverCreateInfo.enabledProtocols.etw = true;

    Result initResult = m_listenerCore.Initialize(createInfo);
    if (initResult == Result::Success)
    {
        IMsgChannel* pRdsMessageChannel = m_listenerCore.GetServer()->GetMessageChannel();
        Result registerResult = pRdsMessageChannel->RegisterService(&m_commandService);

        const QString errorCode = ToolUtil::GetResultString(registerResult);

        m_listening = true;
        if (m_enableUWP)
        {
            ToolUtil::DbgMsg("[RDS] Initialized successfully. Now listening for RDP connection. (UWP Enabled)");
        }
        else
        {
            ToolUtil::DbgMsg("[RDS] Initialized successfully. Now listening for RDP connection. (UWP Disabled)");
        }
    }
    else
    {
        ToolUtil::DbgMsg("[RDS] Failed to initialize listener.");
    }
}

//-----------------------------------------------------------------------------
/// Shut down and clean up the service connection.
//-----------------------------------------------------------------------------
void MainWindow::ShutdownService()
{
    if (m_listening)
    {
        IMsgChannel* pRdsMessageChannel = m_listenerCore.GetServer()->GetMessageChannel();
        Result unregisterResult = pRdsMessageChannel->UnregisterService(&m_commandService);
        if (unregisterResult == Result::Success)
        {
            ToolUtil::DbgMsg("[RDS] Successfully unregistered command service.");
        }
        else
        {
            ToolUtil::DbgMsg("[RDS] Failed to unregister command service.");
        }

        m_listenerCore.Destroy();
        m_listening = false;
    }
}

//-----------------------------------------------------------------------------
/// Method to emit process termination signal.
//-----------------------------------------------------------------------------
void MainWindow::EmitTerminateProcess()
{
    emit TerminateProcess();
}

//-----------------------------------------------------------------------------
/// Slot to handle process termination signal.
//-----------------------------------------------------------------------------
void MainWindow::OnTerminateProcessEmitted()
{
    // Quit out of RDS
    qApp->quit();
}

//-----------------------------------------------------------------------------
/// Handler invoked when the RDS listener endpoint settings have changed, whether it's the IP or Port.
//-----------------------------------------------------------------------------
void MainWindow::OnListenEndpointUpdated()
{
    // Shutdown the service.
    ShutdownService();

    // Start the service again. Endpoint configuration is loaded from the user's settings file.
    InitializeService();
}

//-----------------------------------------------------------------------------
/// Handler invoked when the system tray icon is somehow activated.
/// \param reason The reason why this handler was invoked.
//-----------------------------------------------------------------------------
void MainWindow::OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::ActivationReason::DoubleClick)
    {
        ToggleConfigWindowVisibility();
    }
}

//-----------------------------------------------------------------------------
/// Show or hide the configuration window.  Adjust position if its not fully viewable.
//-----------------------------------------------------------------------------
void MainWindow::ToggleConfigWindowVisibility()
{
    if (m_pConfigurationWindow != nullptr)
    {
        if (m_pConfigurationWindow->isHidden() || m_pConfigurationWindow->isMinimized())
        {
            QRect currentGeometry = m_pConfigurationWindow->geometry();
            QPoint availableDesktopTopLeft = QApplication::desktop()->availableGeometry().topLeft();
            QWindow* pWindow = m_pConfigurationWindow->windowHandle();
            if (pWindow != nullptr)
            {
                QScreen* pScreen = pWindow->screen();
                if (pScreen != nullptr)
                {
                    availableDesktopTopLeft = pScreen->availableGeometry().topLeft();
                }
            }

#ifdef Q_OS_WIN
            // Note: For Windows, the dialog needs to be positioned to get the titlebar on screen the
            // first time it is displayed.  The initial titlebar height is 0 for Windows.  Linux does
            // not have this issue.
            const int titlebarHeight = m_pConfigurationWindow->frameGeometry().height() - currentGeometry.height();
#else
            const int titlebarHeight = QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);
#endif
            if (titlebarHeight == 0 || (currentGeometry.x() < availableDesktopTopLeft.x()) || (currentGeometry.y() < availableDesktopTopLeft.y()))
            {
                m_pConfigurationWindow->move(availableDesktopTopLeft.x(), availableDesktopTopLeft.y());
            }

            m_pConfigurationWindow->showNormal();
            m_pConfigurationWindow->setFocus();
        }
        else
        {
            m_pConfigurationWindow->hide();
        }
    }
}

//-----------------------------------------------------------------------------
/// Respond to UWP changed events.
/// \param enableUWP True if UWP support should be enabled.
//-----------------------------------------------------------------------------
void MainWindow::OnUWPChanged(bool enableUWP)
{
    // If we need to change our internal UWP support status, update our
    // internal value and restart the service.
    if (m_enableUWP != enableUWP)
    {
        // Display a messagebox to ask the user if they really meant to enable/disable UWP.
        QMessageBox msgBox(this);

        QString toggleString = gs_STRINGS_DISABLE;
        if (enableUWP)
        {
            toggleString = gs_STRINGS_ENABLE;
        }

        msgBox.setWindowTitle(gs_TOGGLE_UWP_CONFIRMATION_TITLE.arg(toggleString));
        msgBox.setText(gs_TOGGLE_UWP_CONFIRMATION_TEXT.arg(toggleString));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setModal(true);
        int ret = msgBox.exec();

        // If the user wants to proceed, shutdown and restart RDS with the new mode toggled.
        if (ret == QMessageBox::Yes)
        {
            m_enableUWP = enableUWP;

            ShutdownService();
            InitializeService();
        }
        else
        {
            m_pEnableUWPAction->setChecked(m_enableUWP);
        }
    }
}

//-----------------------------------------------------------------------------
/// Open the configuration window.
//-----------------------------------------------------------------------------
void MainWindow::OnConfigureTriggered()
{
    ToggleConfigWindowVisibility();
}
