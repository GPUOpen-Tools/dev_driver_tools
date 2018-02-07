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
include_directories(AFTER)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

if(WIN32)
    SET(QT_USE_QTMAIN TRUE)
endif(WIN32)

IF(WIN32)
  add_compile_options(/W4 /wd4127 /wd4201 /wd4714 /MP -DBUILD_DLL -DGPUOPEN_CLIENT_INTERFACE_MAJOR_VERSION=27)
ELSEIF(UNIX)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
  add_compile_options(-std=c++11 -D_LINUX -Wall -Wextra -Wmissing-field-initializers -Wno-unused-variable -DGPUOPEN_CLIENT_INTERFACE_MAJOR_VERSION=27)
ENDIF(WIN32)

set ( DEVDRIVERSOURCES
 "../DevDriverComponents/listener/ddListenerURIService.cpp"
 "../DevDriverComponents/listener/ddListenerURIService.h"
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
 "../DevDriverComponents/src/ddClientURIService.h"
 "../DevDriverComponents/src/ddTransferManager.cpp"
 "../DevDriverComponents/src/devDriverClient.cpp"
 "../DevDriverComponents/src/devDriverServer.cpp"
 "../DevDriverComponents/src/messageChannel.h"
 "../DevDriverComponents/src/messageChannel.inl"
 "../DevDriverComponents/src/session.h"
 "../DevDriverComponents/src/session.cpp"
 "../DevDriverComponents/src/sessionManager.h"
 "../DevDriverComponents/src/sessionManager.cpp"
 "../DevDriverComponents/src/socket.h"
 "../DevDriverComponents/src/socketMsgTransport.h"
 "../DevDriverComponents/src/socketMsgTransport.cpp"
 "../DevDriverComponents/src/protocols/ddGpuCrashDumpClient.cpp"
 "../DevDriverComponents/src/protocols/ddGpuCrashDumpServer.cpp"
 "../DevDriverComponents/src/protocols/ddTransferClient.cpp"
 "../DevDriverComponents/src/protocols/ddTransferServer.cpp"
 "../DevDriverComponents/src/protocols/ddURIClient.cpp"
 "../DevDriverComponents/src/protocols/ddURIServer.cpp"
 "../DevDriverComponents/src/protocols/ddURIServer.h"
 "../DevDriverComponents/src/protocols/driverControlClient.cpp"
 "../DevDriverComponents/src/protocols/driverControlServer.cpp"
 "../DevDriverComponents/src/protocols/loggingClient.cpp"
 "../DevDriverComponents/src/protocols/loggingServer.cpp"
 "../DevDriverComponents/src/protocols/rgpClient.cpp"
 "../DevDriverComponents/src/protocols/rgpServer.cpp"
 "../DevDriverComponents/src/protocols/settingsClient.cpp"
 "../DevDriverComponents/src/protocols/settingsServer.cpp"
)

set (DEVDRIVERMESSAGELIB
 ${CMAKE_SOURCE_DIR}/source/DevDriverComponents/message/lib/$(Configuration)/$(Platform)/message.lib
)

set( WINDOWS_DD_SOURCES
 "../DevDriverComponents/listener/transports/winPipeTransport.h"
 "../DevDriverComponents/listener/transports/winPipeTransport.cpp"
 "../DevDriverComponents/src/win/ddWinPipeMsgTransport.h"
 "../DevDriverComponents/src/win/ddWinPipeMsgTransport.cpp"
 "../DevDriverComponents/src/win/ddWinPlatform.cpp"
 "../DevDriverComponents/src/win/ddWinSocket.cpp"
 "../DevDriverComponents/src/win/traceSession.cpp"
 "../DevDriverComponents/src/win/traceSession.h"
 "../DevDriverComponents/src/win/d3d12EtwDxgkParser.h"
 "../DevDriverComponents/src/win/d3d12EtwEventParser.h"
 "../DevDriverComponents/src/win/ddWinEtwServerSession.h"
 "../DevDriverComponents/src/protocols/etwClient.cpp"
 "../DevDriverComponents/src/protocols/etwServer.cpp"
)

set( LINUX_DD_SOURCES
  "../DevDriverComponents/src/posix/ddPosixPlatform.cpp"
  "../DevDriverComponents/src/posix/ddPosixSocket.cpp"
)

set ( SOURCES
 "RGPClientInProcessModel.cpp"
 "RGPClientInProcessModel.h"
 "RGP_API.cpp"
 "RGP_API.h"
)

IF(INTERNAL_BUILD)
 set( EXECUTABLE RGP_API-Internal )
ELSE(INTERNAL_BUILD)
 set( EXECUTABLE RGP_API )
ENDIF(INTERNAL_BUILD)

# Specify output executable name. For windows, indicate this is a Windows application
# and not a console application
IF(WIN32)
 add_library(${EXECUTABLE} SHARED ${SOURCES} ${DEVDRIVERSOURCES} ${WINDOWS_DD_SOURCES})
 include(GenerateExportHeader)
 generate_export_header(${EXECUTABLE})
 add_custom_command(TARGET ${EXECUTABLE}
                    POST_BUILD
                    COMMAND $(SolutionDir)../postbuild.bat $(Configuration))
ELSEIF(APPLE)
 add_library(${EXECUTABLE} SHARED ${SOURCES} ${DEVDRIVERSOURCES} ${LINUX_DD_SOURCES})
ELSEIF(UNIX)
 add_library(${EXECUTABLE} SHARED ${SOURCES} ${DEVDRIVERSOURCES} ${LINUX_DD_SOURCES})
ENDIF(WIN32)

# executable file library dependency list
IF(WIN32)
 target_link_libraries(${EXECUTABLE} tdh.lib)
ELSEIF(UNIX)
 target_link_libraries(${EXECUTABLE} pthread rt)
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
