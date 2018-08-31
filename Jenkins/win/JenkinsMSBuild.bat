set DEVDRIVERTOOLSPATH=%WORKSPACE%
set DEVDRIVERTOOLSBUILD=%DEVDRIVERTOOLSPATH%\build\win\VS2017

cd %DEVDRIVERTOOLSBUILD%
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\Common7\Tools\VsDevCmd.bat"
REM set the build folder again as VsDevCmd.bat seems to set it to something else
cd /D %DEVDRIVERTOOLSBUILD%

REM Build Debug|x64 configuration
msbuild /m:3 /t:Build /p:Configuration=Debug /p:Platform=x64 /p:OutputPath=%DEVDRIVERTOOLSBUILD% /verbosity:minimal DevDriverTools.sln
if not %ERRORLEVEL%==0 (
    echo "Build Failed for file DevDriverTools.sln, configuration Debug|x64"
    exit 1
)

REM Build Release|x64 configuration
msbuild /m:3 /t:Build /p:Configuration=Release /p:Platform=x64 /p:OutputPath=%DEVDRIVERTOOLSBUILD% /verbosity:minimal DevDriverTools.sln
if not %ERRORLEVEL%==0 (
    echo "Build Failed for file DevDriverTools.sln, configuration Release|x64"
    exit 1
)
