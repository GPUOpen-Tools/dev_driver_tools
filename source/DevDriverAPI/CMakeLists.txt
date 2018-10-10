cmake_minimum_required (VERSION 2.6)

if(POLICY CMP0020)
  cmake_policy(SET CMP0020 OLD)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(AFTER)

include_directories(../../../Common/Lib/AMD/ADL/include)

IF(WIN32)
 IF(UWP)
  add_compile_options(/W4 /wd4127 /wd4201 /wd4714 /MP -DBUILD_DLL -DGPUOPEN_UWP_SUPPORT=1)
 ELSE(UWP)
  add_compile_options(/W4 /wd4127 /wd4201 /wd4714 /MP -DBUILD_DLL)
 ENDIF(UWP)
ELSEIF(UNIX)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
  add_compile_options(-std=c++11 -D_LINUX -Wall -Wextra -Wmissing-field-initializers -Wno-unused-variable)
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
 "../DevDriverComponents/src/imported/metrohash/src/metrohash64.cpp"
 "../DevDriverComponents/src/baseProtocolClient.cpp"
 "../DevDriverComponents/src/baseProtocolServer.cpp"
 "../DevDriverComponents/src/ddClientURIService.cpp"
 "../DevDriverComponents/src/ddClientURIService.h"
 "../DevDriverComponents/src/ddSocket.h"
 "../DevDriverComponents/src/ddTransferManager.cpp"
 "../DevDriverComponents/src/ddURIRequestContext.cpp"
 "../DevDriverComponents/src/devDriverClient.cpp"
 "../DevDriverComponents/src/devDriverServer.cpp"
 "../DevDriverComponents/src/messageChannel.h"
 "../DevDriverComponents/src/messageChannel.inl"
 "../DevDriverComponents/src/session.h"
 "../DevDriverComponents/src/session.cpp"
 "../DevDriverComponents/src/sessionManager.h"
 "../DevDriverComponents/src/sessionManager.cpp"
 "../DevDriverComponents/src/socketMsgTransport.h"
 "../DevDriverComponents/src/socketMsgTransport.cpp"
 "../DevDriverComponents/src/protocols/ddSettingsService.cpp"
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
 "../DevDriverComponents/src/util/ddTextWriter.cpp"
 "../DevDriverComponents/src/util/ddJsonWriter.cpp"
 "../DevDriverComponents/inc/util/sharedptr.h"
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
 "DevDriverAPI.cpp"
 "DevDriverAPI.h"
)

set (WINDOWS_SOURCES
 "ADLGetDriverVersion.cpp"
 "ADLGetDriverVersion.h"
 "Windows/DevDriverAPI.rc"
 "Windows/resource.h"
)

# set platform postfix. For Windows 64 bit, use -x64, for Linux 32 bit, use 32
IF(CMAKE_CL_64)
 IF(WIN32)
  set( EXE_PLATFORM_POSTFIX -x64)
 ENDIF(WIN32)
ELSE(CMAKE_CL_64)
 IF(NOT WIN32)
  set( EXE_PLATFORM_POSTFIX 32 )
 ENDIF(NOT WIN32)
ENDIF(CMAKE_CL_64)

# set internal build postfix
IF(INTERNAL_BUILD)
 set( INTERNAL_POSTFIX -Internal )
ELSE(INTERNAL_BUILD)
 set( INTERNAL_POSTFIX )
ENDIF(INTERNAL_BUILD )

set( EXECUTABLE DevDriverAPI${EXE_PLATFORM_POSTFIX}${DEBUG_POSTFIX}${INTERNAL_POSTFIX} )

# Specify output executable name. For windows, indicate this is a Windows application
# and not a console application
IF(WIN32)
 IF(UWP)
  add_library(${EXECUTABLE} SHARED ${SOURCES} ${WINDOWS_SOURCES} ${DEVDRIVERSOURCES} ${WINDOWS_DD_SOURCES} ${WINDOWS_DD_UWP_SOURCE})
 ELSE(UWP)
  add_library(${EXECUTABLE} SHARED ${SOURCES} ${WINDOWS_SOURCES} ${DEVDRIVERSOURCES} ${WINDOWS_DD_SOURCES})
 ENDIF(UWP)
 include(GenerateExportHeader)
 generate_export_header(${EXECUTABLE})
 
 IF(CMAKE_CL_64)
  set( PLATFORM_POSTFIX x64)
 ELSE(CMAKE_CL_64)
  set( PLATFORM_POSTFIX x86 )
 ENDIF(CMAKE_CL_64)
 
 add_custom_command(TARGET ${EXECUTABLE}
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory $(SolutionDir)../$(Configuration)${INTERNAL_POSTFIX}/DevDriverAPI/bin
                    COMMAND ${CMAKE_COMMAND} -E make_directory $(SolutionDir)../$(Configuration)${INTERNAL_POSTFIX}/DevDriverAPI/bin/${PLATFORM_POSTFIX}
                    COMMAND ${CMAKE_COMMAND} -E make_directory $(SolutionDir)../$(Configuration)${INTERNAL_POSTFIX}/DevDriverAPI/lib
                    COMMAND ${CMAKE_COMMAND} -E make_directory $(SolutionDir)../$(Configuration)${INTERNAL_POSTFIX}/DevDriverAPI/lib/${PLATFORM_POSTFIX}
					COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/$(Configuration)/${EXECUTABLE}.lib $(SolutionDir)../$(Configuration)${INTERNAL_POSTFIX}/DevDriverAPI/lib/${PLATFORM_POSTFIX}
					COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/$(Configuration)/${EXECUTABLE}.exp $(SolutionDir)../$(Configuration)${INTERNAL_POSTFIX}/DevDriverAPI/lib/${PLATFORM_POSTFIX}
                    COMMAND move $(SolutionDir)../$(Configuration)${INTERNAL_POSTFIX}/${EXECUTABLE}.* $(SolutionDir)../$(Configuration)${INTERNAL_POSTFIX}/DevDriverAPI/bin/${PLATFORM_POSTFIX}
                    )
ELSEIF(APPLE)
 add_library(${EXECUTABLE} SHARED ${SOURCES} ${DEVDRIVERSOURCES} ${LINUX_DD_SOURCES})
ELSEIF(UNIX)
 add_library(${EXECUTABLE} SHARED ${SOURCES} ${DEVDRIVERSOURCES} ${LINUX_DD_SOURCES})
ENDIF(WIN32)

# executable file library dependency list
IF(WIN32)
 IF(UWP)
  target_link_libraries(${EXECUTABLE} ${DEVDRIVERMESSAGELIB} tdh.lib)
 ELSE(UWP)
  target_link_libraries(${EXECUTABLE} tdh.lib)
 ENDIF(UWP)
ELSEIF(UNIX)
 target_link_libraries(${EXECUTABLE} pthread)
 IF(NOT CMAKE_CL_64)
  set_target_properties(${EXECUTABLE} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
 ENDIF(NOT CMAKE_CL_64)
ENDIF(WIN32)

IF(WIN32)
# Create Visual Studio filters so that the source files in the project match the
# directory structure
foreach(source IN LISTS SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()
ENDIF(WIN32)
