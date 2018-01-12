#!/bin/bash

# Define some variables that identify the version of QT being used. These will be used across the other build scripts

qtVersion="5.9.2"

if [[ `uname` == "Darwin" ]]; then
    # MacOSX
    qtDir="$HOME/Qt$qtVersion/$qtVersion/clang_64"
else
    qtDir="$HOME/Qt$qtVersion/$qtVersion/gcc_64"
fi
