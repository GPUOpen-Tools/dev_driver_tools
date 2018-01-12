: set QtPath
set QtPath=C:\Qt\Qt5.9.2\5.9.2\msvc2017_64

: pull dependencies from git
python ..\..\scripts\UpdateCommon.py

: overwrite QtPath with optional command line argument
IF EXIST "%~f1" SET QtPath=%~f1

: copy Qt release files
if not exist "Release\platforms" mkdir Release\platforms
if not exist "Release\imageformats" mkdir Release\imageformats
copy %QtPath%\bin\Qt5Core.dll Release
copy %QtPath%\bin\Qt5Gui.dll Release
copy %QtPath%\bin\Qt5Svg.dll Release
copy %QtPath%\bin\Qt5Widgets.dll Release
copy %QtPath%\plugins\platforms\qwindows.dll Release\platforms
copy %QtPath%\plugins\imageformats\qico.dll Release\imageformats

if not exist "Release\docs" mkdir Release\docs

: copy Qt debug files
if not exist "Debug\platforms" mkdir Debug\platforms
if not exist "Debug\imageformats" mkdir Debug\imageformats
copy %QtPath%\bin\Qt5Cored.* Debug
copy %QtPath%\bin\Qt5Guid.* Debug
copy %QtPath%\bin\Qt5Svgd.* Debug
copy %QtPath%\bin\Qt5Widgetsd.* Debug
copy %QtPath%\plugins\platforms\qwindowsd.* Debug\platforms
copy %QtPath%\plugins\imageformats\qicod.* Debug\imageformats

if not exist "Debug\docs" mkdir Debug\docs

if not exist "output" mkdir output
cd output
cmake ../../.. -DUWP:BOOL=FALSE -DCMAKE_PREFIX_PATH=%QtPath%\lib\cmake -G "Visual Studio 15 2017 Win64"
cd ..
