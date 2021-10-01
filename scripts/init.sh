#!/bin/bash

# Directories
scriptDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" > /dev/null && pwd )"
projectRoot=$scriptDir/..

# Tries to detect the system, or assumes Windows
os=$(uname -s)
if [ "$os" == "Linux" ]; then
	os="linux"
elif [ "$os" == "Darwin" ]; then
	os="osx"
else
	os="windows"
fi

if [ "$os" == "windows" ]; then
	echo "Windows is not supported at this time..."
elif [ "$os" == "linux" ]; then
	# Tries to locate premake, otherwise downloads it
	if ! command -v premake5 &> /dev/null; then
		if [ ! -f "$projectRoot/pmk/premake5" ]; then
			mkdir "$projectRoot/tmp"
			$(curl -L -o "$projectRoot/tmp/premake5.tar.gz" "https://github.com/premake/premake-core/releases/download/v$PREMAKE_VERSION/premake-5.0.0-alpha12-linux.tar.gz")
			$(tar -xvzf "$projectRoot/tmp/premake5.tar.gz" -C "$projectRoot/pmk")
			rm -r "$projectRoot/tmp/"
		else
			echo "Found pmk/premake5..."
		fi
	else
		echo "Found global premake command..."
	fi
fi

# Init or update submodules
git submodule update --init --recursive
