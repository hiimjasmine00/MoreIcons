cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(MoreIcons VERSION 1.6.0)

add_library(${PROJECT_NAME} SHARED
    src/api/MoreIconsAPI.cpp
    src/classes/ButtonHooker.cpp
    src/classes/LogLayer.cpp
    src/hooks/AppDelegate.cpp
    src/hooks/CharacterColorPage.cpp
    src/hooks/GameManager.cpp
    src/hooks/GJBaseGameLayer.cpp
    src/hooks/GJGarageLayer.cpp
    src/hooks/GJRobotSprite.cpp
    src/hooks/LoadingLayer.cpp
    src/hooks/MenuGameLayer.cpp
    src/hooks/MenuLayer.cpp
    src/hooks/PlayerObject.cpp
    src/hooks/ProfilePage.cpp
    src/hooks/SimplePlayer.cpp
    src/MoreIcons.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE include)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})
