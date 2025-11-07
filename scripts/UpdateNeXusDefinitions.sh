#!/usr/bin/sh

SCORE_HOME_FOLDER=$PWD
NEXUS_DEFINITIONS_FOLDER="src/cxx/thirdparty/mandatory/nexus_definitions"
DEV_PYNXTOOLS_FOLDER="src/cxx/thirdparty/testing/pynxtools/src/pynxtools/definitions"

echo $PWD
cd $NEXUS_DEFINITIONS_FOLDER
echo $PWD
git fetch
git pull
cd $SCORE_HOME_FOLDER
echo $PWD
cd $DEV_PYNXTOOLS_FOLDER
echo $PWD
git fetch
git pull
cd $SCORE_HOME_FOLDER
echo $PWD
