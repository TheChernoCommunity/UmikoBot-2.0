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
	cppdialect "C++11"
	flags {
		"MultiProcessorCompile",
	}
	
	files {
		"src/**.cpp",
		"src/**.h",
	}
	includedirs {
		"src/",
	}
	links {
	}

	filter { "configurations:Release" }
		optimize "Full"
		defines {
		}

	filter { "platforms:x64" }
		debugdir "res/"
		objdir "obj/"
		targetdir "bin/"

	filter {"toolset:msc"}
		disablewarnings { "C4996" }

	filter {}
