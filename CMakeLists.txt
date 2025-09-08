# Copyright (c) Direct Drive Technology Co., Ltd. All rights reserved.
# Author: Zi Min <jianming.zeng@directdrivetech.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.20)
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
set(COMPILER_OPT_CONF_DEFAULT "O0")
set(LOG_TYPE_DEFAULT "Debug")

# 只允许特定的构建类型
set(ALLOWED_BUILD_TYPES Debug Release)
if(NOT CMAKE_BUILD_TYPE IN_LIST ALLOWED_BUILD_TYPES)
    message(FATAL_ERROR "不支持的构建类型: ${CMAKE_BUILD_TYPE}。允许的类型: ${ALLOWED_BUILD_TYPES}")
endif()

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

if(DEFINED COMPILER_OPT_CONF)
    set(COMPILER_OPT_CONF ${COMPILER_OPT_CONF})
else()
    set(COMPILER_OPT_CONF ${COMPILER_OPT_CONF_DEFAULT})
endif()

# 强制要求使用 preset
# if(${CMAKE_BUILD_TYPE})
if(NOT CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "
    ============================================================
    ERROR: CMake preset is mandatory for this project!

    Please configure using one of the available presets:

        cmake .. --preset=dev-build     - Development Build
        cmake .. --preset=release-1.0   - Release 1.0.0

    To list all available presets:
        cmake .. --list-presets
    ============================================================
    ")
endif()

if(DEFINED LOG_TYPE)
    set(LOG_TYPE ${LOG_TYPE})
else()
    set(LOG_TYPE ${LOG_TYPE_DEFAULT})
endif()

set(CAN_IPC_LOG_LEVEL 0)

if(LOG_TYPE STREQUAL "None")
    set(CAN_IPC_LOG_LEVEL 0)
elseif(LOG_TYPE STREQUAL "Debug")
    set(CAN_IPC_LOG_LEVEL 1)
elseif(LOG_TYPE STREQUAL "Info")
    set(CAN_IPC_LOG_LEVEL 2)
elseif(LOG_TYPE STREQUAL "Warning")
    set(CAN_IPC_LOG_LEVEL 3)
elseif(LOG_TYPE STREQUAL "Error")
    set(CAN_IPC_LOG_LEVEL 4)
else()
    set(CAN_IPC_LOG_LEVEL 0)
endif()

message(STATUS "CAN_IPC_LOG_LEVEL is: ${CAN_IPC_LOG_LEVEL}")

add_compile_definitions(CAN_IPC_LOG_LEVEL=${CAN_IPC_LOG_LEVEL})

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
find_library(HOBOT_IPCFHAL_LIB hbipcfhal)
# check if find or not
if(HOBOT_CANHAL_LIB)
    message(STATUS "Found can_hal library: ${HOBOT_CANHAL_LIB}")
else()
    message(FATAL_ERROR "can_hal library not found!")
endif()

if(HOBOT_IPCFHAL_LIB)
    message(STATUS "Found ipcf_hal library: ${HOBOT_IPCFHAL_LIB}")
else()
    message(FATAL_ERROR "ipcf_hal library not found!")
endif()

find_path(HOBOT_CANHAL_INCLUDE_DIR
    NAMES can_hal.h
    PATHS /usr/hobot/include/canhal/
    DOC "Path to Hobot CAN HAL include directory"
)

find_path(HOBOT_IPCFHAL_INCLUDE_DIR
    NAMES hb_ipcf_hal.h ipcf_hal_errno.h
    PATHS /usr/hobot/include/
    DOC "Path to Hobot IPCF HAL include directory"
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
        ${HOBOT_CANHAL_INCLUDE_DIR}
        ${HOBOT_IPCFHAL_INCLUDE_DIR}
)

add_compile_options(-${COMPILER_OPT_CONF})

target_include_directories(can_ipc_layer PUBLIC include/can_ipc_layer)

target_link_libraries(can_ipc_layer PRIVATE ${HOBOT_CANHAL_LIB})

target_compile_options(can_ipc_layer PRIVATE -Wall -Wextra -Werror)

set_target_properties(can_ipc_layer PROPERTIES
    VERSION ${PROJECT_VERSION_FOR_CMAKE}
    SOVERSION ${MAJOR_VERSION}
)