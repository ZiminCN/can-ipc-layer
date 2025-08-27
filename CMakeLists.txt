cmake_minimum_required(VERSION 3.10)
project(can_ipc_layer)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# print debug message
message(STATUS "[CMake debug]: curent cmake source directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "[CMake debug]: current cmake build dircetory: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "[CMake debug]: cmake source directory: ${CMAKE_SOURCE_DIR}")
message(STATUS "[CMake debug]: cmake build directory: ${CMAKE_BINARY_DIR}")

# set default VERSION value
set(MAJOR_VERSION_DEFAULT 0)
set(MINOR_VERSION_DEFAULT 0)
set(CMAKE_BUILD_TYPE_DEFAULT "error")
set(AUTOUPDATE_PATCH_CNT_DEFAULT 5)
set(LAST_VERSION_GIT_COMMIT_CNT_DEFAULT 0)

# get VERSION value from CMakePresets.json
if(DEFINED MAJOR_VERSION)
    set(MAJOR_VERSION ${MAJOR_VERSION})
else()
    set(MAJOR_VERSION ${MAJOR_VERSION_DEFAULT})
endif()

if(DEFINED MINOR_VERSION)
    set(MINOR_VERSION ${MINOR_VERSION})
else()
    set(MINOR_VERSION ${MINOR_VERSION_DEFAULT})
endif()

if(DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})
else()
    set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_DEFAULT})
endif()

if(DEFINED AUTOUPDATE_PATCH_CNT)
    set(AUTOUPDATE_PATCH_CNT ${AUTOUPDATE_PATCH_CNT})
else()
    set(AUTOUPDATE_PATCH_CNT ${AUTOUPDATE_PATCH_CNT_DEFAULT})
endif()

if(DEFINED LAST_VERSION_GIT_COMMIT_CNT)
    set(LAST_VERSION_GIT_COMMIT_CNT ${LAST_VERSION_GIT_COMMIT_CNT})
else()
    set(LAST_VERSION_GIT_COMMIT_CNT ${LAST_VERSION_GIT_COMMIT_CNT_DEFAULT})
endif()

# Automatically generate version numbers by obtaining Git information.
find_package(Git)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_COUNT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_SHORT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    set(GIT_COMMIT_COUNT "0")
    set(GIT_SHORT_HASH "unknown")
endif()

set(VERSION_HEADER "${CMAKE_SOURCE_DIR}/generated/version.h")
if(EXISTS ${VERSION_HEADER})
    file(READ ${VERSION_HEADER} VERSION_CONTENT)
    string(REGEX MATCH "#define TWEAK_VERSION ([0-9]+)" _ ${VERSION_CONTENT})
    math(EXPR PATCH_VERSION "(${GIT_COMMIT_COUNT} - ${LAST_VERSION_GIT_COMMIT_CNT}) / ${AUTOUPDATE_PATCH_CNT}")
else()
    message(WARNING "last version is not exist: ${VERSION_HEADER}")
    set(PATCH_VERSION ${LAST_VERSION_GIT_COMMIT_CNT})
endif()

# Set the version with build number
set(PROJECT_VERSION_WITH_BUILD "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}.${GIT_COMMIT_COUNT}.${GIT_SHORT_HASH}")
set(PROJECT_VERSION_FOR_CMAKE "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
message(STATUS "Building type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Building version: ${PROJECT_VERSION_WITH_BUILD}")
project(can_ipc_layer VERSION ${PROJECT_VERSION_FOR_CMAKE})

configure_file(
    ${CMAKE_SOURCE_DIR}/version.h.in
    ${CMAKE_SOURCE_DIR}/generated/version.h
    @ONLY
)
include_directories(${CMAKE_SOURCE_DIR}/generated)

# find dynamic link library
list(APPEND CMAKE_LIBRARY_PATH /usr/hobot/lib)
find_library(HOBOT_CANHAL_LIB hbcanhal)
# check if find or not
if(HOBOT_CANHAL_LIB)
    message(STATUS "Found can_hal library: ${HOBOT_CANHAL_LIB}")
else()
    message(FATAL_ERROR "can_hal library not found!")
endif()

find_path(HOBOT_CANHAL_INCLUDE_DIR
    NAMES hobot/include/canhal/can_hal.h
)

# create dll
file(GLOB LIB_SOURCES 
    "src/*.c*"
    "src/utils/*.c*"
    "src/core/*.c*"
    "src/api/*.c*"
)

file(GLOB LIB_HEADERS
    "src/core/*.hpp"
    "src/utils/*.hpp"
)

file(GLOB LIB_INTERFACE_HEADERS
    "include/can_ipc_layer/*.h"
)

add_library(can_ipc_layer SHARED ${LIB_SOURCES})
target_include_directories(can_ipc_layer
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/can_ipc_layer>
        $<INSTALL_INTERFACE:include/can_ipc_layer>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/src/api
)

target_include_directories(can_ipc_layer PUBLIC include/can_ipc_layer)

target_link_libraries(can_ipc_layer PRIVATE ${HOBOT_CANHAL_LIB})

target_compile_options(can_ipc_layer PRIVATE -Wall -Wextra -Werror)

set_target_properties(can_ipc_layer PROPERTIES
    VERSION ${PROJECT_VERSION_FOR_CMAKE}
    SOVERSION ${MAJOR_VERSION}
)