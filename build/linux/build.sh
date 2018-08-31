#!/bin/bash

# Force error in any command to cause the script to abort with an error code
set -e

# Print help message
if [[ "$1" == "-h" ]] || [[ "$1" == "-help" ]] || [[ "$1" == "--h" ]] || [[ "$1" == "--help" ]]; then
    echo ""
    echo "This script builds the DevDriverTools on Linux."
    echo ""
    echo "Usage:  build.sh [--build <build_type>] [--internal]"
    echo ""
    echo "Options:"
    echo "   --build        The build type: \"release\" or \"debug\". The default is \"release\"."
    echo "   --internal     Internal build"
    echo "   --platform     The build platform: \"x64\" or \"x86\". The default is x64."
    echo ""
    echo "Examples:"
    echo "   build.sh"
    echo "   build.sh --build release"
    echo "   build.sh --internal --build release"
    echo ""
    exit 0
fi

# Defaults
config="Release"
INTERNAL_POSTFIX=
platform=x64

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
    elif [ "$arg" == "--platform" ]; then
        i=$((i+1))
        platform=${args[i]}
    elif [ "$arg" == "--internal" ]; then
        INTERNAL_FLAG="-DINTERNAL_BUILD:BOOL=TRUE"
        INTERNAL_POSTFIX="-Internal"
    else
        echo "Unexpected argument: $arg. Aborting...";
        exit 1
    fi
done

# build the targets
OUTPUT_FOLDER=Make
PLATFORM_POSTFIX=
if [[ $platform == "x86" ]]; then
    PLATFORM_POSTFIX=$platform
    EXE_PLATFORM_POSTFIX="32"
fi

dir="$OUTPUT_FOLDER$PLATFORM_POSTFIX$INTERNAL_POSTFIX/$config$INTERNAL_POSTFIX"
pushd $dir
make -j5
popd

# Tell RGP to look in the local lib path rather than the Qt install path
if [[ `uname` == "Darwin" ]]; then
    qtVersion="5.9.2"
    qtDir="/Qt/Qt$qtVersion/$qtVersion/clang_64"
    $qtDir/bin/macdeployqt $config$INTERNAL_POSTFIX/RadeonDeveloperPanel$INTERNAL_POSTFIX.app
    $qtDir/bin/macdeployqt $config$INTERNAL_POSTFIX/RadeonDeveloperService$INTERNAL_POSTFIX.app
else
    file=$config$INTERNAL_POSTFIX/RadeonDeveloperPanel$INTERNAL_POSTFIX
    if [ -f $file ]; then
        chrpath -r '$ORIGIN/qt/lib' $file
    fi
    file=$config$INTERNAL_POSTFIX/RadeonDeveloperService$INTERNAL_POSTFIX
    if [ -f $file ]; then
        chrpath -r '$ORIGIN/qt/lib' $file
    fi
fi

# Post build step
pushd $dir
dest=../../$config$INTERNAL_POSTFIX/DevDriverAPI/lib
if [ ! -d $dest ]; then
    mkdir -p $dest
fi
cp -rf obj/DevDriverAPI/libDevDriverAPI$EXE_PLATFORM_POSTFIX$INTERNAL_POSTFIX.* $dest
popd

