# build the targets
cd output/Release
make
cd ../..

# Tell RGP to look in the local lib path rather than the Qt install path
if [[ `uname` != "Darwin" ]]; then
    chrpath -r '$ORIGIN/qt/lib' Release/RadeonDeveloperPanel
    chrpath -r '$ORIGIN/qt/lib' Release/RadeonDeveloperService
fi

./postbuild.sh Release
