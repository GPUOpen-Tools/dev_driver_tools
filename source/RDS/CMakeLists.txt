cmake_minimum_required (VERSION 2.6)

# switch on the autogenerators needed for Qt. Includes the MetaObject Compiler (MOC),
# the UI compiler (UIC) and the resource compiler (RCC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(POLICY CMP0020)
  cmake_policy(SET CMP0020 OLD)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

if(WIN32)
    SET(QT_USE_QTMAIN TRUE)
endif(WIN32)

IF(WIN32)
  add_compile_options(/W4 /wd4127 /wd4201 /MP -DGPUOPEN_CLIENT_INTERFACE_MAJOR_VERSION=27 -DDD_PRINT_FUNC=LogMessage -DGPUOPEN_UWP_SUPPORT=1)
ELSEIF(UNIX)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
  add_compile_options(-std=c++11 -D_LINUX -Wall -Wextra -Wmissing-field-initializers -Wno-unused-variable -DGPUOPEN_CLIENT_INTERFACE_MAJOR_VERSION=27 -DDD_PRINT_FUNC=LogMessage)
ENDIF(WIN32)

set (DEVDRIVERSOURCES
 "../DevDriverComponents/listener/ddListenerURIService.cpp"
 "../DevDriverComponents/listener/hostMsgTransport.h"
 "../DevDriverComponents/listener/hostMsgTransport.cpp"
 "../DevDriverComponents/listener/listenerServer.h"
 "../DevDriverComponents/listener/listenerServer.cpp"
 "../DevDriverComponents/listener/transports/hostTransport.h"
 "../DevDriverComponents/listener/transports/hostTransport.cpp"
 "../DevDriverComponents/listener/routerCore.h"
 "../DevDriverComponents/listener/routerCore.cpp"
 "../DevDriverComponents/listener/listenerCore.h"
 "../DevDriverComponents/listener/listenerCore.cpp"
 "../DevDriverComponents/listener/transportThread.h"
 "../DevDriverComponents/listener/transportThread.cpp"
 "../DevDriverComponents/listener/transports/abstractListenerTransport.h"
 "../DevDriverComponents/listener/transports/socketTransport.h"
 "../DevDriverComponents/listener/transports/socketTransport.cpp"
 "../DevDriverComponents/listener/clientmanagers/abstractClientManager.h"
 "../DevDriverComponents/listener/clientmanagers/listenerClientManager.h"
 "../DevDriverComponents/listener/clientmanagers/listenerClientManager.cpp"
 "../DevDriverComponents/src/baseProtocolClient.cpp"
 "../DevDriverComponents/src/baseProtocolServer.cpp"
 "../DevDriverComponents/src/ddClientURIService.cpp"
 "../DevDriverComponents/src/ddTransferManager.cpp"
 "../DevDriverComponents/src/devDriverServer.cpp"
 "../DevDriverComponents/src/messageChannel.h"
 "../DevDriverComponents/src/messageChannel.inl"
 "../DevDriverComponents/src/session.cpp"
 "../DevDriverComponents/src/session.h"
 "../DevDriverComponents/src/sessionManager.cpp"
 "../DevDriverComponents/src/sessionManager.h"
 "../DevDriverComponents/src/socket.h"
 "../DevDriverComponents/src/socketMsgTransport.cpp"
 "../DevDriverComponents/src/socketMsgTransport.h"
 "../DevDriverComponents/src/protocols/ddGpuCrashDumpClient.cpp"
 "../DevDriverComponents/src/protocols/ddGpuCrashDumpServer.cpp"
 "../DevDriverComponents/src/protocols/ddTransferClient.cpp"
 "../DevDriverComponents/src/protocols/ddTransferServer.cpp"
 "../DevDriverComponents/src/protocols/ddURIServer.cpp"
 "../DevDriverComponents/src/protocols/driverControlServer.cpp"
 "../DevDriverComponents/src/protocols/loggingServer.cpp"
 "../DevDriverComponents/src/protocols/rgpServer.cpp"
 "../DevDriverComponents/src/protocols/settingsServer.cpp"
)

set (DEVDRIVERMESSAGELIB
 ${CMAKE_SOURCE_DIR}/source/DevDriverComponents/message/lib/$(Configuration)/$(Platform)/message.lib
)

set( WINDOWS_DD_UWP_SOURCES
 "../DevDriverComponents/listener/clientmanagers/uwpClientManager.h"
 "../DevDriverComponents/listener/clientmanagers/uwpClientManager.cpp"
)

set( WINDOWS_DD_SOURCES
 "../DevDriverComponents/listener/transports/winPipeTransport.h"
 "../DevDriverComponents/listener/transports/winPipeTransport.cpp"
 "../DevDriverComponents/src/win/d3d12EtwDxgkParser.h"
 "../DevDriverComponents/src/win/d3d12EtwEventParser.h"
 "../DevDriverComponents/src/win/ddWinEtwServerSession.h"
 "../DevDriverComponents/src/win/ddWinPipeMsgTransport.cpp"
 "../DevDriverComponents/src/win/ddWinPipeMsgTransport.h"
 "../DevDriverComponents/src/win/ddWinPlatform.cpp"
 "../DevDriverComponents/src/win/ddWinSocket.cpp"
 "../DevDriverComponents/src/win/traceSession.cpp"
 "../DevDriverComponents/src/protocols/etwServer.cpp"
)

set( WINDOWS_SOURCES
 "../../images/RDS_Icon.ico"
 "../../images/RDS_Icon.png"
 "Windows/RDS.rc"
 "Windows/resource.h"
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
 "../Common/Version.h"
 "../Common/DriverToolsDefinitions.h"
 "../Common/RestoreCursorPosition.h"
 "../Common/ToolUtil.cpp"
 "../Common/ToolUtil.h"
 "../Common/ddMemAlloc.cpp"
 "../Common/ddMemAlloc.h"
 "../Common/ModelViewMapper.h"
 "../Common/ModelViewMapper.cpp"
 "../Common/Views/DebugWindow.h"
 "../Common/Views/DebugWindow.cpp"
 "../Common/Util/SingleApplicationInstance.h"
 "../Common/Util/SingleApplicationInstance.cpp"
 "../Common/NamedMutex.h"
 "../Common/NamedMutex.cpp"
 "../Common/SingleInstance.h"
 "Models/ConfigurationWindowModel.h"
 "Models/ConfigurationWindowModel.cpp"
 "Settings/RDSSettings.h"
 "Settings/RDSSettings.cpp"
 "Settings/RDSSettingsReader.h"
 "Settings/RDSSettingsReader.cpp"
 "Settings/RDSSettingsWriter.h"
 "Settings/RDSSettingsWriter.cpp"
 "Views/ConfigurationWindow.cpp"
 "Views/ConfigurationWindow.h"
 "Views/ConfigurationWindow.ui"
 "Views/MainWindow.cpp"
 "Views/MainWindow.h"
 "Views/MainWindow.ui"
 "Service.qrc"
 "ServiceMain.cpp"
 "RDSDefinitions.h"
)

set ( HEADLESS_SOURCES
 "../Common/Version.h"
 "../Common/DriverToolsDefinitions.h"
 "../Common/CommandlineParser.cpp"
 "../Common/CommandlineParser.h"
 "../Common/CommandlineParameter.cpp"
 "../Common/CommandlineParameter.h"
 "../Common/ddMemAlloc.cpp"
 "../Common/ddMemAlloc.h"
 "../Common/NamedMutex.h"
 "../Common/NamedMutex.cpp"
 "../Common/SingleInstance.h"
 "HeadlessMain.cpp"
 "RDSDefinitions.h"
)

IF(INTERNAL_BUILD)
 set( EXECUTABLE RadeonDeveloperService-Internal )
 set( HEADLESS_EXECUTABLE RadeonDeveloperServiceCLI-Internal )
ELSE(INTERNAL_BUILD)
 set( EXECUTABLE RadeonDeveloperService )
 set( HEADLESS_EXECUTABLE RadeonDeveloperServiceCLI )
ENDIF(INTERNAL_BUILD)

# Specify output executable name. For windows, indicate this is a Windows application
# and not a console application
IF(WIN32)
 IF(UWP)
  add_executable(${EXECUTABLE} WIN32 ${SOURCES} ${DEVDRIVERSOURCES} ${WINDOWS_DD_SOURCES} ${WINDOWS_DD_UWP_SOURCES} ${WINDOWS_SOURCES})
  add_executable(${HEADLESS_EXECUTABLE} ${HEADLESS_SOURCES} ${DEVDRIVERSOURCES} ${WINDOWS_DD_SOURCES} ${WINDOWS_DD_UWP_SOURCES} ${WINDOWS_SOURCES})
 ELSE(UWP)
  add_executable(${EXECUTABLE} WIN32 ${SOURCES} ${DEVDRIVERSOURCES} ${WINDOWS_DD_SOURCES} ${WINDOWS_SOURCES})
  add_executable(${HEADLESS_EXECUTABLE} ${HEADLESS_SOURCES} ${DEVDRIVERSOURCES} ${WINDOWS_DD_SOURCES} ${WINDOWS_SOURCES})
 ENDIF(UWP)
 set_target_properties(${EXECUTABLE} PROPERTIES COMPILE_FLAGS "-DRDS_LOG_TO_PANEL")
 set_target_properties(${HEADLESS_EXECUTABLE} PROPERTIES COMPILE_FLAGS "-DHEADLESS -DDEVDRIVER_PRINT_TO_CONSOLE -DDEVDRIVER_LOG_LEVEL=LogLevel::Debug")
ELSEIF(APPLE)
add_executable(${EXECUTABLE} MACOSX_BUNDLE ${SOURCES} ${DEVDRIVERSOURCES} ${LINUX_DD_SOURCES} ${LINUX_SOURCES})
add_executable(${HEADLESS_EXECUTABLE} MACOSX_BUNDLE ${HEADLESS_SOURCES} ${DEVDRIVERSOURCES} ${LINUX_DD_SOURCES} ${LINUX_SOURCES})
set_target_properties(${EXECUTABLE} PROPERTIES COMPILE_FLAGS "-DRDS_LOG_TO_PANEL")
set_target_properties(${HEADLESS_EXECUTABLE} PROPERTIES COMPILE_FLAGS "-DHEADLESS -DDEVDRIVER_PRINT_TO_CONSOLE -DDEVDRIVER_LOG_LEVEL=LogLevel::Debug")
ELSEIF(UNIX)
add_executable(${EXECUTABLE} ${SOURCES} ${DEVDRIVERSOURCES} ${LINUX_DD_SOURCES} ${LINUX_SOURCES})
add_executable(${HEADLESS_EXECUTABLE} ${HEADLESS_SOURCES} ${DEVDRIVERSOURCES} ${LINUX_DD_SOURCES} ${LINUX_SOURCES})
set_target_properties(${EXECUTABLE} PROPERTIES COMPILE_FLAGS "-DRDS_LOG_TO_PANEL")
set_target_properties(${HEADLESS_EXECUTABLE} PROPERTIES COMPILE_FLAGS "-DHEADLESS -DDEVDRIVER_PRINT_TO_CONSOLE -DDEVDRIVER_LOG_LEVEL=LogLevel::Debug")
ENDIF(WIN32)

# Executable file library dependency list.
IF(WIN32)
 IF(UWP)
  target_link_libraries(${EXECUTABLE} Qt5::Core Qt5::Widgets Qt5::WinMain QtScaling ${DEVDRIVERMESSAGELIB} tdh.lib)
  target_link_libraries(${HEADLESS_EXECUTABLE} ${DEVDRIVERMESSAGELIB} tdh.lib)
 ELSE(UWP)
  target_link_libraries(${EXECUTABLE} Qt5::Core Qt5::Widgets Qt5::WinMain QtScaling tdh.lib)
  target_link_libraries(${HEADLESS_EXECUTABLE} tdh.lib)
 ENDIF(UWP)
ELSEIF(UNIX)
target_link_libraries(${EXECUTABLE} Qt5::Core Qt5::Widgets QtScaling pthread)
target_link_libraries(${HEADLESS_EXECUTABLE} pthread)
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
