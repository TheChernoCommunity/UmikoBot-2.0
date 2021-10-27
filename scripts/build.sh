#!/bin/bash

# Directories
originalDir=$(pwd)
scriptDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" > /dev/null && pwd )"
projectRoot=$scriptDir/..

echo Cleaning...
rm -r $projectRoot/bin/ 2> /dev/null

echo
echo Generating project files...
$scriptDir/generate_project_files.sh

echo
echo Building...
cd $projectRoot/sln/

# nproc returns the number of CPU cores/threads available
make -j $(nproc) || exit 1

echo
echo Running...
cd $projectRoot/res/

$projectRoot/bin/UmikoBot -platform offscreen "$@"

cd $originalDir
exit 0
