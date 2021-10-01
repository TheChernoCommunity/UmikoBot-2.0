#!/bin/bash

# Directories
scriptDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" > /dev/null && pwd )"
projectRoot=$scriptDir/..

# Locates premake
if ! command -v premake5 &> /dev/null; then
	if [ ! -f "$projectRoot/pmk/premake5" ]; then
		echo "Premake executable not found, run init.sh to get one."
		read -p "Press enter to continue..."
		exit
	else
		premakeCommand="$projectRoot/pmk/premake5"
	fi
else
	premakeCommand="premake5"
fi

# Use custom arguments if passed
if [ $# -gt 0 ]; then
	args=$@
else
	# Determine default action from system
	os=$(uname -s)
	if [ "$os" == "Linux" ]; then
		args="qmake"
	elif [ "$os" == "Darwin" ]; then
		args="xcode4"
	else
		args="vs2019"
	fi
fi

$premakeCommand $args
