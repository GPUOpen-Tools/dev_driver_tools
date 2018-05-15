cmake_minimum_required (VERSION 2.6)

# switch on the autogenerators needed for Qt. Includes the MetaObject Compiler (MOC),
# the UI compiler (UIC) and the resource compiler (RCC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# policy to not link executables to qtmain.lib on windows
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 OLD)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(AFTER)

include_directories(../../../Common/Lib/AMD/ADL/include)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

if(WIN32)
    SET(QT_USE_QTMAIN TRUE)
endif(WIN32)

IF(WIN32)
  add_compile_options(/W4 /wd4127 /wd4201 /wd4714 /MP -DGPUOPEN_CLIENT_INTERFACE_MAJOR_VERSION=30)
ELSEIF(UNIX)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
  add_compile_options(-std=c++11 -D_LINUX -Wall -Wextra -Wmissing-field-initializers -Wno-unused-variable -DGPUOPEN_CLIENT_INTERFACE_MAJOR_VERSION=30)
ENDIF(WIN32)

set ( DEVDRIVERSOURCES
 "../DevDriverComponents/src/baseProtocolClient.cpp"
 "../DevDriverComponents/src/baseProtocolServer.cpp"
 "../DevDriverComponents/src/ddClientURIService.cpp"
 "../DevDriverComponents/src/ddSocket.h"
 "../DevDriverComponents/src/ddTransferManager.cpp"
 "../DevDriverComponents/src/devDriverClient.cpp"
 "../DevDriverComponents/src/messageChannel.h"
 "../DevDriverComponents/src/messageChannel.inl"
 "../DevDriverComponents/src/session.h"
 "../DevDriverComponents/src/session.cpp"
 "../DevDriverComponents/src/sessionManager.h"
 "../DevDriverComponents/src/sessionManager.cpp"
 "../DevDriverComponents/src/socketMsgTransport.h"
 "../DevDriverComponents/src/socketMsgTransport.cpp"
 "../DevDriverComponents/src/protocols/ddGpuCrashDumpClient.cpp"
 "../DevDriverComponents/src/protocols/ddGpuCrashDumpServer.cpp"
 "../DevDriverComponents/src/protocols/ddTransferClient.cpp"
 "../DevDriverComponents/src/protocols/ddTransferServer.cpp"
 "../DevDriverComponents/src/protocols/ddURIClient.cpp"
 "../DevDriverComponents/src/protocols/ddURIServer.cpp"
 "../DevDriverComponents/src/protocols/driverControlClient.cpp"
 "../DevDriverComponents/src/protocols/loggingClient.cpp"
 "../DevDriverComponents/src/protocols/rgpClient.cpp"
 "../DevDriverComponents/src/protocols/settingsClient.cpp"
)

set (DEVDRIVERMESSAGELIB
 ${CMAKE_SOURCE_DIR}/source/DevDriverComponents/message/lib/$(Configuration)/$(Platform)/message.lib
)

set( WINDOWS_DD_SOURCES
 "../DevDriverComponents/src/protocols/etwClient.cpp"
 "../DevDriverComponents/src/win/ddWinPipeMsgTransport.h"
 "../DevDriverComponents/src/win/ddWinPipeMsgTransport.cpp"
 "../DevDriverComponents/src/win/ddWinPlatform.cpp"
 "../DevDriverComponents/src/win/ddWinSocket.cpp"
)

set( WINDOWS_SOURCES
 "../../images/Computer.png"
 "../../images/RDP_Icon.ico"
 "../../images/RDP_Icon.png"
 "../../images/X.png"
 "Windows/RDP.rc"
 "Windows/resource.h"
 "../DevDriverAPI/ADLGetDriverVersion.cpp"
 "../DevDriverAPI/ADLGetDriverVersion.h"
)

set( LINUX_DD_SOURCES
  "../DevDriverComponents/src/posix/ddPosixPlatform.cpp"
  "../DevDriverComponents/src/posix/ddPosixSocket.cpp"
)

set( LINUX_SOURCES
 "../Common/Linux/SignalHandler.cpp"
 "../Common/Linux/SignalHandler.h"
)

set ( SOURCES
 "../Common/Util/SingleApplicationInstance.cpp"
 "../Common/Util/SingleApplicationInstance.h"
 "../Common/SingleInstance.h"
 "../Common/NamedMutex.cpp"
 "../Common/NamedMutex.h"
 "../Common/ddMemAlloc.cpp"
 "../Common/ddMemAlloc.h"
 "../Common/Util/SystemKeyboardHook.cpp"
 "../Common/Util/SystemKeyboardHook.h"
 "../Common/Util/LogFileWriter.cpp"
 "../Common/Util/LogFileWriter.h"
 "../Common/Version.h"
 "../Common/DriverToolsDefinitions.h"
 "../Common/RestoreCursorPosition.h"
 "../Common/ToolUtil.cpp"
 "../Common/ToolUtil.h"
 "../Common/Views/DebugWindow.cpp"
 "../Common/Views/DebugWindow.h"
 "../Common/Views/DebugWindow.ui"
 "../Common/ModelViewMapper.cpp"
 "../Common/ModelViewMapper.h"
 "../Common/Process.cpp"
 "../Common/Process.h"
 "../Common/ToolbarTabWidget.h"
 "../Common/ToolbarTabWidget.cpp"
 "Models/ActiveApplicationTableModel.cpp"
 "Models/ActiveApplicationTableModel.h"
 "Models/ApplicationSettingsModel.cpp"
 "Models/ApplicationSettingsModel.h"
 "Models/ConnectionAttemptWorker.cpp"
 "Models/ConnectionAttemptWorker.h"
 "Models/ConnectionSettingsModel.cpp"
 "Models/ConnectionSettingsModel.h"
 "Models/ConnectionStatusWorker.cpp"
 "Models/ConnectionStatusWorker.h"
 "Models/DeveloperPanelModel.cpp"
 "Models/DeveloperPanelModel.h"
 "Models/ClocksTabModel.cpp"
 "Models/ClocksTabModel.h"
 "Models/DeviceClockModeModel.cpp"
 "Models/DeviceClockModeModel.h"
 "Models/DriverLogBackgroundWorker.cpp"
 "Models/DriverLogBackgroundWorker.h"
 "Models/DriverMessageProcessorThread.cpp"
 "Models/DriverMessageProcessorThread.h"
 "Models/RGPClientProcessorThread.cpp"
 "Models/RGPClientProcessorThread.h"
 "Models/DriverLogfileModel.cpp"
 "Models/DriverLogfileModel.h"
 "Models/DriverLoggingModel.cpp"
 "Models/DriverLoggingModel.h"
 "Models/DriverProtocolModel.cpp"
 "Models/DriverProtocolModel.h"
 "Models/DriverSettingsModel.cpp"
 "Models/DriverSettingsModel.h"
 "Models/ProcessInfoModel.cpp"
 "Models/ProcessInfoModel.h"
 "Models/RecentConnectionsModel.cpp"
 "Models/RecentConnectionsModel.h"
 "Models/SetupTargetApplicationModel.cpp"
 "Models/SetupTargetApplicationModel.h"
 "Models/RGPTraceModel.cpp"
 "Models/RGPTraceModel.h"
 "Models/RGPRecentTraceListModel.cpp"
 "Models/RGPRecentTraceListModel.h"
 "AppSettings/ApplicationSettingsFile.cpp"
 "AppSettings/ApplicationSettingsFile.h"
 "AppSettings/ApplicationSettingsFileReader.cpp"
 "AppSettings/ApplicationSettingsFileReader.h"
 "AppSettings/ApplicationSettingsFileWriter.cpp"
 "AppSettings/ApplicationSettingsFileWriter.h"
 "Settings/RDPSettingsWriter.cpp"
 "Settings/RDPSettingsWriter.h"
 "Settings/RDPSettingsReader.cpp"
 "Settings/RDPSettingsReader.h"
 "Settings/RDPSettings.cpp"
 "Settings/RDPSettings.h"
 "Util/RDPUtil.cpp"
 "Util/RDPUtil.h"
 "Views/ActiveApplicationsTableView.cpp"
 "Views/ActiveApplicationsTableView.h"
 "Views/ActiveApplicationsTableView.ui"
 "Views/AppListTreeView.cpp"
 "Views/AppListTreeView.h"
 "Views/CaptureProgressWidget.cpp"
 "Views/CaptureProgressWidget.h"
 "Views/CaptureProgressWidget.ui"
 "Views/ClockModeWidget.cpp"
 "Views/ClockModeWidget.h"
 "Views/ClockModeWidget.ui"
 "Views/ClocksView.cpp"
 "Views/ClocksView.h"
 "Views/ClocksView.ui"
 "Views/ConnectionStatusView.cpp"
 "Views/ConnectionStatusView.h"
 "Views/ConnectionStatusView.ui"
 "Views/ConnectionSettingsView.cpp"
 "Views/ConnectionSettingsView.h"
 "Views/ConnectionSettingsView.ui"
 "Views/DriverLoggerView.cpp"
 "Views/DriverLoggerView.h"
 "Views/DriverLoggerView.ui"
 "Views/DriverSettingItemWidget.cpp"
 "Views/DriverSettingItemWidget.h"
 "Views/DriverSettingsView.cpp"
 "Views/DriverSettingsView.h"
 "Views/DriverSettingsView.ui"
 "Views/EmptyDriverSettingsView.cpp"
 "Views/EmptyDriverSettingsView.h"
 "Views/EmptyDriverSettingsView.ui"
 "Views/MainWindow.cpp"
 "Views/MainWindow.h"
 "Views/MainWindow.ui"
 "Views/NotificationWidget.cpp"
 "Views/NotificationWidget.h"
 "Views/NotificationWidget.ui"
 "Views/RDPLogBox.cpp"
 "Views/RDPLogBox.h"
 "Views/RGPTraceView.cpp"
 "Views/RGPTraceView.h"
 "Views/RGPTraceView.ui"
 "Views/LogView.cpp"
 "Views/LogView.h"
 "Views/LogView.ui"
 "Views/NewConnectionView.cpp"
 "Views/NewConnectionView.h"
 "Views/NewConnectionView.ui"
 "Views/RecentConnectionsView.cpp"
 "Views/RecentConnectionsView.h"
 "Views/RecentConnectionsView.ui"
 "Views/SetupTargetApplicationView.cpp"
 "Views/SetupTargetApplicationView.h"
 "Views/SetupTargetApplicationView.ui"
 "RDPDefinitions.h"
 "Panel.qrc"
 "PanelMain.cpp"
)

IF(INTERNAL_BUILD)
 set( EXECUTABLE RadeonDeveloperPanel-Internal )
ELSE(INTERNAL_BUILD)
 set( EXECUTABLE RadeonDeveloperPanel )
ENDIF(INTERNAL_BUILD)

# Specify output executable name. For windows, indicate this is a Windows application
# and not a console application
IF(WIN32)
add_executable(${EXECUTABLE} WIN32 ${SOURCES} ${DEVDRIVERSOURCES} ${WINDOWS_DD_SOURCES} ${WINDOWS_SOURCES})
ELSEIF(APPLE)
add_executable(${EXECUTABLE} MACOSX_BUNDLE ${SOURCES} ${DEVDRIVERSOURCES} ${LINUX_DD_SOURCES} ${LINUX_SOURCES})
ELSEIF(UNIX)
add_executable(${EXECUTABLE} ${SOURCES} ${DEVDRIVERSOURCES} ${LINUX_DD_SOURCES} ${LINUX_SOURCES})
ENDIF(WIN32)

# executable file library dependency list
IF(WIN32)
 IF(UWP)
  target_link_libraries(${EXECUTABLE} Qt5::Widgets Qt5::WinMain QtScaling QtUtil ${DEVDRIVERMESSAGELIB})
 ELSE(UWP)
  target_link_libraries(${EXECUTABLE} Qt5::Widgets Qt5::WinMain QtScaling QtUtil)
 ENDIF(UWP)
ELSEIF(UNIX)
target_link_libraries(${EXECUTABLE} Qt5::Widgets QtScaling QtUtil pthread rt)
ENDIF()

IF(WIN32)
# Create Visual Studio filters so that the source files in the project match the
# directory structure
foreach(source IN LISTS SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()
ENDIF(WIN32)
