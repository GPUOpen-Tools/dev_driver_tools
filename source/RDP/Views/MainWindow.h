//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The main window for the Radeon Developer Panel.
//=============================================================================

#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <QEvent>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include "NotificationWidget.h"

class QAction;
class QMenu;
class QPushButton;
class ApplicationSettingsModel;
class ApplicationSettingsFile;
class ConnectionSettingsView;
class ClocksView;
class DebugWindow;
class DeveloperPanelModel;
class DriverSettingsView;
class DriverLoggerView;
class LogView;
class NotificationWidget;
class ProcessInfo;
class RGPTraceView;

namespace Ui {
class MainWindow;
}

/// The main window instance for the Radeon Developer Panel.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* pParent = nullptr);
    ~MainWindow();

    void InitializeInterfaceAndSettings();
    void InitializeToolbar();
    void InitializeHelpButton();
    void ScalingFactorChanged(double oldScaleFactor, double newScaleFactor);
    virtual void resizeEvent(QResizeEvent* pEvent);
    virtual void moveEvent(QMoveEvent* pEvent);
    void SetConnectedControlsEnabled(bool enabled);
    void InitializeConnectionIndicator();
    void OpenProfilingTab();
    void SetDisconnectButtonEnabled(bool enabled);
    NotificationWidget::Button ShowNotification(const QString& title, const QString& text, uint buttons, uint defaultButton);
    NotificationWidget::Button ShowNotification(const QString& title, const QString& text, uint buttons, bool& showDoNotAsk, uint defaultButton);

signals:
    void EmitSetText(const QString& str);

public slots:
    void OnDriverSettingsPopulated(int modelIndex);
    void OnAppInstanceStarted();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void closeEvent(QCloseEvent* pEvent);

private slots:
    void OnConnectionStatusUpdated(bool connected, const QString& hostConnectionString);
    void OnLogText(const QString& str);
    void OnHelpButtonPressed();
    void OnLostRDSConnection();

private:
    void BringToForeground();
    void ToggleConnectedTabs(bool enabled);

    Ui::MainWindow* ui;
    DeveloperPanelModel* m_pDeveloperPanelModel;        ///< The main model instance for RDP.
    ConnectionSettingsView* m_pConnectionSettingsView;  ///< The Connection Settings view.
    DriverSettingsView* m_pDriverSettingsView;          ///< The Driver Settings view.
    DriverLoggerView* m_pDriverLoggerView;              ///< The Driver Logger view.
    RGPTraceView* m_pRGPTraceView;                      ///< The pane holding the contents of the "Profiling" tab.
    ClocksView* m_pClocksView;                          ///< The pane holding the contents of the "Clocks" tab.
    LogView*      m_pLogView;                           ///< The pane holding the contents of the log.
    QIcon* m_pRedIndicatorIcon;                         ///< The red connectedness indicator icon.
    QIcon* m_pGreenIndicatorIcon;                       ///< The green connectedness indicator icon.
    NotificationWidget* m_pNotificationOverlay;         ///< A notification widget that can be overlaid on top of RDP.
    ContainerWidget m_messageOverlayContainer;          ///< The container used to overlay a message within the RDP interface.
    bool m_isExiting;                                   ///< A flag that indicates if the Panel process is closing.
    QVector<QWidget*> m_tabs;                           ///< A list of tabs found in the RDP interface.
    bool m_rdsConnected;                                ///< The boolean to indicate whether an instance of RDS is currently running.
    bool m_lostRDSConnection;                           ///< The boolean to indicate if RDS quit while RDP was still connected to it.
};

#endif // _MAIN_WINDOW_H_
