#-------------------------------------------------
#
# Project created by QtCreator 2016-08-26T03:53:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RadeonDeveloperPanel
TEMPLATE = app

# This is to solve "cannot convert parameter 1 from 'char *' to 'LPCWSTR'" problem when compiling the back-end
DEFINES -= UNICODE
DEFINES += _MBCS
DEFINES += GPUOPEN_CLIENT_INTERFACE_MAJOR_VERSION=26
DEFINES += DEVDRIVER_UM_BUILD=1

# Add a \ at the end of a line to continue declarations on the next line.
SOURCES += \
    ../source/RDP/PanelMain.cpp \
    ../source/RDP/Views/MainWindow.cpp \
    ../source/Common/ModelViewMapper.cpp \
    ../source/Common/ToolbarTabWidget.cpp \
    ../source/RDP/Views/AppListTreeView.cpp \
    ../source/RDP/Views/ConnectionSettingsView.cpp \
    ../source/RDP/Views/DriverSettingItemWidget.cpp \
    ../source/RDP/Views/DriverSettingsView.cpp \
    ../source/RDP/Views/EmptyDriverSettingsView.cpp \
    ../source/RDP/Views/RecentConnectionsView.cpp \
    ../source/RDP/Views/NewConnectionView.cpp \
    ../source/RDP/Views/SetupTargetApplicationView.cpp \
    ../source/RDP/Models/ConnectionSettingsModel.cpp \
    ../source/RDP/Models/DriverSettingsModel.cpp \
    ../source/RDP/Models/DeveloperPanelModel.cpp \
    ../source/RDP/Models/RGPTraceModel.cpp \
    ../source/RDP/Views/DriverLoggerView.cpp \
    ../source/RDP/Views/RGPTraceView.cpp \
    ../source/RDP/Views/LogView.cpp \
    ../source/DevDriverComponents/src/ddTransferManager.cpp \
    ../source/DevDriverComponents/src/protocols/ddTransferClient.cpp \
    ../source/DevDriverComponents/src/protocols/ddTransferServer.cpp \
    ../source/DevDriverComponents/src/protocols/ddURIServer.cpp \
    ../source/DevDriverComponents/src/ddClientURIService.cpp \
    ../source/DevDriverComponents/src/protocols/driverControlClient.cpp \
    ../source/DevDriverComponents/src/protocols/loggingClient.cpp \
    ../source/DevDriverComponents/src/protocols/rgpClient.cpp \
    ../source/DevDriverComponents/src/protocols/settingsClient.cpp \
    ../source/DevDriverComponents/src/protocols/etwClient.cpp \
    ../source/RDP/Models/DriverLogBackgroundWorker.cpp \
    ../source/RDP/Models/DriverLoggingModel.cpp \
    ../source/RDP/Models/DriverLogfileModel.cpp \
    ../source/RDP/Models/DriverProtocolModel.cpp \
    ../source/Common/Views/DebugWindow.cpp \
    ../source/Common/ToolUtil.cpp \
    ../source/Common/Process.cpp \
    ../source/RDP/Settings/RDPSettings.cpp \
    ../source/RDP/Settings/RDPSettingsReader.cpp \
    ../source/RDP/Settings/RDPSettingsWriter.cpp \
    ../source/RDP/Models/ApplicationSettingsModel.cpp \
    ../source/RDP/Models/DriverMessageProcessorThread.cpp \
    ../source/RDP/Models/RecentConnectionsModel.cpp \
    ../source/RDP/Models/SetupTargetApplicationModel.cpp \
    ../source/RDP/AppSettings/ApplicationSettingsFile.cpp \
    ../source/RDP/AppSettings/ApplicationSettingsFileReader.cpp \
    ../source/RDP/AppSettings/ApplicationSettingsFileWriter.cpp \
    ../source/RDP/Models/RGPRecentTraceListModel.cpp \
    ../source/RDP/Models/ProcessInfoModel.cpp \
    ../source/RDP/Util/RDPUtil.cpp \
    ../source/Common/Scaling/ScaledGridLayoutWrapper.cpp \
    ../source/Common/Scaling/ScaledLayoutWrapper.cpp \
    ../source/Common/Scaling/ScaledWidgetWrapper.cpp \
    ../source/Common/Scaling/ScalingManager.cpp \
    ../source/RDP/Views/ConnectionStatusView.cpp \
    ../source/RDP/Views/CaptureProgressWidget.cpp \
    ../source/RDP/Views/ClocksView.cpp \
    ../source/RDP/Views/ActiveApplicationsTableView.cpp \
    ../source/RDP/Models/ActiveApplicationTableModel.cpp \
    ../source/RDP/Views/NotificationWidget.cpp \
    ../source/RDP/Views/ClockModeWidget.cpp \
    ../source/Common/Util/SingleApplicationInstance.cpp \
    ../source/Common/Util/SystemKeyboardHook.cpp \
    ../source/RDP/Models/ConnectionAttemptWorker.cpp \
    ../source/RDP/Models/ConnectionStatusWorker.cpp \
    ../source/RDP/Models/RGPClientProcessorThread.cpp \
    ../source/Common/Util/LogFileWriter.cpp \
    ../source/RDP/Models/ClocksTabModel.cpp \
    ../source/RDP/Models/DeviceClockModeModel.cpp \
    ../source/RDP/Views/RDPLogBox.cpp \
    ../../QtCommon/Util/QtUtil.cpp

# The following DevDriverComponents are from the driver team's source drop.
SOURCES += \
    ../source/DevDriverComponents/src/socketMsgTransport.cpp \
    ../source/DevDriverComponents/src/session.cpp \
    ../source/DevDriverComponents/src/sessionManager.cpp \
    ../source/DevDriverComponents/src/baseProtocolServer.cpp \
    ../source/DevDriverComponents/src/baseProtocolClient.cpp \
    ../source/DevDriverComponents/src/devDriverClient.cpp

win32 {
SOURCES += \
    ../source/DevDriverComponents/src/win/ddWinPipeMsgTransport.cpp \
    ../source/DevDriverComponents/src/win/ddWinPlatform.cpp \
    ../source/DevDriverComponents/src/win/ddWinSocket.cpp \
}

unix {
SOURCES += \
    ../source/DevDriverComponents/src/lnx/ddLnxPlatform.cpp \
    ../source/DevDriverComponents/src/lnx/ddLnxSocket.cpp
}

HEADERS  += \
    ../source/Common/ModelViewMapper.h \
    ../source/Common/ToolbarTabWidget.h \
    ../source/Common/DriverToolsDefinitions.h \
    ../source/Common/ToolUtil.h \
    ../source/Common/Version.h \
    ../source/Common/Views/DebugWindow.h \
    ../source/RDP/Views/MainWindow.h \
    ../source/RDP/Views/AppListTreeView.h \
    ../source/RDP/Views/ConnectionSettingsView.h \
    ../source/RDP/Views/DriverSettingItemWidget.h \
    ../source/RDP/Views/DriverSettingsView.h \
    ../source/RDP/Views/EmptyDriverSettingsView.h \
    ../source/RDP/Views/RecentConnectionsView.h \
    ../source/RDP/Views/NewConnectionView.h \
    ../source/RDP/Views/SetupTargetApplicationView.h \
    ../source/RDP/Models/ConnectionSettingsModel.h \
    ../source/RDP/Models/DriverSettingsModel.h \
    ../source/RDP/Models/DeveloperPanelModel.h \
    ../source/RDP/Models/RGPTraceModel.h \
    ../source/RDP/Views/DriverLoggerView.h \
    ../source/RDP/Views/RGPTraceView.h \
    ../source/RDP/Views/LogView.h \
    ../source/DevDriverComponents/inc/msgTransport.h \
    ../source/DevDriverComponents/src/session.h \
    ../source/DevDriverComponents/src/sessionManager.h \
    ../source/DevDriverComponents/src/socket.h \
    ../source/DevDriverComponents/inc/devDriverClient.h \
    ../source/DevDriverComponents/inc/protocols/driverControlClient.h \
    ../source/DevDriverComponents/inc/protocols/loggingClient.h \
    ../source/DevDriverComponents/inc/protocols/rgpClient.h \
    ../source/DevDriverComponents/inc/protocols/settingsClient.h \
    ../source/RDP/Models/DriverLogBackgroundWorker.h \
    ../source/RDP/Models/DriverLoggingModel.h \
    ../source/RDP/Models/DriverLogfileModel.h \
    ../source/RDP/Models/DriverProtocolModel.h \
    ../source/Common/Process.h \
    ../source/RDP/Settings/RDPSettings.h \
    ../source/RDP/Settings/RDPSettingsReader.h \
    ../source/RDP/Settings/RDPSettingsWriter.h \
    ../source/RDP/Models/ApplicationSettingsModel.h \
    ../source/RDP/Models/DriverMessageProcessorThread.h \
    ../source/RDP/Models/RecentConnectionsModel.h \
    ../source/RDP/Models/SetupTargetApplicationModel.h \
    ../source/RDP/AppSettings/ApplicationSettingsFile.h \
    ../source/RDP/AppSettings/ApplicationSettingsFileReader.h \
    ../source/RDP/AppSettings/ApplicationSettingsFileWriter.h \
    ../source/RDP/Models/RGPRecentTraceListModel.h \
    ../source/RDP/Models/ProcessInfoModel.h \
    ../source/Common/Scaling/ScaledGridLayoutWrapper.h \
    ../source/Common/Scaling/ScaledLayoutWrapper.h \
    ../source/Common/Scaling/ScaledObjectWrapper.h \
    ../source/Common/Scaling/ScaledWidgetWrapper.h \
    ../source/Common/Scaling/ScalingManager.h \
    ../source/RDP/Views/ConnectionStatusView.h \
    ../source/RDP/Util/RDPUtil.h \
    ../source/DevDriverComponents/message/inc/escape.h \
    ../source/DevDriverComponents/message/inc/message.h \
    ../source/RDP/Views/CaptureProgressWidget.h \
    ../source/RDP/Views/ClocksView.h \
    ../source/RDP/Views/ActiveApplicationsTableView.h \
    ../source/RDP/Models/ActiveApplicationTableModel.h \
    ../source/RDP/Views/NotificationWidget.h \
    ../source/RDP/Views/ClockModeWidget.h \
    ../source/Common/Util/SingleApplicationInstance.h \
    ../source/Common/Util/SystemKeyboardHook.h \
    ../source/RDP/Models/ConnectionAttemptWorker.h \
    ../source/RDP/Models/ConnectionStatusWorker.h \
    ../source/RDP/Models/RGPClientProcessorThread.h \
    ../source/Common/Util/LogFileWriter.h \
    ../source/RDP/Models/ClocksTabModel.h \
    ../source/RDP/Models/DeviceClockModeModel.h \
    ../source/RDP/Views/RDPLogBox.h \
    ../../QtCommon/Util/QtUtil.h

FORMS += \
    ../source/RDP/Views/MainWindow.ui \
    ../source/RDP/Views/ConnectionSettingsView.ui \
    ../source/RDP/Views/DriverSettingsView.ui \
    ../source/RDP/Views/EmptyDriverSettingsView.ui \
    ../source/RDP/Views/RecentConnectionsView.ui \
    ../source/RDP/Views/NewConnectionView.ui \
    ../source/RDP/Views/SetupTargetApplicationView.ui \
    ../source/RDP/Views/RGPTraceView.ui \
    ../source/RDP/Views/LogView.ui \
    ../source/Common/Views/DebugWindow.ui \
    ../source/RDP/Views/DriverLoggerView.ui \
    ../source/RDP/Views/ConnectionStatusView.ui \
    ../source/RDP/Views/CaptureProgressWidget.ui \
    ../source/RDP/Views/ClocksView.ui \
    ../source/RDP/Views/ActiveApplicationsTableView.ui \
    ../source/RDP/Views/NotificationWidget.ui \
    ../source/RDP/Views/ClockModeWidget.ui

RESOURCES += \
    ../source/RDP/Panel.qrc

INCLUDEPATH += ../source/DevDriverComponents/inc

win32:RC_ICONS += ../images/RDP_Icon.ico

Release:DESTDIR = release
Release:OBJECTS_DIR = release/.obj
Release:MOC_DIR = release/.moc
Release:RCC_DIR = release/.rcc
Release:UI_DIR = release/.ui

Debug:DESTDIR = debug
Debug:OBJECTS_DIR = debug/.obj
Debug:MOC_DIR = debug/.moc
Debug:RCC_DIR = debug/.rcc
Debug:UI_DIR = debug/.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../source/DevDriverComponents/message/lib/Release/x64/ -lmessage -luser32
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../source/DevDriverComponents/message/lib/Debug/x64/ -lmessage -luser32

DISTFILES +=

unix:!macx: LIBS += -lrt-2.23
