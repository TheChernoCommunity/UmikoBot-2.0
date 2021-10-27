#!/bin/bash

# Directories
originalDir=$(pwd)
scriptDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" > /dev/null && pwd )"
projectRoot=$scriptDir/..

# Ensures submodules are present
cd $projectRoot
git submodule update --init --recursive

# Generating files
mkdir $projectRoot/sln
cd $projectRoot/sln

cmake ..

cd $originalDir
