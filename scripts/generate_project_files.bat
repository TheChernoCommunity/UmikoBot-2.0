@echo off

rem Directories
set scriptDir=%~dp0
set projectRoot=%scriptDir%\..

rem Locates premake
where premake5

if ERRORLEVEL 1 (
	set premakeCommand=%projectRoot%\pmk\premake5
) else (
	set premakeCommand=premake5
)

rem Use custom arguments if passed
if "%~1"=="" (
	set args=vs2019
) else (
	set args=%*
)

%premakeCommand% %args%
