cmake_minimum_required(VERSION 3.1)
if (CMAKE_VERSION VERSION_LESS "3.7.0")
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

project(UmikoBot VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Runs Qt stuff
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Dependencies
find_package(Qt5 COMPONENTS Core Gui Network WebSockets Widgets)
add_subdirectory(dep/QDiscord/)

# Files
set(SOURCES
	main.cpp UmikoBot.cpp
	core/Data.cpp core/InterruptHandler.cpp core/Permissions.cpp
	modules/CurrencyModule.cpp modules/FunModule.cpp modules/GeneralModule.cpp modules/LevelModule.cpp modules/ModerationModule.cpp modules/Module.cpp modules/UserModule.cpp)
list(TRANSFORM SOURCES PREPEND src/)

# The executable
add_executable(UmikoBot ${SOURCES})
target_link_libraries(UmikoBot Qt5::Core Qt5::Gui Qt5::Network Qt5::WebSockets Qt5::Widgets)
target_link_libraries(UmikoBot QDiscord)
target_include_directories(UmikoBot PUBLIC src/)

# Copies DLLs on Windows
if (WIN32)
	set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT UmikoBot)
	
	add_custom_command(TARGET UmikoBot POST_BUILD COMMAND
		${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE:Qt5::Core>
		$<TARGET_FILE:Qt5::Gui>
		$<TARGET_FILE:Qt5::Network>
		$<TARGET_FILE:Qt5::WebSockets>
		$<TARGET_FILE:Qt5::Widgets>

		${CMAKE_PREFIX_PATH}/plugins/platforms/qwindows.dll
		${CMAKE_PREFIX_PATH}/plugins/platforms/qwindowsd.dll
		${CMAKE_PREFIX_PATH}/../../Tools/OpenSSL/Win_x64/bin/libcrypto-1_1-x64.dll
		${CMAKE_PREFIX_PATH}/../../Tools/OpenSSL/Win_x64/bin/libssl-1_1-x64.dll

		$<TARGET_FILE_DIR:UmikoBot>
	)
endif()
