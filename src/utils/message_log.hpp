#pragma once

#ifndef __MESSAGE_LOG_HPP__
#define __MESSAGE_LOG_HPP__

#include <iostream>
#include <sstream>

// 条件包含 systemd
#ifdef __linux__
#if __has_include(<systemd/sd-journal.h>)
#include <systemd/sd-journal.h>
#define HAS_SYSTEMD_JOURNAL 1
#else
#define HAS_SYSTEMD_JOURNAL 0
#endif
#else
#define HAS_SYSTEMD_JOURNAL 0
#endif

// 从编译定义中获取日志级别
#ifndef CAN_IPC_LOG_LEVEL
#define CAN_IPC_LOG_LEVEL 2 // 不要修改这里，修改LOG等级需要在CMake里修改CAN_IPC_LOG_LEVEL参数
#endif

#ifndef CAN_IPC_LOG_AS_JOURNAL
#define CAN_IPC_LOG_AS_JOURNAL 0
#endif

// 防止与其他库的宏冲突
#ifdef LOG_DEBUG
#undef LOG_DEBUG
#endif
#ifdef LOG_INFO
#undef LOG_INFO
#endif
#ifdef LOG_WARNING
#undef LOG_WARNING
#endif
#ifdef LOG_ERROR
#undef LOG_ERROR
#endif

// 日志级别枚举
enum CANIPCLogLevel {
	CAN_IPC_LOG_NONE = 0,	 // 默认缺省
	CAN_IPC_LOG_DEBUG = 1,	 // 输出调试信息及以上
	CAN_IPC_LOG_INFO = 2,	 // 输出一般信息及以上
	CAN_IPC_LOG_WARNING = 3, // 输出警告信息以及上
	CAN_IPC_LOG_ERROR = 4	 // 输出错误信息
};

constexpr CANIPCLogLevel CURRENT_IPC_LOG_LEVEL = static_cast<CANIPCLogLevel>(CAN_IPC_LOG_LEVEL);

#if CAN_IPC_LOG_AS_JOURNAL && HAS_SYSTEMD_JOURNAL

// 使用 systemd journal 的版本 - 修复可变参数处理
#define CAN_IPC_LOG(log_level, ...)                                                                \
	do {                                                                                       \
		if (CURRENT_IPC_LOG_LEVEL == 0)                                                    \
			break;                                                                     \
		if (log_level >= CURRENT_IPC_LOG_LEVEL) {                                          \
			int sd_level;                                                              \
			const char *prefix;                                                        \
			switch (log_level) {                                                       \
			case CAN_IPC_LOG_DEBUG:                                                    \
				sd_level = LOG_DEBUG;                                              \
				prefix = "[CAN_IPC][Debug]";                                       \
				break;                                                             \
			case CAN_IPC_LOG_INFO:                                                     \
				sd_level = LOG_INFO;                                               \
				prefix = "[CAN_IPC][Info]";                                        \
				break;                                                             \
			case CAN_IPC_LOG_WARNING:                                                  \
				sd_level = LOG_WARNING;                                            \
				prefix = "[CAN_IPC][Warning]";                                     \
				break;                                                             \
			case CAN_IPC_LOG_ERROR:                                                    \
				sd_level = LOG_ERR;                                                \
				prefix = "[CAN_IPC][Error]";                                       \
				break;                                                             \
			default:                                                                   \
				sd_level = LOG_INFO;                                               \
				prefix = "[CAN_IPC]";                                              \
				break;                                                             \
			}                                                                          \
			/* 使用 sd_journal_print 的可变参数版本 */                        \
			sd_journal_print(sd_level, "%s " __VA_ARGS__, prefix);                     \
		}                                                                                  \
	} while (0)

#else

// 使用标准输出的版本（原版本）
#define CAN_IPC_LOG(log_level, ...)                                                                \
	do {                                                                                       \
		if (CURRENT_IPC_LOG_LEVEL == 0)                                                    \
			break;                                                                     \
		if (log_level >= CURRENT_IPC_LOG_LEVEL) {                                          \
			std::ostringstream oss;                                                    \
			oss << __VA_ARGS__;                                                        \
			switch (log_level) {                                                       \
			case CAN_IPC_LOG_DEBUG:                                                    \
				std::cout << "[CAN_IPC][Debug]: " << oss.str() << std::endl;       \
				break;                                                             \
			case CAN_IPC_LOG_INFO:                                                     \
				std::cout << "[CAN_IPC][Info]: " << oss.str() << std::endl;        \
				break;                                                             \
			case CAN_IPC_LOG_WARNING:                                                  \
				std::cout << "[CAN_IPC][WARNING]: " << oss.str() << std::endl;     \
				break;                                                             \
			case CAN_IPC_LOG_ERROR:                                                    \
				std::cout << "[CAN_IPC][ERROR]: " << oss.str() << std::endl;       \
				break;                                                             \
			default:                                                                   \
				break;                                                             \
			}                                                                          \
		}                                                                                  \
	} while (0)

#endif

#ifndef LOG_DEBUG
#define LOG_DEBUG(...) CAN_IPC_LOG(CAN_IPC_LOG_DEBUG, ##__VA_ARGS__)
#endif

#ifndef LOG_INFO
#define LOG_INFO(...) CAN_IPC_LOG(CAN_IPC_LOG_INFO, ##__VA_ARGS__)
#endif

#ifndef LOG_WARNING
#define LOG_WARNING(...) CAN_IPC_LOG(CAN_IPC_LOG_WARNING, ##__VA_ARGS__)
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(...) CAN_IPC_LOG(CAN_IPC_LOG_ERROR, ##__VA_ARGS__)
#endif

#endif // __MESSAGE_LOG_HPP__