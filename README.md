To build on Windows (Visual Studio 2017):
-----------------------------------------
Download Qt5.9.2 for Windows (the .exe file) from:
 https://download.qt.io/official_releases/qt/5.9/5.9.2
and install it to the default location when prompted by the installer.

Get and Install cmake from:
 https://cmake.org/download/
Any version higher than 3.9.1 should be good enough (the Visual Studio 2017
generator is required)

Run the prebuild.bat file in the build/win folder. This can be done from a DOS
prompt or by double-clicking on the file. This will create a sub-folder called
'output' containing the necessary build files.

Go into the 'output' folder (build/win/output) and double click on the VS 2017
.sln file (DevDriverTools.sln) and build the 64-bit Debug and Release builds.

The release and debug builds of the Developer Driver Tools are now available
in the build/win/Release and build/win/Debug folders.


To build on Linux (command line):
---------------------------------
Download Qt5.9.2 for linux (the .run file) from:
 https://download.qt.io/official_releases/qt/5.9/5.9.2
and install it to the default location when prompted by the installer.

Install cmake using:
 sudo apt-get update
 sudo apt-get install cmake

Install chrpath using:
 sudo apt-get install chrpath

Run the configure.sh script (in the build/linux directory).

$ ./prebuild.sh

This will construct the output folder and build the necessary makefiles. To
build the release build, use:

$ ./build_release.sh

Similarly, use:

$ ./build_debug.sh

for the debug build.

The prebuild.sh script should only need to be used when adding or removing
source files.


If a newer version of Qt needs to be used, it can be downloaded from the Qt website.
Go to https://www.qt.io/download/ and select one of the offline packages for Linux.
Run this (you may need to add execute permissions (chmod +x <downloaded file>). This
installer will ask for an install location. Once installed, edit the prebuild script file
to point to the cmake folder where the new version of Qt is installed. Simply edit the
line
        cmake ../../../.. -DCMAKE_BUILD_TYPE=$config
to
        cmake ../../../.. -DCMAKE_BUILD_TYPE=$config -DCMAKE_PREFIX_PATH=~/Qt5.9.2/5.9.2/gcc_64/lib/cmake/

For Qt5.9.2 installed to the current users' root folder in the Qt5.9.2 folder in this example
