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
project(debug_can_ipc_layer)

# 强制要求用户指定构建类型
if(NOT CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "请指定构建类型: -DCMAKE_BUILD_TYPE=Local 或 -DCMAKE_BUILD_TYPE=System")
endif()

# 只允许特定的构建类型
set(ALLOWED_BUILD_TYPES Local System)
if(NOT CMAKE_BUILD_TYPE IN_LIST ALLOWED_BUILD_TYPES)
    message(FATAL_ERROR "不支持的构建类型: ${CMAKE_BUILD_TYPE}。允许的类型: ${ALLOWED_BUILD_TYPES}")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wfatal-errors) # GCC 在第一次错误时停止

# 打印调试信息
message(STATUS "当前debug测试目录: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "项目debug根目录: ${CMAKE_SOURCE_DIR}")
message(STATUS "构建debug目录: ${CMAKE_BINARY_DIR}")

# 检查并标准化
if(CMAKE_BUILD_TYPE STREQUAL "Local")
        # 查找Local动态链接库
        # Local是动态库直接在build文件夹下cmake出来的产物，方便调试
        find_library(TEST_CAN_IPC_LIB test_driver
                PATHS ${CMAKE_SOURCE_DIR}/../build
                NO_DEFAULT_PATH
                REQUIRED
        )

        message(STATUS "库文件位置: ${TEST_CAN_IPC_LIB}")
        add_executable(debug_can_ipc_layer local_main.c)
        # 添加动态库头文件目录
        target_include_directories(debug_can_ipc_layer PRIVATE
                ${CMAKE_SOURCE_DIR}/../include
        )
        # 链接动态库
        target_link_libraries(debug_can_ipc_layer ${TEST_CAN_IPC_LIB})

        # 设置运行时库搜索路径（避免每次设置LD_LIBRARY_PATH）
        set_target_properties(debug_can_ipc_layer PROPERTIES
                BUILD_RPATH "${CMAKE_SOURCE_DIR}/../build"
                INSTALL_RPATH "${CMAKE_SOURCE_DIR}/../build"
        )

        target_compile_options(debug_can_ipc_layer PRIVATE -Werror)

elseif(BUILD_TYPE_LOWER STREQUAL "System")
        # 查找System动态链接库
        # System是动态库被部署到系统层/usr/层里面的产物
        message(FATAL_ERROR "There is no System build function.")
endif()

