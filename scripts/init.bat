@echo off

set scriptDir=%~dp0
set projectRoot=%scriptDir%\..
set PREMAKE_VERSION=5.0.0-alpha16

mkdir tmp bin bin\x64 >nul 2>nul

echo Looking for premake...
cd %projectRoot%\pmk
where premake5 >nul 2>nul

if ERRORLEVEL 1 (
	echo Premake not found, downloading it...
	powershell -nologo -noprofile -command "(New-Object Net.WebClient).DownloadFile('https://github.com/premake/premake-core/releases/download/v%PREMAKE_VERSION%/premake-%PREMAKE_VERSION%-windows.zip', '%projectRoot%\tmp\premake5.zip')"
	powershell Expand-Archive %projectRoot%\tmp\premake5.zip -DestinationPath %projectRoot%\pmk\
	echo Downloaded and unzipped premake to pmk\premake5.exe!
) else (
	echo Found premake!
)

echo Looking for Qt directory...
cd C:\Qt\5*\msvc*\ >nul 2>nul
if ERRORLEVEL 1 (
	echo Qt directory not found automatically, please input...
	set /p qtDir="Qt Directory (e.g. C:\Qt\5.15.2\msvc2019_64\): "
	
	cd %qtDir%
	if ERRORLEVEL 1 (
		echo Qt directory not found!
		goto :end
	) else (
		echo Found directory!
	)
) else (
	echo Qt directory found automatically (%CD%\^)^!
)

echo Copying Qt files to output directory...
xcopy /y /q "plugins\platforms\qwindows.dll" "%projectRoot%\bin\x64\" >nul
xcopy /y /q "plugins\platforms\qwindowsd.dll" "%projectRoot%\bin\x64\" >nul
xcopy /y /q "bin\Qt5Core.dll" "%projectRoot%\bin\x64\" >nul
xcopy /y /q "bin\Qt5Cored.dll" "%projectRoot%\bin\x64\" >nul
xcopy /y /q "bin\Qt5Gui.dll" "%projectRoot%\bin\x64\" >nul
xcopy /y /q "bin\Qt5Guid.dll" "%projectRoot%\bin\x64\" >nul
xcopy /y /q "bin\Qt5Network.dll" "%projectRoot%\bin\x64\" >nul
xcopy /y /q "bin\Qt5Networkd.dll" "%projectRoot%\bin\x64\" >nul
xcopy /y /q "bin\Qt5WebSockets.dll" "%projectRoot%\bin\x64\" >nul
xcopy /y /q "bin\Qt5WebSocketsd.dll" "%projectRoot%\bin\x64\" >nul
xcopy /y /q "bin\Qt5Widgets.dll" "%projectRoot%\bin\x64\" >nul
xcopy /y /q "bin\Qt5Widgetsd.dll" "%projectRoot%\bin\x64\" >nul

cd %projectRoot%

echo Initialising submodules
git submodule update --init --recursive

:end
cd %projectRoot%
