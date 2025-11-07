#!/usr/bin/sh

echo $PWD
DEFINITIONS_FOLDER="src/cxx/thirdparty/mandatory/nexus_definitions"

cd $DEFINITIONS_FOLDER && echo "updating nexus-version.txt"
printf "$(git describe --dirty --tags --long --abbrev=8 --match '*[0-9]*')" > ../nexus-version.txt
cd ../../../../../ && echo $PWD
