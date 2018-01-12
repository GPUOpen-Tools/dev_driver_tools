#-------------------------------------------------
#
# Project created by QtCreator 2016-08-26T03:53:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RadeonDeveloperService
TEMPLATE = app

# This is to solve "cannot convert parameter 1 from 'char *' to 'LPCWSTR'" problem when compiling the back-end
DEFINES -= UNICODE
DEFINES += _MBCS
DEFINES += GPUOPEN_CLIENT_INTERFACE_MAJOR_VERSION=26
DEFINES += DEVDRIVER_UM_BUILD=0

SOURCES += \
    ../source/RDS/ServiceMain.cpp \
    ../source/RDS/Views/MainWindow.cpp \
    ../source/DevDriverComponents/listener/clientmanagers/listenerClientManager.cpp \
    ../source/DevDriverComponents/listener/ddListenerURIService.cpp \
    ../source/DevDriverComponents/listener/listenerCore.cpp \
    ../source/DevDriverComponents/listener/routerCore.cpp \
    ../source/DevDriverComponents/listener/transportThread.cpp \
    ../source/DevDriverComponents/listener/transports/socketTransport.cpp \
    ../source/Common/ModelViewMapper.cpp \
    ../source/Common/Views/DebugWindow.cpp \
    ../source/Common/ToolUtil.cpp \
    ../source/Common/Util/SingleApplicationInstance.cpp \
    ../source/DevDriverComponents/listener/hostMsgTransport.cpp \
    ../source/DevDriverComponents/listener/listenerServer.cpp \
    ../source/DevDriverComponents/listener/transports/hostTransport.cpp \
    ../source/DevDriverComponents/src/baseProtocolClient.cpp \
    ../source/DevDriverComponents/src/ddTransferManager.cpp \
    ../source/DevDriverComponents/src/sessionManager.cpp \
    ../source/DevDriverComponents/src/ddClientURIService.cpp \
    ../source/DevDriverComponents/src/protocols/loggingServer.cpp \
    ../source/DevDriverComponents/src/protocols/ddURIServer.cpp \
    ../source/DevDriverComponents/src/protocols/ddTransferClient.cpp \
    ../source/DevDriverComponents/src/protocols/ddTransferServer.cpp \
    ../source/DevDriverComponents/src/session.cpp \
    ../source/DevDriverComponents/src/baseProtocolServer.cpp \
    ../source/DevDriverComponents/src/protocols/etwServer.cpp \
    ../source/DevDriverComponents/src/win/traceSession.cpp \
    ../source/RDS/Views/ConfigurationWindow.cpp \
    ../source/RDS/Models/ConfigurationWindowModel.cpp \
    ../source/Common/Scaling/ScaledGridLayoutWrapper.cpp \
    ../source/Common/Scaling/ScaledLayoutWrapper.cpp \
    ../source/Common/Scaling/ScaledWidgetWrapper.cpp \
    ../source/Common/Scaling/ScalingManager.cpp \
    ../source/RDS/Settings/RDSSettings.cpp \
    ../source/RDS/Settings/RDSSettingsReader.cpp \
    ../source/RDS/Settings/RDSSettingsWriter.cpp

# UNIX-ONLY SOURCES
unix {
SOURCES += \
    ../source/DevDriverComponents/src/lnx/ddLnxPlatform.cpp \
    ../source/DevDriverComponents/src/lnx/ddLnxSocket.cpp \
}

win32 {
SOURCES += \
    ../source/DevDriverComponents/listener/transports/winPipeTransport.cpp \
    ../source/DevDriverComponents/listener/clientmanagers/uwpClientManager.cpp \
    ../source/DevDriverComponents/src/win/ddWinPlatform.cpp \
    ../source/DevDriverComponents/src/win/ddWinSocket.cpp
}

HEADERS += \
    ../source/Common/ModelViewMapper.h \
    ../source/Common/ToolUtil.h \
    ../source/Common/Util/SingleApplicationInstance.h \
    ../source/Common/Views/DebugWindow.h \
    ../source/RDS/Views/MainWindow.h \
    ../source/RDS/RDSDefinitions.h \
    ../source/DevDriverComponents/inc/msgChannel.h \
    ../source/DevDriverComponents/inc/msgTransport.h \
    ../source/DevDriverComponents/inc/platform.h \
    ../source/DevDriverComponents/inc/ddTransferManager.h \
    ../source/DevDriverComponents/listener/ddListenerURIService.h \
    ../source/DevDriverComponents/listener/clientmanagers/abstractClientManager.h \
    ../source/DevDriverComponents/listener/clientmanagers/listenerClientManager.h \
    ../source/DevDriverComponents/listener/listenerCore.h \
    ../source/DevDriverComponents/listener/routerCore.h \
    ../source/DevDriverComponents/listener/transportThread.h \
    ../source/DevDriverComponents/src/socket.h \
    ../source/DevDriverComponents/listener/hostMsgTransport.h \
    ../source/DevDriverComponents/listener/listenerServer.h \
    ../source/DevDriverComponents/listener/transports/hostTransport.h \
    ../source/DevDriverComponents/src/sessionManager.h \
    ../source/DevDriverComponents/src/ddClientURIService.h \
    ../source/DevDriverComponents/inc/protocols/loggingServer.h \
    ../source/DevDriverComponents/src/session.h \
    ../source/DevDriverComponents/inc/baseProtocolServer.h \
    ../source/DevDriverComponents/inc/protocols/etwServer.h \
    ../source/DevDriverComponents/inc/protocols/ddTransferServer.h \
    ../source/DevDriverComponents/inc/protocols/ddTransferClient.h \
    ../source/DevDriverComponents/src/protocols/ddURIServer.h \
    ../source/DevDriverComponents/src/win/traceSession.h \
    ../source/RDS/Views/ConfigurationWindow.h \
    ../source/RDS/Models/ConfigurationWindowModel.h \
    ../source/Common/Scaling/ScaledGridLayoutWrapper.h \
    ../source/Common/Scaling/ScaledLayoutWrapper.h \
    ../source/Common/Scaling/ScaledObjectWrapper.h \
    ../source/Common/Scaling/ScaledWidgetWrapper.h \
    ../source/Common/Scaling/ScalingManager.h \
    ../source/RDS/Settings/RDSSettings.h \
    ../source/RDS/Settings/RDSSettingsReader.h \
    ../source/RDS/Settings/RDSSettingsWriter.h

# UNIX-ONLY HEADER
unix {
HEADERS +=
}

win32 {
HEADERS += \
    ../source/DevDriverComponents/listener/clientmanagers/uwpClientManager.h
}

FORMS += \
    ../source/RDS/Views/MainWindow.ui \
    ../source/Common/Views/DebugWindow.ui \
    ../source/RDS/Views/ConfigurationWindow.ui

RESOURCES += \
    ../source/RDS/Service.qrc

INCLUDEPATH += ../source/DevDriverComponents/inc \
    ../source/DevDriverComponents/message/inc
    ../source/DevDriverComponents/listener

win32:RC_ICONS += ../images/RDS_Icon.ico

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

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../source/DevDriverComponents/message/lib/Release/x64/ -lmessage -luser32 -ladvapi32 -lole32 -ltdh
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../source/DevDriverComponents/message/lib/Debug/x64/ -lmessage -luser32 -ladvapi32 -lole32 -ltdh
