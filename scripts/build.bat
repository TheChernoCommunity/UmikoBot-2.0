@echo off
setlocal EnableDelayedExpansion

rem Directories
set originalDir=%CD%
set scriptDir=%~dp0
set projectRoot=%scriptDir%\..

echo Looking for Qt directory...
if exist %projectRoot%\tmp\qtDir.txt (
	set /p qtDir=<%projectRoot%\tmp\qtDir.txt
	echo Qt directory found automatically (!qtDir!\^)^!
) else (
	cd /D C:\Qt\5*\msvc*\ >nul 2>nul
	if ERRORLEVEL 1 (
		echo Qt directory not found automatically, please input...
		set /p qtDir="Qt Directory (e.g. C:\Qt\5.15.2\msvc2019_64\): "

		rem Writes Qt directory to file for future
		mkdir %projectRoot%\tmp\ >nul 2>nul
		break>%projectRoot%\tmp\qtDir.txt
		(echo | set /p=!qtDir!)>%projectRoot%\tmp\qtDir.txt
	) else (
		set qtDir=!cd!
		echo Qt directory found automatically (!qtDir!\^)^!
	)
)

rem Ensures submodules are present
echo.
echo Checking submodules...
cd %projectRoot%
git submodule update --init --recursive

rem Generate project files
echo.
echo Cleaning...
del /F /S /Q %projectRoot%\bin\* >nul 2>nul

echo.
echo Generating project files...
mkdir %projectRoot%\sln\ >nul 2>nul
cd %projectRoot%\sln\

rem Generates the project files
cmake -DCMAKE_PREFIX_PATH=%qtDir% ..

rem Cleanup
cd %originalDir%
exit /B 0
