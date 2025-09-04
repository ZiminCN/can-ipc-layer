// Copyright (c) Direct Drive Technology Co., Ltd. All rights reserved.
// Author: Zi Min <jianming.zeng@directdrivetech.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#ifndef __MESSAGE_LOG_HPP__
#define __MESSAGE_LOG_HPP__

#include <iostream>
#include <sstream>
// 从编译定义中获取日志级别
#ifndef CAN_IPC_LOG_LEVEL
#define CAN_IPC_LOG_LEVEL 1
#endif

// 日志级别枚举
enum CANIPCLogLevel {
    CAN_IPC_LOG_NONE = 0,    // 调试信息
    CAN_IPC_LOG_DEBUG = 1,    // 调试信息
    CAN_IPC_LOG_INFO = 2,     // 一般信息
    CAN_IPC_LOG_WARNING = 3,  // 警告信息
    CAN_IPC_LOG_ERROR = 4     // 错误信息
};

constexpr CANIPCLogLevel CURRENT_IPC_LOG_LEVEL = static_cast<CANIPCLogLevel>(CAN_IPC_LOG_LEVEL);

#define CAN_IPC_LOG(log_level, ...)                                                                 \
        do{                                                                                             \
                if(CURRENT_IPC_LOG_LEVEL == 0){                                                         \
                        break;                                                                          \
                }                                                                                       \
                if(log_level >= CURRENT_IPC_LOG_LEVEL){                                                 \
                        std::ostringstream oss;                                                         \
                        oss << __VA_ARGS__;                                                             \
                        switch(log_level){                                                              \
                                case CAN_IPC_LOG_DEBUG:{                                                \
                                        std::cout << "[CAN_IPC][Debug]: " << oss.str() << std::endl;    \
                                        break;                                                          \
                                }                                                                       \
                                case CAN_IPC_LOG_INFO:{                                                 \
                                        std::cout << "[CAN_IPC][Info]: " << oss.str() << std::endl;       \
                                        break;                                                          \
                                }                                                                       \
                                case CAN_IPC_LOG_WARNING:{                                              \
                                        std::cout << "[CAN_IPC][WARNING]: " << oss.str() << std::endl;    \
                                        break;                                                          \
                                }                                                                       \
                                case CAN_IPC_LOG_ERROR:{                                                \
                                        std::cout << "[CAN_IPC][ERROR]: " << oss.str() << std::endl;      \
                                        break;                                                          \
                                }                                                                       \
                                default:{                                                               \
                                        break;                                                          \
                                }                                                                       \
                        }                                                                               \
                }                                                                                       \
        }while(0)                                                                                       \

#define LOG_DEBUG(...) CAN_IPC_LOG(CAN_IPC_LOG_DEBUG, ##__VA_ARGS__)
#define LOG_INFO(...) CAN_IPC_LOG(CAN_IPC_LOG_INFO, ##__VA_ARGS__)
#define LOG_WARNING(...) CAN_IPC_LOG(CAN_IPC_LOG_WARNING, ##__VA_ARGS__)
#define LOG_ERROR(...) CAN_IPC_LOG(CAN_IPC_LOG_ERROR, ##__VA_ARGS__)

#endif // __MESSAGE_LOG_HPP__
