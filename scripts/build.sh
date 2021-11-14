#!/bin/bash

# Directories
originalDir=$(pwd)
scriptDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" > /dev/null && pwd )"
projectRoot=$scriptDir/..

echo Cleaning...
rm -r $projectRoot/bin/ 2> /dev/null

# Ensures submodules are present
echo
echo Checking submodules...
cd $projectRoot
git submodule update --init --recursive

echo
echo Generating project files...
mkdir $projectRoot/sln/ 2> /dev/null
cd $projectRoot/sln/
# Generates the project files
cmake .. "${@:2}"

echo
echo Building...
# nproc returns the number of CPU cores/threads available
make -j $(nproc) || exit 1

echo
echo Running...
cd $projectRoot/res/

$projectRoot/bin/UmikoBot -platform offscreen "$1"

cd $originalDir
exit 0
