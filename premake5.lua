if _ACTION:match("vs*") then
	require "pmk/extensions/premake-qt/qt"
elseif _ACTION == "qmake" then
	require "pmk/extensions/premake-qmake/qmake"
end

local qtDir = io.readfile("tmp/qtDir.txt")

workspace "UmikoBot"
	location "sln/"
	configurations {
		"Debug",
		"Release",
	}
	platforms {
		"x64",
	}

project "UmikoBot"
	location "sln/prj/"
	
	kind "ConsoleApp"
	cppdialect "C++17"
	flags {
		"MultiProcessorCompile",
	}
	
	files {
		"src/**.cpp",
		"src/**.h",
	}
	includedirs {
		"src/",
		"dep/QDiscord/src/core/",
	}
	links {
		"QDiscordCore",
	}
	qtmodules {
		"core",
		"gui",
		"network",
		"websockets",
		"widgets",
	}

	filter { "configurations:Release" }
		optimize "Full"
		defines {
			"QT_NO_DEBUG",
		}

	filter { "platforms:x64" }
		debugdir "res/"
		objdir "obj/"
		targetdir "bin/"

	filter {"toolset:msc"}
		disablewarnings { "C4996" }

	filter {}

	-- Enable premake-qt when targeting Visual Studio
	if _ACTION:match("vs*") then
		premake.extensions.qt.enable()
		qtprefix "Qt5"
		qtgenerateddir "src/GeneratedFiles/"

		filter { "configurations:Debug" }
			qtsuffix "d"

		filter { "platforms:x64" }
			qtpath (qtDir)
	end

group "QDiscord"

project "QDiscordCore"
	location "sln/prj/"
	kind "StaticLib"
	cppdialect "C++17"
	flags {
		"MultiProcessorCompile",
	}
	
	files {
		"dep/QDiscord/src/core/**.h",
		"dep/QDiscord/src/core/**.cpp",
	}
	includedirs {
		"dep/QDiscord/src/core/",
	}

	filter {"configurations:Release"}
		optimize "Full"
		defines {
			"QT_NO_DEBUG",
		}

	filter { "platforms:x64" }
		objdir "obj/"
		targetdir "bin/"

	filter {}

	-- Enable premake-qt when targeting Visual Studio
	if _ACTION:match("vs*") then
		premake.extensions.qt.enable()
		qtprefix "Qt5"
		qtgenerateddir "dep/QDiscord/src/core/GeneratedFiles/"

		filter { "configurations:Debug" }
			qtsuffix "d"

		filter { "platforms:x64" }
			qtpath (qtDir)
	end
