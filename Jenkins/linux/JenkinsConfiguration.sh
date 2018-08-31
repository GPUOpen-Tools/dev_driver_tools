#!/bin/bash -xl

# create the makefiles and build everything
cd ../../build/linux
./prebuild.sh
./build_release.sh

if [[ `uname` != "Darwin" ]]; then
    # build the help files
    sphinxDir="../../docs"
    pushd $sphinxDir
    ./make.sh html
    popd

    # make the destination help directory and move the help files
    dir="Release/docs/help/rdp"
    if [ ! -d $dir ]; then
        mkdir -p $dir
    fi
    mv  ../../docs/build/html Release/docs/help/rdp
fi

mv Release DevDriverTools.$1

# archive it all
tar cfz DevDriverTools.$1.tgz DevDriverTools.$1
tar cfz DevDriverAPI.$1.tgz DevDriverTools.$1/DevDriverAPI
