cmake_minimum_required(VERSION 3.1)

project(UmikoBot VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Runs Qt stuff
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (CMAKE_VERSION VERSION_LESS "3.7.0")
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Core Gui Network WebSockets Widgets)

set(SOURCES
	main.cpp UmikoBot.cpp
	core/Data.cpp core/InterruptHandler.cpp core/Permissions.cpp
	modules/CurrencyModule.cpp modules/FunModule.cpp modules/GeneralModule.cpp modules/LevelModule.cpp modules/ModerationModule.cpp modules/Module.cpp modules/UserModule.cpp)
list(TRANSFORM SOURCES PREPEND src/)

add_executable(UmikoBot ${SOURCES})
target_link_libraries(UmikoBot Qt5::Core Qt5::Gui Qt5::Network Qt5::WebSockets Qt5::Widgets)
