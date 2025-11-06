#!/bin/sh

# EXECUTE starting from the home directory of SCORE

# GNU C/C++ compiler
MY_C_COMPILER=gcc-14
MY_CXX_COMPILER=g++-14
# Intel oneAPI C/C++ compiler
# make sure to call source setvars.sh within the with which you compile the paraprobe tools
# so that the Intel oneAPI compiler and library locations will be resolved properly
# MY_C_COMPILER=icx
# MY_CXX_COMPILER=icpx

# alternatively mpicxx
if [ `git describe` ]; then
    SCORE_VERSION=`git describe --dirty --tags --long --abbrev=8 --match "*[0-9]*"`
else
    if [ `git rev-parse HEAD` ]; then
        SCORE_VERSION=`git rev-parse HEAD`
    else
        SCORE_VERSION=0
    fi
fi
echo $SCORE_VERSION
cd src/cxx/thirdparty/mandatory/nexus_definitions
if [ `git describe` ]; then
    NEXUS_VERSION=`git describe --dirty --tags --long --abbrev=8 --match "*[0-9]*"`
else
    if [ `git rev-parse HEAD` ]; then
        NEXUS_VERSION=`git rev-parse HEAD`
    else
        NEXUS_VERSION=0
    fi
fi
echo $NEXUS_VERSION
cd ../../../../../
echo $PWD

# rm -rf build/
mkdir -p build
cd build
echo $PWD
cmake -DSCOREVERSION=$SCORE_VERSION -DNEXUSVERSION=$NEXUS_VERSION -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=mpicxx ..
make -j16
chmod +x score
cd ..
