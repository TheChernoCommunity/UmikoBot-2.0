@echo off

set scriptDir=%~dp0
set projectRoot=%scriptDir%\..
set PREMAKE_VERSION=5.0.0-alpha16

mkdir tmp bin bin\x64

rem Tries to locate premake
cd %projectRoot%\pmk
where premake5

if %ERRORLEVEL% neq 0 (
	rem Premake not found, download it
	powershell -nologo -noprofile -command "Invoke-WebRequest https://github.com/premake/premake-core/releases/download/v%PREMAKE_VERSION%/premake-%PREMAKE_VERSION%-windows.zip -OutFile %projectRoot%\tmp\premake5.zip"
	powershell -nologo -noprofile -command "& { $shell = New-Object -COM Shell.Application; $target = $shell.NameSpace('%projectRoot%\pmk'); $zip = $shell.NameSpace('%projectRoot%\tmp\premake5.zip'); $target.CopyHere($zip.Items(), 16); }"
)

rem Tries to locate Qt files
for /r C:\Qt\5* %%a in (*) do if "%%~nxa"=="Qt5Core.dll" set qtDir=%%~dpa
if defined qtDir (
	echo Found Qt folder: %qtDir%
) else (
	rem Qt folder not found
	set /p qtDir="Qt Directory: "
)

rem Copies Qt files to output directory
xcopy "%qtDir%\..\plugins\platforms\qwindows.dll" "%projectRoot%\bin\x64\"
xcopy "%qtDir%\..\plugins\platforms\qwindowsd.dll" "%projectRoot%\bin\x64\"
xcopy "%qtDir%\bin\Qt5Core.dll" "bin\x64\"
xcopy "%qtDir%\bin\Qt5Cored.dll" "bin\x64\"
xcopy "%qtDir%\bin\Qt5Gui.dll" "bin\x64\"
xcopy "%qtDir%\bin\Qt5Guid.dll" "bin\x64\"
xcopy "%qtDir%\bin\Qt5Network.dll" "bin\x64\"
xcopy "%qtDir%\bin\Qt5Networkd.dll" "bin\x64\"
xcopy "%qtDir%\bin\Qt5WebSockets.dll" "bin\x64\"
xcopy "%qtDir%\bin\Qt5WebSocketsd.dll" "bin\x64\"
xcopy "%qtDir%\bin\Qt5Widgets.dll" "bin\x64\"
xcopy "%qtDir%\bin\Qt5Widgetsd.dll" "bin\x64\"

rem Init or update submodules
git submodule update --init --recursive
