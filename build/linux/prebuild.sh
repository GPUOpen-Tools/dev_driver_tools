#!/bin/bash

# Build the Debug and Release makefiles

qtVersion="5.9.2"
qtDir="$HOME/Qt$qtVersion/$qtVersion/gcc_64"
for config in Debug Release ; do
    # create the output folder if it doesn't exist
    dir="output/$config"
    if [ ! -d $dir ]; then
        mkdir -p $dir
    fi
    pushd $dir
    rm -rf *
    if [[ `uname` == "Darwin" ]]; then
        # MacOSX
        cmake ../../../.. -DCMAKE_BUILD_TYPE=$config -DCMAKE_PREFIX_PATH=~/Qt5.7.1/5.7/clang_64/lib/cmake/
    else
        cmake ../../../.. -DCMAKE_BUILD_TYPE=$config -DCMAKE_PREFIX_PATH="$qtDir/lib/cmake/"
    fi
    popd

    # copy the help files
    dir="$config/docs"
    if [ ! -d $dir ]; then
        mkdir -p $dir
    fi
    dir="$config"
    pushd $dir

    cp -rf ../../../documentation/KeyboardDevice.txt docs
    mkdir -p scripts
    cp -rf ../../../scripts/RemoveSharedMemory.sh ./scripts
    chmod +x scripts/RemoveSharedMemory.sh
    cp -rf ../../../scripts/EnableSetClockMode.sh ./scripts
    chmod +x scripts/EnableSetClockMode.sh

    cp -rf ../../qt.conf .

    # copy Qt runtime (not on mac for now)
    if [[ `uname` != "Darwin" ]]; then
        mkdir qt
        cd qt
        mkdir lib
        mkdir plugins
        cp -rf $qtDir/lib/libicudata.so.56.1 lib/libicudata.so.56
        cp -rf $qtDir/lib/libicui18n.so.56.1 lib/libicui18n.so.56
        cp -rf $qtDir/lib/libicuuc.so.56.1 lib/libicuuc.so.56
        cp -rf $qtDir/lib/libQt5Core.so.$qtVersion lib/libQt5Core.so.5
        cp -rf $qtDir/lib/libQt5DBus.so.$qtVersion lib/libQt5DBus.so.5
        cp -rf $qtDir/lib/libQt5Gui.so.$qtVersion lib/libQt5Gui.so.5
        cp -rf $qtDir/lib/libQt5Widgets.so.$qtVersion lib/libQt5Widgets.so.5
        cp -rf $qtDir/lib/libQt5XcbQpa.so.$qtVersion lib/libQt5XcbQpa.so.5
        cp -rf $qtDir/lib/libQt5Svg.so.$qtVersion lib/libQt5Svg.so.5

        cd plugins
        mkdir platforms
        cp -rf $qtDir/plugins/platforms/libqxcb.so platforms
        mkdir iconengines
        cp -rf $qtDir/plugins/iconengines/libqsvgicon.so iconengines
        mkdir imageformats
        cp -rf $qtDir/plugins/imageformats/libqsvg.so imageformats
    fi
    popd
done


