cmake_minimum_required (VERSION 2.6)

# policy to allow input directories to not have CMakeLists.txt files
if(POLICY CMP0014)
  cmake_policy(SET CMP0014 OLD)
endif()

# tell cmake to put the final executable in the correct place. Paths will be
# different on Windows & Linux
IF (INTERNAL_BUILD)
 project (DevDriverTools-Internal)
 IF(WIN32)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../Release-Internal)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../Debug-Internal)
 ELSE(WIN32)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../../Release-Internal)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../../Debug-Internal)
 ENDIF(WIN32)
ELSE (INTERNAL_BUILD)
 project (DevDriverTools)
 IF(WIN32)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../Release)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../Debug)
 ELSE(WIN32)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../../Release)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../../Debug)
 ENDIF(WIN32)
ENDIF(INTERNAL_BUILD)

# If this is an internal build, define the switch that RDS/RDP can use.
IF (INTERNAL_BUILD)
 IF(WIN32)
  add_compile_options(/D "RADEON_DRIVER_TOOLS_INTERNAL")
 ELSEIF(UNIX)
  add_compile_options(-DRADEON_DRIVER_TOOLS_INTERNAL)
 ENDIF(WIN32)
ENDIF(INTERNAL_BUILD)
	
add_definitions(-DGPUOPEN_CLIENT_INTERFACE_MAJOR_VERSION=30)

# Add .pdb files to Windows release build
IF(WIN32)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
ENDIF(WIN32)

include_directories(source/DevDriverComponents/inc)
include_directories(source/DevDriverComponents/message/inc)
IF(WIN32)
include_directories(source/DevDriverComponents/listener)
ELSE(WIN32)
add_subdirectory(source/DevDriverComponents/listener)
ENDIF(WIN32)

# Add the QtCommon Scaling project to the solution.
include_directories("${PROJECT_SOURCE_DIR}/../QtCommon/Scaling/")
add_subdirectory (${PROJECT_SOURCE_DIR}/../QtCommon/Scaling obj/QtCommon/Scaling)

# Add the QtCommon Util project to the solution.
include_directories("${PROJECT_SOURCE_DIR}/../QtCommon/Util/")
add_subdirectory (${PROJECT_SOURCE_DIR}/../QtCommon/Util obj/QtCommon/Util)

# Add RDS and RDP projects to solution.
add_subdirectory(source/DevDriverComponents/src         obj/DevDriverComponents)
add_subdirectory(source/RDS                             obj/RDS)
add_subdirectory(source/RDP                             obj/RDP)
add_subdirectory(source/DevDriverAPI                    obj/DevDriverAPI)
