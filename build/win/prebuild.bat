: set QtPath and Qt generator
set QtPath=C:\Qt\Qt5.9.2\5.9.2\msvc2017_64
set QtGenerator="Visual Studio 15 2017 Win64"

: overwrite QtPath with optional command line argument
if exist "%~f1" set QtPath=%~f1
if [%2] NEQ [] set QtGenerator=%2

: pull dependencies from git
python ..\..\build\FetchDependencies.py

: copy release and debug files to target folders
call :BuildVer "Release"
call :BuildVer "Debug"

: generate the build files
if not exist "output" mkdir output
cd output
cmake ../../.. -DUWP:BOOL=FALSE -DCMAKE_PREFIX_PATH=%QtPath%\lib\cmake -G %QtGenerator%
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

    : copy Qt debug files
    if not exist "%1\platforms" mkdir %1\platforms
    if not exist "%1\imageformats" mkdir %1\imageformats
    if not exist "%1\DevDriverAPI" mkdir %1\DevDriverAPI
    if not exist "%1\DevDriverAPI\include" mkdir %1\DevDriverAPI\include
    if not exist "%1\DevDriverAPI\bin" mkdir %1\DevDriverAPI\bin
    if not exist "%1\DevDriverAPI\lib" mkdir %1\DevDriverAPI\lib
    copy %QtPath%\bin\Qt5Core%postfix%.* %1
    copy %QtPath%\bin\Qt5Gui%postfix%.* %1
    copy %QtPath%\bin\Qt5Svg%postfix%.* %1
    copy %QtPath%\bin\Qt5Widgets%postfix%.* %1
    copy %QtPath%\plugins\platforms\qwindows%postfix%.* %1\platforms
    copy %QtPath%\plugins\imageformats\qico%postfix%.* %1\imageformats

    if not exist "%1\docs" mkdir %1\docs

    goto EOF
