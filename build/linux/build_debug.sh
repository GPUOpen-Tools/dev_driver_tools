# build the targets
cd output/Debug
make -j5
cd ../..

# Tell RGP to look in the local lib path rather than the Qt install path
if [[ `uname` != "Darwin" ]]; then
    chrpath -r '$ORIGIN/qt/lib' Debug/RadeonDeveloperPanel
    chrpath -r '$ORIGIN/qt/lib' Debug/RadeonDeveloperService
fi

./postbuild.sh Debug
