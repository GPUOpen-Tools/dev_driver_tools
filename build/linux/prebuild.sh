#!/bin/bash

# Force error in any command to cause the script to abort with an error code
set -e

# Identify the version of QT to be used - define qtVersion and qtDir variables
#. $(dirname "$0")/findqt.sh

# Print help message
if [[ "$1" == "-h" ]] || [[ "$1" == "-help" ]] || [[ "$1" == "--h" ]] || [[ "$1" == "--help" ]]; then
    echo ""
    echo "This script generates Makefiles for the DevDriverTools on Linux."
    echo ""
    echo "Usage:  prebuild.sh [--cmake <cmake_path>] [--build <build_type>] [--platform <platform>] [--qt <qt5_root>] [--no-fetch] [--internal]"
    echo ""
    echo "Options:"
    echo "   --cmake        Path to cmake executable to use. If not specified, the cmake from PATH env variable will be used.\n"
    echo "   --clean        Clean out the old build folders.\n"
    echo "   --build        The build type: \"release\" or \"debug\". The default is \"release\"."
    echo "   --platform     The build platform: \"x64\" or \"x86\". The default is x64."
    echo "   --qt           Path to Qt5 root folder. The default is empty (cmake will look for Qt5 package istalled on the system)."
    echo "   --no-fetch     Do not call FetchDependencies.py script before running cmake."
    echo "   --internal     Internal build"
    echo ""
    echo "Examples:"
    echo "   prebuild.sh"
    echo "   prebuild.sh --build release"
    echo "   prebuild.sh --qt /opt/Qt5.9.2/5.9.2/gcc_64"
    echo "   prebuild.sh --build release"
    echo ""
    exit 0
fi

# Defaults
CMAKE_PATH="cmake"
config="Release"
platform="x64"
clean=false
INTERNAL_POSTFIX=
INTERNAL_FLAG=

qtVersion="5.9.2"
if [[ `uname` == "Darwin" ]]; then
    # MacOSX
    qtDir="/Qt/Qt$qtVersion/$qtVersion/clang_64"
else
    qtDir="$HOME/Qt/Qt$qtVersion/$qtVersion/gcc_64"
fi

# Parse command line arguments
args=("$@")
for ((i=0; i<$#; i++)); do
    arg=${args[i]}
    if [ "$arg" == "--build" ]; then
        i=$((i+1))
        config=${args[i]}
        if [ "$config" == "release" ]; then
            config="Release"
        fi
        if [ "$config" == "debug" ]; then
            config="Debug"
        fi
    elif [ "$arg" == "--clean" ]; then
        clean="TRUE"
    elif [ "$arg" == "--platform" ]; then
        i=$((i+1))
        platform=${args[i]}
    elif [ "$arg" == "--cmake" ]; then
        i=$((i+1))
        CMAKE_PATH=${args[i]}
    elif [ "$arg" == "--qt" ]; then
        i=$((i+1))
        qtDir=${args[i]}
    elif [ "$arg" == "--no-fetch" ]; then
        NO_UPDATE="TRUE"
    elif [ "$arg" == "--internal" ]; then
        INTERNAL_FLAG="-DINTERNAL_BUILD:BOOL=TRUE -DENABLE_UNIFIED_DRIVER_SETTINGS:BOOL=FALSE"
        INTERNAL_POSTFIX="-Internal"
    else
        echo "Unexpected argument: $arg. Aborting...";
        exit 1
    fi
done

# set default Qt path if not set up
# figure out the build postfix based on 32 or 64 bit build
postfix=""
if [[ $platform == "x64" ]]; then
    postfix="_64"
fi

# Fetch the dependencies if required
if [[ $NO_UPDATE != "TRUE" ]]; then
    python ../FetchDependencies.py
fi

# clean out the build folders if specified
if [[ $clean == "TRUE" ]]; then
    rm -rf output$INTERNAL_POSTFIX
    rm -rf Release$INTERNAL_POSTFIX
    rm -rf Debug$INTERNAL_POSTFIX
fi

# create the output folder if it doesn't exist
OUTPUT_FOLDER=Make
PLATFORM_POSTFIX=
if [[ $platform == "x86" ]]; then
    PLATFORM_POSTFIX=$platform
fi

dir="$OUTPUT_FOLDER$PLATFORM_POSTFIX$INTERNAL_POSTFIX/$config$INTERNAL_POSTFIX"
if [ ! -d $dir ]; then
    mkdir -p $dir
fi
pushd $dir
rm -rf *
if [[ $platform == "x64" ]]; then
    cmake ../../../.. -DCMAKE_BUILD_TYPE=$config $INTERNAL_FLAG -DCMAKE_CL_64=TRUE -DCMAKE_PREFIX_PATH="$qtDir/lib/cmake/"
else
    cmake ../../../.. -DCMAKE_BUILD_TYPE=$config $INTERNAL_FLAG
fi
popd

dir="$config$INTERNAL_POSTFIX"
if [ ! -d $dir ]; then
    mkdir -p $dir
fi
pushd $dir

if [[ $platform == "x64" ]]; then
    # copy the help files
    dir="docs"
    if [ ! -d $dir ]; then
        mkdir -p $dir
    fi

    cp -rf ../../../documentation/KeyboardDevice.txt docs
    mkdir -p scripts
    cp -rf ../../../scripts/RemoveSharedMemory.sh ./scripts
    chmod +x scripts/RemoveSharedMemory.sh
    cp -rf ../../../scripts/EnableSetClockMode.sh ./scripts
    chmod +x scripts/EnableSetClockMode.sh

    cp -rf ../../qt.conf .
fi

# make the DevDriverAPI directories
mkdir -p DevDriverAPI
mkdir -p DevDriverAPI/include
cp -rf ../../../source/DevDriverAPI/DevDriverAPI.h DevDriverAPI/include

# copy Qt runtime (not on mac for now. Also not on 32-bit since there's no 32-bit Qt)
if [[ `uname` != "Darwin" ]]; then
    if [[ $platform == "x64" ]]; then
        mkdir -p qt
        cd qt
        mkdir -p lib
        mkdir -p plugins
        cp -rfL $qtDir/lib/libicudata.so.56 lib
        cp -rfL $qtDir/lib/libicui18n.so.56 lib
        cp -rfL $qtDir/lib/libicuuc.so.56 lib
        cp -rfL $qtDir/lib/libQt5Core.so.5 lib
        cp -rfL $qtDir/lib/libQt5DBus.so.5 lib
        cp -rfL $qtDir/lib/libQt5Gui.so.5 lib
        cp -rfL $qtDir/lib/libQt5Widgets.so.5 lib
        cp -rfL $qtDir/lib/libQt5XcbQpa.so.5 lib
        cp -rfL $qtDir/lib/libQt5Svg.so.5 lib

        cd plugins
        mkdir -p platforms
        cp -rfL $qtDir/plugins/platforms/libqxcb.so platforms
        mkdir -p iconengines
        cp -rfL $qtDir/plugins/iconengines/libqsvgicon.so iconengines
        mkdir -p imageformats
        cp -rfL $qtDir/plugins/imageformats/libqsvg.so imageformats
    fi
fi
popd
