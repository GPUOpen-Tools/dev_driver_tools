@echo off
SETLOCAL

rem Print help message
if "%1"=="-h" goto :print_help
if "%1"=="-help" goto :print_help
if "%1"=="--h" goto :print_help
if "%1"=="--help" goto :print_help
if "%1"=="/?" goto :print_help

goto :start

:print_help
echo:
echo This script generates Visual Studio project and solution files for the DevDriverTools on Windows.
echo:
echo Usage:  prebuild.bat ^[--cmake ^<cmake_path^>^] ^[--vs ^<vs_version^>^] ^[--qt ^<qt5_root^>^] ^[--platform ^<Platform^>^]
echo:
echo Options:
echo    --cmake        Path to cmake executable to use. If not specified, the cmake from PATH env variable will be used.
echo    --clean        Clean out the old build folders
echo    --platform     The build platform: "x64" or "x86". The default is x64.
echo    --vs           Microsoft Visual Studio verson. Currently supported values are: "2015", "2017". The default is "2017".
echo    --qt           Path to Qt5 root folder. The default is empty (cmake will look for Qt5 package istalled on the system).
echo    --no-fetch     Do not call FetchDependencies.py script before running cmake.
echo    --internal     Internal build
echo:
echo Examples:
echo    prebuild.bat
echo    prebuild.bat --vs 2017 --qt C:\Qt\5.7\msvc2015_64
echo    prebuild.bat --vs 2015
goto :exit

:start

: set QtPath and Qt generator
set QtPath=C:\Qt\Qt5.9.2\5.9.2\msvc2017_64
set QtGenerator="Visual Studio 15 2017 Win64"
set Platform=x64
set VS_VER=2017
set NO_UPDATE=FALSE
set CLEAN=FALSE
set INTERNAL_POSTFIX=
set INTERNAL_FLAGS=

:begin
if [%1]==[] goto :start_cmake
if "%1"=="--cmake" goto :set_cmake
if "%1"=="--clean" goto :set_clean
if "%1"=="--platform" goto :set_platform
if "%1"=="--vs" goto :set_vs
if "%1"=="--qt" goto :set_qt
if "%1"=="--no-fetch" goto :set_no_update
if "%1"=="--internal" goto :set_internal
goto :bad_arg

:set_cmake
set CMAKE_PATH=%2
goto :shift_2args
:set_clean
set CLEAN=TRUE
goto :shift_arg
:set_platform
set Platform=%2
goto :shift_2args
:set_vs
set VS_VER=%2
goto :shift_2args
:set_qt
set QtPath=%2
goto :shift_2args
:set_no_update
set NO_UPDATE=TRUE
goto :shift_arg
:set_internal
set INTERNAL_POSTFIX=-Internal
set INTERNAL_FLAGS=-DINTERNAL_BUILD:BOOL=TRUE -DENABLE_DRIVER_LOGGING:BOOL=FALSE -DENABLE_UNIFIED_DRIVER_SETTINGS:BOOL=FALSE
goto :shift_arg

:shift_2args
rem Shift to the next pair of arguments
shift
:shift_arg
shift
goto :begin

:bad_arg
echo Error: Unexpected argument: %1%. Aborting...
exit /b

:start_cmake

if "%VS_VER%"=="2015" (
    if "%Platform%"=="x64" (
        set QtGenerator="Visual Studio 14 2015 Win64"
    ) else (
        set QtGenerator="Visual Studio 14 2015"
    )
) else (
    if "%VS_VER%"=="2017" (
        if "%Platform%"=="x64" (
            set QtGenerator="Visual Studio 15 2017 Win64"
    	) else (
            set QtGenerator="Visual Studio 15 2017"
        )
    ) else (
        echo Error: Unknown VisualStudio version provided. Aborting...
        exit /b
    )
)

: pull dependencies from git
rem clone or download dependencies
if not "%NO_UPDATE%"=="TRUE" (
    echo:
    echo Updating Common...
    python ..\..\build\FetchDependencies.py
    echo ErrorLevel %ERRORLEVEL%
)

: create an output folder
set VS_FOLDER=VS%VS_VER%
set PLATFORM_POSTFIX=
if "%Platform%"=="x86" (
    set PLATFORM_POSTFIX=%Platform%
)
set OUTPUT_FOLDER=%VS_FOLDER%%PLATFORM_POSTFIX%%INTERNAL_POSTFIX%

: Clean out all temporary/build folders
if "%CLEAN%"=="TRUE" (
    rmdir /S /Q %OUTPUT_FOLDER%
    rmdir /S /Q Release%INTERNAL_POSTFIX%
    rmdir /S /Q Debug%INTERNAL_POSTFIX%
)

: copy release and debug files to target folders
call :BuildVer "Release"
call :BuildVer "Debug"

: make the output folder
if not exist %OUTPUT_FOLDER% (
    mkdir %OUTPUT_FOLDER%
)

: generate the build files
cd %OUTPUT_FOLDER%
cmake ../../.. -DUWP:BOOL=FALSE %INTERNAL_FLAGS% -DCMAKE_PREFIX_PATH=%QtPath%/lib/cmake -G %QtGenerator%
cd ..
exit /B

: Copy some files to the target folder
: Pass in: Debug or Release depending on build type
:BuildVer
    echo off
    set postfix=""
    if %1=="Debug" (
        set postfix="d"
    )

    : make folders for DevDriverAPI & copy header file
    if not exist "%1%INTERNAL_POSTFIX%\DevDriverAPI" mkdir %1%INTERNAL_POSTFIX%\DevDriverAPI
    if not exist "%1%INTERNAL_POSTFIX%\DevDriverAPI\include" mkdir %1%INTERNAL_POSTFIX%\DevDriverAPI\include
    copy ..\..\source\DevDriverAPI\DevDriverAPI.h %1%INTERNAL_POSTFIX%\DevDriverAPI\include
	
    : copy Qt debug files
    if "%Platform%" == "x64" (
        if not exist "%1%INTERNAL_POSTFIX%\platforms" mkdir %1%INTERNAL_POSTFIX%\platforms
        if not exist "%1%INTERNAL_POSTFIX%\imageformats" mkdir %1%INTERNAL_POSTFIX%\imageformats
        copy %QtPath%\bin\Qt5Core%postfix%.* %1%INTERNAL_POSTFIX%
        copy %QtPath%\bin\Qt5Gui%postfix%.* %1%INTERNAL_POSTFIX%
        copy %QtPath%\bin\Qt5Svg%postfix%.* %1%INTERNAL_POSTFIX%
        copy %QtPath%\bin\Qt5Widgets%postfix%.* %1%INTERNAL_POSTFIX%
        copy %QtPath%\plugins\platforms\qwindows%postfix%.* %1%INTERNAL_POSTFIX%\platforms
        copy %QtPath%\plugins\imageformats\qico%postfix%.* %1%INTERNAL_POSTFIX%\imageformats

        if not exist "%1%INTERNAL_POSTFIX%\docs" mkdir %1%INTERNAL_POSTFIX%\docs
    )

    goto exit:

:exit
