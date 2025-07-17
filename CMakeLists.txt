cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(GEODE_DISABLE_PRECOMPILED_HEADERS ON)
set(GEODE_CODEGEN_EXTRA_ARGS "--skip-pugixml")

project(MoreIcons VERSION 1.14.5)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE include)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

CPMAddPackage("gh:zeux/pugixml@1.15")
CPMAddPackage("gh:hiimjasmine00/texpack@0.4.0")
if (WIN32)
    target_compile_definitions(pugixml-static PRIVATE _HAS_ITERATOR_DEBUGGING=0)
    target_compile_definitions(texpack PRIVATE _HAS_ITERATOR_DEBUGGING=0)
endif()
target_link_libraries(${PROJECT_NAME} pugixml-static texpack)

setup_geode_mod(${PROJECT_NAME})
