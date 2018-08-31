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
  add_compile_options(/W4 /wd4127 /wd4201 /wd4714 /MP)
ELSEIF(UNIX)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
  add_compile_options(-std=c++11 -D_LINUX -Wall -Wextra -Wmissing-field-initializers -Wno-unused-variable)
ENDIF(WIN32)

set ( DEVDRIVERSOURCES
 "../DevDriverComponents/src/imported/metrohash/src/metrohash64.cpp"
 "../DevDriverComponents/src/baseProtocolClient.cpp"
 "../DevDriverComponents/src/baseProtocolServer.cpp"
 "../DevDriverComponents/src/ddClientURIService.cpp"
 "../DevDriverComponents/src/ddSocket.h"
 "../DevDriverComponents/src/ddTransferManager.cpp"
 "../DevDriverComponents/src/ddURIRequestContext.cpp"
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
 "../DevDriverComponents/src/util/ddTextWriter.cpp"
 "../DevDriverComponents/src/util/ddJsonWriter.cpp"
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
 "../../assets/Computer.png"
 "../../assets/RDP_Icon.ico"
 "../../assets/RDP_Icon.png"
 "../../assets/X.png"
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

set( INTERNAL_SOURCES
 "Models/PipelineBinsModel.cpp"
 "Models/PipelineBinsModel.h"
 "Models/PipelineBinsWorker.cpp"
 "Models/PipelineBinsWorker.h"
 "Views/PipelineBinsView.ui"
 "Views/PipelineBinsView.cpp"
 "Views/PipelineBinsView.h"
 "Views/PipelineBinsOutputWindow.ui"
 "Views/PipelineBinsOutputWindow.cpp"
 "Views/PipelineBinsOutputWindow.h"
 "Models/MultiColumnProxyModel.cpp"
 "Models/MultiColumnProxyModel.h"
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
 "Views/AppListSetupTreeView.cpp"
 "Views/AppListSetupTreeView.h"
 "Views/CaptureProgressWidget.cpp"
 "Views/CaptureProgressWidget.h"
 "Views/CaptureProgressWidget.ui"
 "Views/ClockModeWidget.cpp"
 "Views/ClockModeWidget.h"
 "Views/ClockModeWidget.ui"
 "Views/ClocksView.cpp"
 "Views/ClocksView.h"
 "Views/ClocksView.ui"
 "Views/ConnectionLogView.cpp"
 "Views/ConnectionLogView.h"
 "Views/ConnectionLogView.ui"
 "Views/ConnectionStatusView.cpp"
 "Views/ConnectionStatusView.h"
 "Views/ConnectionStatusView.ui"
 "Views/ConnectionSettingsView.cpp"
 "Views/ConnectionSettingsView.h"
 "Views/ConnectionSettingsView.ui"
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
 "Views/CustomWidgets/ArrowIconComboBox.cpp"
 "Views/CustomWidgets/ArrowIconComboBox.h"
 "RDPDefinitions.h"
 "Panel.qrc"
 "PanelMain.cpp"
)

IF(ENABLE_DRIVER_LOGGING)
set ( DRIVER_LOGGING_SOURCES
 "Models/DDTagListModel.cpp"
 "Models/DDTagListModel.h"
 "Models/DDTagListFilterProxyModel.cpp"
 "Models/DDTagListFilterProxyModel.h"
 "Models/DDTagIDList.h"
 "Models/DriverLogConfigModel.cpp"
 "Models/DriverLogConfigModel.h"
 "Views/DriverLogConfigView.cpp"
 "Views/DriverLogConfigView.h"
 "Views/DriverLogConfigView.ui"
 "Views/DriverLogMessageView.cpp"
 "Views/DriverLogMessageView.h"
 "Views/DriverLogMessageView.ui"
 "Views/DriverLogTabView.cpp"
 "Views/DriverLogTabView.h"
 "Views/DriverLogTabView.ui"
 "Views/CustomWidgets/DDTagListView.cpp"
 "Views/CustomWidgets/DDTagListView.h"
)
ENDIF(ENABLE_DRIVER_LOGGING)

IF(ENABLE_UNIFIED_DRIVER_SETTINGS)
set ( UNIFIED_DRIVER_SETTINGS_SOURCES
 "Models/EnumListDefinition.cpp"
 "Models/EnumListDefinition.h"
 "Models/EnumListValue.cpp"
 "Models/EnumListValue.h"
 "Models/DevDriverSettingsDataFormats.h"
 "Models/DevDriverSettingsFilterProxyModel.cpp"
 "Models/DevDriverSettingsFilterProxyModel.h"
 "Models/DevDriverSettingsModel.cpp"
 "Models/DevDriverSettingsModel.h"
 "Models/DevDriverSettingsModelTreeNode.cpp"
 "Models/DevDriverSettingsModelTreeNode.h"
 "Models/DevDriverSettingsInterface.cpp"
 "Models/DevDriverSettingsInterface.h"
 "Models/DDTagListFilterProxyModel.cpp"
 "Models/DDTagListFilterProxyModel.h"
 "Models/DDTagListModel.cpp"
 "Models/DDTagListModel.h"
 "Models/DDTagIDList.cpp"
 "Models/DDTagIDList.h"
 "Models/MultiBaseValue.cpp"
 "Models/MultiBaseValue.h"
 "Views/CustomWidgets/ScaledLineEdit.cpp"
 "Views/CustomWidgets/ScaledLineEdit.h"
 "Views/CustomWidgets/MultiBaseSpinBox.cpp"
 "Views/CustomWidgets/MultiBaseSpinBox.h"
 "Views/CustomWidgets/DevDriverSettingsTreeView.cpp"
 "Views/CustomWidgets/DevDriverSettingsTreeView.h"
 "Views/CustomWidgets/DDTagListView.cpp"
 "Views/CustomWidgets/DDTagListView.h"
 "Views/Delegates/AppStartOptionComboBoxDelegate.cpp"
 "Views/Delegates/FilePathDelegate.cpp"
 "Views/Delegates/FilePathDelegate.h"
 "Views/Delegates/AppStartOptionComboBoxDelegate.h"
 "Views/Delegates/DevDriverSettingDelegateBase.h"
 "Views/Delegates/DevDriverSettingWidgetDelegate.h"
 "Views/Delegates/DevDriverSettingButtonDelegate.h"
 "Views/Delegates/DevDriverSettingCheckBoxDelegate.h"
 "Views/Delegates/DevDriverSettingComboBoxDelegate.h"
 "Views/Delegates/DevDriverSettingEditBoxDelegate.h"
 "Views/Delegates/DevDriverSettingLabelDelegate.h"
 "Views/Delegates/DevDriverSettingListBoxDelegate.h"
 "Views/Delegates/DevDriverSettingViewDelegate.cpp"
 "Views/Delegates/DevDriverSettingViewDelegate.h"
 "Views/Delegates/DevDriverSettingMultiBaseSpinBoxDelegate.h"
 "Views/Delegates/DevDriverSettingDelegateFactory.cpp"
 "Views/Delegates/DevDriverSettingDelegateFactory.h"
 "Views/DevDriverSettingsPaneView.cpp"
 "Views/DevDriverSettingsPaneView.h"
 "Views/DevDriverSettingsPaneView.ui"
 "Views/DevDriverSettingsTargetAppView.cpp"
 "Views/DevDriverSettingsTargetAppView.h"
 "Views/DevDriverSettingsTargetAppView.ui"
 "Views/FilePathWidget.cpp"
 "Views/FilePathWidget.h"
 "Views/FilePathWidget.ui"
 "Views/TagViewContainer.cpp"
 "Views/TagViewContainer.h"
 "Views/TagViewContainer.ui"
 "Views/TagListViewContextMenu.cpp"
 "Views/TagListViewContextMenu.h"
 "Views/TagManagerDialog.cpp"
 "Views/TagManagerDialog.h"
 "Views/TagManagerDialog.ui"
)
ENDIF(ENABLE_UNIFIED_DRIVER_SETTINGS)

IF(INTERNAL_BUILD)
 set( SOURCES ${SOURCES} ${INTERNAL_SOURCES} )
 set( EXECUTABLE RadeonDeveloperPanel-Internal )
ELSE(INTERNAL_BUILD)
 set( EXECUTABLE RadeonDeveloperPanel )
ENDIF(INTERNAL_BUILD)

# Specify output executable name. For windows, indicate this is a Windows application
# and not a console application
IF(WIN32)
add_executable(${EXECUTABLE} WIN32 ${SOURCES} ${UNIFIED_DRIVER_SETTINGS_SOURCES} ${DRIVER_LOGGING_SOURCES} ${DEVDRIVERSOURCES} ${WINDOWS_DD_SOURCES} ${WINDOWS_SOURCES})
ELSEIF(APPLE)
add_executable(${EXECUTABLE} MACOSX_BUNDLE ${SOURCES} ${UNIFIED_DRIVER_SETTINGS_SOURCES} ${DRIVER_LOGGING_SOURCES} ${DEVDRIVERSOURCES} ${LINUX_DD_SOURCES} ${LINUX_SOURCES})
ELSEIF(UNIX)
add_executable(${EXECUTABLE} ${SOURCES} ${UNIFIED_DRIVER_SETTINGS_SOURCES} ${DRIVER_LOGGING_SOURCES} ${DEVDRIVERSOURCES} ${LINUX_DD_SOURCES} ${LINUX_SOURCES})
ENDIF(WIN32)

# executable file library dependency list
IF(WIN32)
 IF(UWP)
  target_link_libraries(${EXECUTABLE} Qt5::Widgets Qt5::WinMain QtScaling QtUtil QtCustomWidgets ${DEVDRIVERMESSAGELIB})
 ELSE(UWP)
  target_link_libraries(${EXECUTABLE} Qt5::Widgets Qt5::WinMain QtScaling QtUtil QtCustomWidgets)
 ENDIF(UWP)
ELSEIF(APPLE)
 target_link_libraries(${EXECUTABLE} Qt5::Widgets QtScaling QtUtil QtCustomWidgets pthread)
ELSEIF(UNIX)
 target_link_libraries(${EXECUTABLE} Qt5::Widgets QtScaling QtUtil QtCustomWidgets pthread rt)
ENDIF()

IF(WIN32)
# Create Visual Studio filters so that the source files in the project match the
# directory structure
foreach(source IN LISTS SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()

IF(ENABLE_UNIFIED_DRIVER_SETTINGS)
foreach(source IN LISTS UNIFIED_DRIVER_SETTINGS_SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()
ENDIF(ENABLE_UNIFIED_DRIVER_SETTINGS)

IF(ENABLE_DRIVER_LOGGING)
foreach(source IN LISTS DRIVER_LOGGING_SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()
ENDIF(ENABLE_DRIVER_LOGGING)
ENDIF(WIN32)
