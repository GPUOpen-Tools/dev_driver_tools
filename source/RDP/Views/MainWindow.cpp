//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The main window for the Radeon Developer Panel.
//=============================================================================

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QIcon>
#include <QApplication>
#include <QCoreApplication>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QScreen>
#include <QSizePolicy>

#include <ScalingManager.h>
#include "../../Common/Util/LogFileWriter.h"
#include "../../Common/Views/DebugWindow.h"
#include "../Util/RDPUtil.h"
#include "../AppSettings/ApplicationSettingsFile.h"
#include "../AppSettings/ApplicationSettingsFileReader.h"
#include "../Models/ApplicationSettingsModel.h"
#include "../Models/DeveloperPanelModel.h"
#include "../Models/RGPTraceModel.h"
#include "../Models/SetupTargetApplicationModel.h"
#include "../Settings/RDPSettings.h"
#include "../RDPDefinitions.h"
#include "ConnectionSettingsView.h"
#include "DriverLoggerView.h"
#include "DriverSettingsView.h"
#include "RGPTraceView.h"
#include "LogView.h"
#include "NotificationWidget.h"
#include "EmptyDriverSettingsView.h"
#include "SetupTargetApplicationView.h"
#include "ClocksView.h"

/// Enumeration of all MainWindow tab Id's.
enum TabID
{
    TAB_ID_CONNECTION,
    TAB_ID_SETTINGS,
    TAB_ID_CLOCKS,
    TAB_ID_PROFILING,
    TAB_ID_LOG,
    TAB_ID_COUNT,
};

/// The title strings for all tabs.
static const QString kTabTitles[] =
{
    "CONNECTION",
    "SETTINGS",
    "CLOCKS",
    "PROFILING",
    "LOG",
};

/// The special style applied to the help button embedded in the main tab bar.
static const QString kHelpButtonStyle = "QPushButton { margin: 0px; padding: 0px; border-radius: 3px; border: 2px solid rgb(135,20,16); font: bold 14px; color: #999; min-width: 30px; min-height: 24px; }"
                                        "QPushButton:hover { border-color: rgb(224,30,55); color: white; }";

//-----------------------------------------------------------------------------
/// Constructor for the Panel's main window.
/// \param pParent The parent widget for the interface.
//-----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* pParent) :
    QMainWindow(pParent),
    ui(new Ui::MainWindow),
    m_pDeveloperPanelModel(nullptr),
    m_pConnectionSettingsView(nullptr),
    m_pDriverLoggerView(nullptr),
    m_pRedIndicatorIcon(nullptr),
    m_pGreenIndicatorIcon(nullptr),
    m_messageOverlayContainer(this),
    m_isExiting(false),
    m_rdsConnected(false),
    m_lostRDSConnection(false)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/images/RDP_Icon.png"));

    // Set white background for this pane
    ToolUtil::SetWidgetBackgroundColor(this, Qt::white);

    // Set grey background for main tab bar background
    ToolUtil::SetWidgetBackgroundColor(ui->mainTabWidget,   gs_PRODUCT_COLOR_GUNMETAL);

    // Set grey background for the applications tab bar background
    ToolUtil::SetWidgetBackgroundColor(ui->connectionTab,   gs_PRODUCT_COLOR_GUNMETAL);
    ToolUtil::SetWidgetBackgroundColor(ui->settingsTab,     gs_PRODUCT_COLOR_GUNMETAL);
    ToolUtil::SetWidgetBackgroundColor(ui->clocksTab,       gs_PRODUCT_COLOR_GUNMETAL);
    ToolUtil::SetWidgetBackgroundColor(ui->profilingTab,    gs_PRODUCT_COLOR_GUNMETAL);
    ToolUtil::SetWidgetBackgroundColor(ui->logTab,          gs_PRODUCT_COLOR_GUNMETAL);

    // Update the window title to include the version number.
    QString windowTitleString = gs_PRODUCT_NAME_STRING;
    windowTitleString.append(" - ");
    const QString& versionString = ToolUtil::GetFormattedVersionString();
    windowTitleString.append(versionString);
    this->setWindowTitle(windowTitleString);

    InitializeInterfaceAndSettings();

    ToggleConnectedTabs(false);
}

//-----------------------------------------------------------------------------
/// Destructor for the Panel's main window.
//-----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    // Destroy all tab widget views and their associated models.
    m_pDeveloperPanelModel->UnregisterModel(MAIN_PANEL_MODEL_CONNECTION_SETTINGS);

    // Allow message box to be displayed when RDS disconnects.
    m_pConnectionSettingsView->m_disableRdsDisconnectNotification = true;
    SAFE_DELETE(m_pConnectionSettingsView);
    SAFE_DELETE(m_pDriverSettingsView);
    SAFE_DELETE(m_pDriverLoggerView);
    SAFE_DELETE(m_pRGPTraceView);
    SAFE_DELETE(m_pLogView);

    SAFE_DELETE(m_pRedIndicatorIcon);
    SAFE_DELETE(m_pGreenIndicatorIcon);
    SAFE_DELETE(m_pNotificationOverlay);

//    SAFE_DELETE(m_pDeveloperPanelModel);
    SAFE_DELETE(ui);
}

//-----------------------------------------------------------------------------
/// Initialize the main RDP window, loading settings for anything that needs to be re-populated from disk.
//-----------------------------------------------------------------------------
void MainWindow::InitializeInterfaceAndSettings()
{
    // create the log view
    m_pLogView = new LogView(this);
    ui->logTabLayoutGrid->addWidget(m_pLogView);

    // The DeveloperPanelModel is the Panel's main model, and maintains a connection to RDS.
    m_pDeveloperPanelModel = new DeveloperPanelModel;

    InitializeToolbar();

    // Attach the debug output to forward to "this" window's log message handler.
    RDPUtil::RegisterLogWindow(this);
    connect(this, SIGNAL(EmitSetText(QString)), this, SLOT(OnLogText(QString)));

    int mainWidth = gs_PRODUCT_DEFAULT_WIDTH;
    int mainHeight = gs_PRODUCT_DEFAULT_HEIGHT;

    int mainLocX = gs_DESKTOP_MARGIN;
    int mainLocY = gs_DESKTOP_MARGIN * 2;

    // Load the application's default settings and any user modifications to them.
    static RDPSettings& rdpSettings = RDPSettings::Get();
    if (rdpSettings.LoadSettings())
    {
        RDPUtil::DbgMsg("[RDP] Loaded RDP settings file.");

        // Overwrite mainWidth, mainHeight, mainLocX, and mainLocY
        mainWidth =     rdpSettings.GetWindowWidth();
        mainHeight =    rdpSettings.GetWindowHeight();
        mainLocX =      rdpSettings.GetWindowXPos();
        mainLocY =      rdpSettings.GetWindowYPos();
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Failed to load RDP settings file. Will use default settings.");
    }

    // set the default geometry
    setGeometry(0, 50, mainWidth, mainHeight);

    // if the saved coordinates still exist,
    // use them
    QPoint currentPos = QPoint(mainLocX, mainLocY);
    for (QScreen *pScreen : QGuiApplication::screens())
    {
        QRect screenRect = pScreen->geometry();
        if (screenRect.contains(currentPos))
        {
            setGeometry(mainLocX, mainLocY, mainWidth, mainHeight);
        }
    }

    // If we've loaded the settings file and there aren't any recent connections, we're starting fresh.
    // Manually add the first connection to "localhost" so the user doesn't have to.
    if (rdpSettings.GetRecentConnections().isEmpty())
    {
        // RDP will always start fresh with "localhost" added to the available connections table.
        RDSConnectionInfo localhostConnection   = {};
        localhostConnection.autoconnect       = true;
        localhostConnection.hostnameString    = gs_LOCAL_HOST;
        localhostConnection.ipString          = gs_LOCAL_HOST;
        localhostConnection.port              = gs_DEFAULT_CONNECTION_PORT;

        rdpSettings.AddRecentConnection(localhostConnection);
    }

    // If the Developer Panel Model isn't created successfully, the entire application cannot function.
    if (m_pDeveloperPanelModel != nullptr)
    {
        // Create the contents of the main "Connection" tab.
        m_pConnectionSettingsView = new ConnectionSettingsView(m_pDeveloperPanelModel, this);
        ui->connectionTabLayoutGrid->addWidget(m_pConnectionSettingsView);
        m_tabs.push_back(ui->connectionTab);

        // The main window will be updated if the connection status is changed.
        connect(m_pConnectionSettingsView, &ConnectionSettingsView::ConnectionStatusUpdated, this, &MainWindow::OnConnectionStatusUpdated);

        // Get notified when RDS quits.
        connect(m_pConnectionSettingsView, &ConnectionSettingsView::LostRDSConnection,       this, &MainWindow::OnLostRDSConnection);

        // Populate the driver settings interface once the data has been loaded.
        connect(m_pDeveloperPanelModel, SIGNAL(DriverSettingsPopulated(int)), this, SLOT(OnDriverSettingsPopulated(int)));

        // Read the driver settings file
        RDPApplicationSettingsFile* pFileInfo = new RDPApplicationSettingsFile;
        pFileInfo->filepath = ToolUtil::GetDriverToolsXmlFileLocation() + "/DriverSettings.rds"; // @TODO: Store file info for driver settings file somewhere better
        ApplicationSettingsFile* pApplicationSettingsFile = RDPSettings::Get().ReadApplicationSettingsFile(pFileInfo);

        // No driver settings file - create a new one
        if (pApplicationSettingsFile == nullptr)
        {
            pApplicationSettingsFile = new ApplicationSettingsFile;
            pApplicationSettingsFile->SetFileInfo(pFileInfo);
            RDPSettings::Get().WriteApplicationSettingsFile(pApplicationSettingsFile);
        }

        // Create the contents of the main "Settings" tab
        ApplicationSettingsModel* pAppSettingsModel = m_pDeveloperPanelModel->AddNewApplicationSettings(pApplicationSettingsFile);

        m_pDriverSettingsView = new DriverSettingsView(m_pDeveloperPanelModel, pAppSettingsModel, this);
        ui->settingsTabLayoutGrid->addWidget(m_pDriverSettingsView);
        m_tabs.push_back(ui->settingsTab);

        // Create the contents of the main "Clocks" tab
        m_pClocksView = new ClocksView(m_pDeveloperPanelModel, this);
        ui->clocksTabLayoutGrid->addWidget(m_pClocksView);
        m_tabs.push_back(ui->clocksTab);

        // Create the contents of the main "Profiling" tab.
        m_pRGPTraceView = new RGPTraceView(m_pDeveloperPanelModel, pAppSettingsModel, this);
        ui->protocolsTabLayoutGrid->addWidget(m_pRGPTraceView);
        m_tabs.push_back(ui->profilingTab);

        // The log tab is the last in the list.
        m_tabs.push_back(ui->logTab);

        // Connect the slot that updates the profiling tab's target process section.
        connect(m_pDeveloperPanelModel, &DeveloperPanelModel::ProfiledProcessInfoUpdate, m_pRGPTraceView, &RGPTraceView::OnProfilingTargetUpdated);

        // Connect the slot that warns the user if multiple applications targeted for profiling are started.
        connect(m_pDeveloperPanelModel, &DeveloperPanelModel::MultipleProfilerTargetsStarted, m_pConnectionSettingsView->GetSetupTargetApplicationView(), &SetupTargetApplicationView::OnProfilingMultipleTargetsWarning);

        // Connect the slot that updates profiling tab's trace in progress indicator
        connect((RGPTraceModel*)m_pDeveloperPanelModel->GetProtocolModel(MAIN_PANEL_MODEL_RGP), &RGPTraceModel::CurrentlyCollectingTrace, m_pConnectionSettingsView->GetSetupTargetApplicationView(), &SetupTargetApplicationView::OnTraceCollectionStatusUpdated);

        // Connect the slot that updates process name and id in profiling tab
        connect(m_pConnectionSettingsView->GetSetupTargetApplicationView()->GetSetupTargetApplicationModel(), &SetupTargetApplicationModel::ProfilingCheckboxUnchecked, (RGPTraceModel*)m_pDeveloperPanelModel->GetProtocolModel(MAIN_PANEL_MODEL_RGP), &RGPTraceModel::OnApplicationUnchecked);

        // Connect the slot that warns if the user attempts to modify the profiled target application when another one is already being profiled.
        connect(m_pConnectionSettingsView->GetSetupTargetApplicationView()->GetSetupTargetApplicationModel(), &SetupTargetApplicationModel::ProfilerInUseWarning, m_pConnectionSettingsView->GetSetupTargetApplicationView(), &SetupTargetApplicationView::OnProfilerInUseWarning);

        // Connect the slot that allows the SetupTargetApplicationModel to query which, if any, process is actively being profiled
        connect(m_pConnectionSettingsView->GetSetupTargetApplicationView()->GetSetupTargetApplicationModel(), &SetupTargetApplicationModel::QueryProfiledTargetInfo, m_pDeveloperPanelModel, &DeveloperPanelModel::OnProfiledTargetInfoQuery);

        // Default to the connection tab.
        ui->mainTabWidget->setCurrentIndex(TAB_ID_CONNECTION);

        InitializeConnectionIndicator();

        // Set the mouse cursor to pointing hand cursor for all of the tabs
        QList<QTabBar*> tabBar = ui->mainTabWidget->findChildren<QTabBar*>();
        foreach(QTabBar* pItem, tabBar)
        {
            if (pItem != nullptr)
            {
                pItem->setCursor(Qt::PointingHandCursor);
            }
        }

        // @TODO: We should start with most of the interface disabled. Need to connect to RDS to do anything.
        SetConnectedControlsEnabled(true);
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Failed to create the Developer Panel model.");
    }

    // Configure the notification overlay and hide it initially.
    m_pNotificationOverlay = new NotificationWidget(&m_messageOverlayContainer);

    // Initialize the background and foreground widgets for the overlay container.
    m_messageOverlayContainer.SetBackgroundWidget(ui->centralWidget);
    m_messageOverlayContainer.SetOverlayWidget(m_pNotificationOverlay);
    m_messageOverlayContainer.HideOverlay();
}

//-----------------------------------------------------------------------------
/// Handles notification for another instance of this application starting.
/// Brings main window to the foreground.
//-----------------------------------------------------------------------------
void MainWindow::OnAppInstanceStarted()
{
    RDPUtil::DbgMsg("[RDP] Another instance of RDP was detected. Bringing primary instance to foreground.");

    BringToForeground();
}

//-----------------------------------------------------------------------------
/// Event filter for intercepting resize events in the tab widget. This is a hack
/// to stick a button into the right side of the tab bar. When the tab widget
/// size changes, this updates the size of the last tab so it fills the entire
/// width.
/// \param obj A pointer to the QObject that is receiving the event.
/// \param event The QEvent that is being intercepted.
/// \return false if event handling should continue as normal or true if event
/// handling should not be passed any further.
//-----------------------------------------------------------------------------
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize && obj == ui->mainTabWidget)
    {
        // Get size from event
        QResizeEvent *resizeEvent = static_cast<QResizeEvent *>(event);
        QSize size = resizeEvent->size();

        // Calculate width of last tab such that it fills the entire tab bar width
        int tabWidth = ui->mainTabWidget->tabBar()->tabRect(0).width();

        // Add +5 to account for right side padding which we can't disable
        int lastTabWidth = size.width() - (tabWidth * (ui->mainTabWidget->count() - 1)) + 5;

        // Width adjustments to account for scaling
        ScalingManager& sm = ScalingManager::Get();
        lastTabWidth -= sm.Scaled(8);

        // Set the width of the last tab
        ui->mainTabWidget->setStyleSheet(QString("QTabBar::tab:last { width: %1px; padding: 0px;}").arg(lastTabWidth));
    }

    return QObject::eventFilter(obj, event);
}

//-----------------------------------------------------------------------------
/// Overridden handler invoked when the main RDP window has been requested to close.
/// \param pEvent The close event arguments.
//-----------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent* pEvent)
{
    Q_UNUSED(pEvent);

    m_isExiting = true;

    // Connect to RDS just in case currently disconnected, so the tray icon gets removed.
    // Only connect if this instance of the panel created the service.
    if (m_lostRDSConnection == false && m_pConnectionSettingsView->CreatedRdsProcess())
    {
        if (m_rdsConnected == false)
        {
            m_pConnectionSettingsView->OnConnectClicked();
            m_rdsConnected = true;
        }

        // Send a request to terminate the connected RDS process.
        m_pDeveloperPanelModel->TerminateConnectedRDS();

        if (m_rdsConnected == true)
        {
            m_pConnectionSettingsView->OnDisconnectClicked();
        }
    }
}

//-----------------------------------------------------------------------------
/// Initialize the main RDP toolbar.
//-----------------------------------------------------------------------------
void MainWindow::InitializeToolbar()
{
    InitializeConnectionIndicator();

    // Insert the help button item into the end of the toolbar.
    InitializeHelpButton();
}

//-----------------------------------------------------------------------------
/// Initialize the toolbar by creating tabs and the help button.
//-----------------------------------------------------------------------------
void MainWindow::InitializeHelpButton()
{
    // Create help button
    QPushButton* pHelpButton = new QPushButton("?");

    // Set stylesheets
    pHelpButton->setStyleSheet(kHelpButtonStyle);

    // Add widgets to the toolbar
    ui->mainTabWidget->setToolbarContentsMargins(32, 4, 8, 4);
    ui->mainTabWidget->addWidgetToToolbar(pHelpButton, QTabBar::RightSide);

    // Signal/slot connection
    connect(pHelpButton, &QPushButton::clicked, this, &MainWindow::OnHelpButtonPressed);
}

//-----------------------------------------------------------------------------
/// Slot to handle what happens when a log message gets posted. Add the log
/// message to the log window contents and scroll to the bottom of the text
/// \param str The string to be added to the log
//-----------------------------------------------------------------------------
void MainWindow::OnLogText(const QString& str)
{
    // Write to the log file.
    LogFileWriter::Get().WriteLog(str);

    // If the log window is alive, display the new line.
    if (m_pLogView != nullptr)
    {
        m_pLogView->AddLogMessage(str);
    }
}

//-----------------------------------------------------------------------------
/// Slot to handle what happens when the scaling factor changes. When the user
/// changes the DPI scaling or when dragging the application between monitors
/// with different DPI scale settings
/// \param oldScaleFactor The old scaling factor
/// \param newScaleFactor The new scaling factor
//-----------------------------------------------------------------------------
void MainWindow::ScalingFactorChanged(double oldScaleFactor, double newScaleFactor)
{
    double width = (double)(RDPSettings::Get().GetWindowWidth());
    double height = (double)(RDPSettings::Get().GetWindowHeight());

    double scaleFactor = newScaleFactor / oldScaleFactor;

    width *= scaleFactor;
    height *= scaleFactor;

    Q_UNUSED(width);
    Q_UNUSED(height);
}

//-----------------------------------------------------------------------------
/// Overridden window resize event. Handle what happens when a user resizes the main window.
/// \param pEvent The resize event object.
//-----------------------------------------------------------------------------
void MainWindow::resizeEvent(QResizeEvent* pEvent)
{
    m_messageOverlayContainer.resize(pEvent->size());
    RDPSettings::Get().SetWindowSize(pEvent->size().width(), pEvent->size().height());
}

//-----------------------------------------------------------------------------
/// Overridden window move event. Handle what happens when a user moves the main window.
/// \param pEvent The move event object.
//-----------------------------------------------------------------------------
void MainWindow::moveEvent(QMoveEvent* pEvent)
{
    RDPSettings::Get().SetWindowPos(pEvent->pos().x(), pEvent->pos().y());
}

//-----------------------------------------------------------------------------
/// A helper to disable all interface tabs that require a connection to RDS.
/// \param enabled True to enable all interfaces that require connection to RDS. False to disable.
//-----------------------------------------------------------------------------
void MainWindow::SetConnectedControlsEnabled(bool enabled)
{
    Q_UNUSED(enabled);
}

//-----------------------------------------------------------------------------
/// Initialize the indicator that lives in the connection tab header.
//-----------------------------------------------------------------------------
void MainWindow::InitializeConnectionIndicator()
{
    m_pRedIndicatorIcon = new QIcon(":/images/RedIndicator.png");
    m_pGreenIndicatorIcon = new QIcon(":/images/GreenIndicator.png");
}

//-----------------------------------------------------------------------------
/// Handler invoked when the Driver Settings data has finished being loaded from file.
/// \param modelIndex The index of the associated model that has been updated.
//-----------------------------------------------------------------------------
void MainWindow::OnDriverSettingsPopulated(int modelIndex)
{
    Q_UNUSED(modelIndex);
    m_pDriverSettingsView->PopulateSettingsInterface();
}

//-----------------------------------------------------------------------------
/// Handler invoked when the connection status to RDS is updated.
/// \param connected True if RDP is connected, and false if it's disconnected.
//-----------------------------------------------------------------------------
void MainWindow::OnConnectionStatusUpdated(bool connected, const QString& hostConnectionString)
{
    Q_UNUSED(hostConnectionString);

    ToggleConnectedTabs(connected);

    // Save the current status.
    if (hostConnectionString.isEmpty() && connected == false)
    {
        m_rdsConnected = false;
    }
    else if (connected == true)
    {
        m_rdsConnected = true;
        m_lostRDSConnection = false;
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when RDP loses connection with RDS because RDS quit.
//-----------------------------------------------------------------------------
void MainWindow::OnLostRDSConnection()
{
    // Save the current status
    m_lostRDSConnection = true;
}

//-----------------------------------------------------------------------------
/// Handler invoked when the "Help" button is pressed.
//-----------------------------------------------------------------------------
void MainWindow::OnHelpButtonPressed()
{
    // Open help file
    QUrl fileUrl = QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/docs/help/rdp/html/index.html");
    QDesktopServices::openUrl(fileUrl);
}

//-----------------------------------------------------------------------------
/// Toggle the visibility of tabs that are shown/hidden based on the connected status.
/// \param enabled If true, the tabs are shown. If false, they are hidden.
//-----------------------------------------------------------------------------
void MainWindow::ToggleConnectedTabs(bool enabled)
{
    if (enabled)
    {
        // Add the tabs in reverse order so the tab indices remain correct as more tabs are added.
        ui->mainTabWidget->insertTab(TAB_ID_SETTINGS, m_tabs[TAB_ID_SETTINGS], kTabTitles[TAB_ID_SETTINGS]);
        ui->mainTabWidget->insertTab(TAB_ID_CLOCKS, m_tabs[TAB_ID_CLOCKS], kTabTitles[TAB_ID_CLOCKS]);
        ui->mainTabWidget->insertTab(TAB_ID_PROFILING, m_tabs[TAB_ID_PROFILING], kTabTitles[TAB_ID_PROFILING]);

        // The "CONNECTIONS" tab toggles between CONNECTIONS and STATUS.
        ui->mainTabWidget->tabBar()->setTabIcon(TAB_ID_CONNECTION, *m_pGreenIndicatorIcon);
    }
    else
    {
        ui->mainTabWidget->removeTab(TAB_ID_PROFILING);
        ui->mainTabWidget->removeTab(TAB_ID_CLOCKS);
        ui->mainTabWidget->removeTab(TAB_ID_SETTINGS);

        // Toggle the tab text back to CONNECTIONS.
        ui->mainTabWidget->tabBar()->setTabIcon(TAB_ID_CONNECTION, *m_pRedIndicatorIcon);
    }

    // In either case, the CONNECTION/STATUS tab should be selected when the connection status is updated.
    ui->mainTabWidget->setCurrentIndex(TAB_ID_CONNECTION);
}

//-----------------------------------------------------------------------------
/// Open the profiling tab
//-----------------------------------------------------------------------------
void MainWindow::OpenProfilingTab()
{
    ui->mainTabWidget->setCurrentIndex(TAB_ID_PROFILING);
}

//-----------------------------------------------------------------------------
/// Set the enabled state of the disconnect button.
/// \param enabled Whether the button should be enabled.
//-----------------------------------------------------------------------------
void MainWindow::SetDisconnectButtonEnabled(bool enabled)
{
    m_pConnectionSettingsView->SetDisconnectButtonEnabled(enabled);
}

//-----------------------------------------------------------------------------
/// Show the Notification overlay on top of RDP.
/// \param title The title text to display within the notification.
/// \param text The text to display within the notification.
/// \param buttons The buttons to display within the notification.
/// \param defaultButton An enum button bit for the default button (or zero to use the
/// highest bit value from the buttons parameter).
/// \return The enumeration member of the button that was clicked by the user.
//-----------------------------------------------------------------------------
NotificationWidget::Button MainWindow::ShowNotification(const QString& title, const QString& text, uint buttons, uint defaultButton)
{
    m_pNotificationOverlay->SetTitle(title);
    m_pNotificationOverlay->SetText(text);
    m_pNotificationOverlay->SetButtons(buttons, defaultButton);
    m_pNotificationOverlay->ShowDoNotAsk(false);

    m_messageOverlayContainer.ShowOverlay();

    // Bring the window to the foreground to get the user's attention if RDP is hidden.
    BringToForeground();

    // Loop and process events until the user selects an option in the notification overlay.
    while ((m_pNotificationOverlay->GetResult() == NotificationWidget::Unset) && (m_isExiting == false))
    {
        QCoreApplication::processEvents();

        // Wait a bit before checking for new events.
        QThread::msleep(10);
    }

    NotificationWidget::Button buttonResult = m_pNotificationOverlay->GetResult();
    m_messageOverlayContainer.HideOverlay();

    return buttonResult;
}

//-----------------------------------------------------------------------------
/// Show the Notification overlay on top of RDP.
/// \param title The title text to display within the notification.
/// \param text The text to display within the notification.
/// \param buttons The buttons to display within the notification.
/// \param showDoNotAsk A flag that indicates if the "Do not ask again" checkbox will be displayed.
/// \param defaultButton An enum button bit for the default button (or zero to use the
/// highest bit value from the buttons parameter).
/// \return The enumeration member of the button that was clicked by the user.
//-----------------------------------------------------------------------------
NotificationWidget::Button MainWindow::ShowNotification(const QString& title, const QString& text, uint buttons, bool& showDoNotAsk, uint defaultButton)
{
    bool checkboxShown = showDoNotAsk;

    m_pNotificationOverlay->SetTitle(title);
    m_pNotificationOverlay->SetText(text);
    m_pNotificationOverlay->SetButtons(buttons, defaultButton);
    m_pNotificationOverlay->ShowDoNotAsk(checkboxShown);
    m_messageOverlayContainer.ShowOverlay();

    // Bring the window to the foreground to get the user's attention if RDP is hidden.
    BringToForeground();

    // Loop and process events until the user selects an option in the notification overlay.
    while ((m_pNotificationOverlay->GetResult() == NotificationWidget::Unset) && (m_isExiting == false))
    {
        QCoreApplication::processEvents();

        // Wait a bit before checking for new events.
        QThread::msleep(10);
    }

    // If the "Do not ask again" checkbox was displayed, return the checked status as an outparam.
    if (checkboxShown)
    {
        showDoNotAsk = !(m_pNotificationOverlay->GetIsDoNotAskChecked());
    }

    NotificationWidget::Button buttonResult = m_pNotificationOverlay->GetResult();
    m_messageOverlayContainer.HideOverlay();

    return buttonResult;
}

//-----------------------------------------------------------------------------
/// Bring the main RDP window to the foreground.
//-----------------------------------------------------------------------------
void MainWindow::BringToForeground()
{
#ifdef Q_OS_WIN
    SetWindowPos((HWND)this->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    setWindowState(Qt::WindowActive);
    raise();
    showNormal();
    show();
    SetWindowPos((HWND)this->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#else
    raise();
    showNormal();
    // Note: On Linux, setWindowState(Qt::WindowActive) fails to restore window and also prevents raise() from bring the window to the foreground.
#endif // Q_OS_WIN
}
