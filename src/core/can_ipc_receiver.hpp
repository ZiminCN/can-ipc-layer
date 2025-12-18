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
#ifndef __CAN_IPC_RECEIVER_HPP__
#define __CAN_IPC_RECEIVER_HPP__
#include "can_struct_define.h"

#include "can_struct_internal_define.hpp"
#include "message_log.hpp"
#include "work_queue.hpp"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

/**
 * @brief can receiver filter struct
 * @note hash table key: can id
 * @note hash table value: callback function (only one callback function is allowed for each can id)
 */
typedef struct {
	uint32_t *can_filter_id;
	uint32_t can_filter_id_cnt;
	CAN_DEV_PORT_E can_port;
	int socket_index;
	can_rx_callback_t can_filter_callback;
} CAN_IPC_FILTER_T;

typedef struct {
	CAN_DEV_PORT_E can_port;
	// can id hash table: can id(key value) + can id count
	// can filter hash table: can id(key value) + CAN_IPC_FILTER_T(include callback
	// func interface)
	std::unordered_map<uint32_t, int> can_ipc_filter_id_map;
	std::unordered_map<uint32_t, CAN_IPC_FILTER_T> can_ipc_filter_map;
} CAN_IPC_RECEIVER_FILTER_T;

struct canframe;

class CAN_IPC_RECEIVER
{
      public:
	CAN_IPC_RECEIVER()
	{
		LOG_DEBUG("CAN_IPC_RECEIVER impl init.");
		this->init_can_filter();
	};
	~CAN_IPC_RECEIVER()
	{
		this->receive_ipc_can_task_paused_.store(true);
		this->work_task_queue_paused_.store(true);
		this->receive_ipc_can_task_running_.store(false);
		this->work_task_queue_running_.store(false);
	};
	CAN_IPC_RECEIVER(const CAN_IPC_RECEIVER &) = delete;
	CAN_IPC_RECEIVER &operator=(const CAN_IPC_RECEIVER &) = delete;
	static std::unique_ptr<CAN_IPC_RECEIVER> &getInstance()
	{
		static std::unique_ptr<CAN_IPC_RECEIVER> Instance =
			std::make_unique<CAN_IPC_RECEIVER>();
		return Instance;
	};
	int register_can_filter(const CAN_IPC_FILTER_T &can_ipc_filter);
	int deregister_can_filter(const CAN_IPC_FILTER_T &can_ipc_filter);
	void start_can_ipc_receiver();
	void pause_can_ipc_receiver();
	void resume_can_ipc_receiver();
	void enable_can_receiver_port(const CAN_IPC_CONFIG_T *can_ipc_config);

      private:
	static inline std::atomic<bool> receive_ipc_can_task_running_{false};
	static inline std::atomic<bool> work_task_queue_running_{false};
	static inline std::atomic<bool> receive_ipc_can_task_paused_{true};
	static inline std::atomic<bool> work_task_queue_paused_{true};

	static inline std::atomic<bool> is_received_can_5_port{false};
	static inline std::atomic<bool> is_received_can_6_port{false};
	static inline std::atomic<bool> is_received_can_7_port{false};
	static inline std::atomic<bool> is_received_can_8_port{false};
	static inline std::atomic<bool> is_received_can_9_port{false};

	static std::unique_ptr<CAN_IPC_CONFIG_T> &get_can_ipc_port_5_instance()
	{
		static std::unique_ptr<CAN_IPC_CONFIG_T> instance =
			std::make_unique<CAN_IPC_CONFIG_T>();
		return instance;
	}

	static std::unique_ptr<CAN_IPC_CONFIG_T> &get_can_ipc_port_6_instance()
	{
		static std::unique_ptr<CAN_IPC_CONFIG_T> instance =
			std::make_unique<CAN_IPC_CONFIG_T>();
		return instance;
	}

	static std::unique_ptr<CAN_IPC_CONFIG_T> &get_can_ipc_port_7_instance()
	{
		static std::unique_ptr<CAN_IPC_CONFIG_T> instance =
			std::make_unique<CAN_IPC_CONFIG_T>();
		return instance;
	}

	static std::unique_ptr<CAN_IPC_CONFIG_T> &get_can_ipc_port_8_instance()
	{
		static std::unique_ptr<CAN_IPC_CONFIG_T> instance =
			std::make_unique<CAN_IPC_CONFIG_T>();
		return instance;
	}

	static std::unique_ptr<CAN_IPC_CONFIG_T> &get_can_ipc_port_9_instance()
	{
		static std::unique_ptr<CAN_IPC_CONFIG_T> instance =
			std::make_unique<CAN_IPC_CONFIG_T>();
		return instance;
	}

	// static std::thread work_queue_task_;
	// static std::thread receive_ipc_can_task_;

	static std::thread &get_work_queue_task()
	{
		static std::thread work_queue_task_;
		return work_queue_task_;
	}

	static std::thread &get_receive_ipc_can_task()
	{
		static std::thread receive_ipc_can_task_;
		return receive_ipc_can_task_;
	}

	mutable std::mutex receive_ipc_can_task_paused_mutex_;
	mutable std::mutex work_task_queue_paused_mutex_;
	std::condition_variable receive_ipc_can_task_paused_condition_;
	std::condition_variable work_task_queue_paused_condition_;

	std::unique_ptr<WORK_QUEUE> &work_queue_handle = WORK_QUEUE::getInstance();
	static std::unique_ptr<CAN_IPC_RECEIVER_FILTER_T> &get_can_ipc_receiver_port_5_filter()
	{
		static std::unique_ptr<CAN_IPC_RECEIVER_FILTER_T> instance =
			std::make_unique<CAN_IPC_RECEIVER_FILTER_T>();
		return instance;
	}

	static std::unique_ptr<CAN_IPC_RECEIVER_FILTER_T> &get_can_ipc_receiver_port_6_filter()
	{
		static std::unique_ptr<CAN_IPC_RECEIVER_FILTER_T> instance =
			std::make_unique<CAN_IPC_RECEIVER_FILTER_T>();
		return instance;
	}

	static std::unique_ptr<CAN_IPC_RECEIVER_FILTER_T> &get_can_ipc_receiver_port_7_filter()
	{
		static std::unique_ptr<CAN_IPC_RECEIVER_FILTER_T> instance =
			std::make_unique<CAN_IPC_RECEIVER_FILTER_T>();
		return instance;
	}

	static std::unique_ptr<CAN_IPC_RECEIVER_FILTER_T> &get_can_ipc_receiver_port_8_filter()
	{
		static std::unique_ptr<CAN_IPC_RECEIVER_FILTER_T> instance =
			std::make_unique<CAN_IPC_RECEIVER_FILTER_T>();
		return instance;
	}

	static std::unique_ptr<CAN_IPC_RECEIVER_FILTER_T> &get_can_ipc_receiver_port_9_filter()
	{
		static std::unique_ptr<CAN_IPC_RECEIVER_FILTER_T> instance =
			std::make_unique<CAN_IPC_RECEIVER_FILTER_T>();
		return instance;
	}

	void init_can_filter();
	void clear_hash_table(CAN_IPC_RECEIVER_FILTER_T &can_ipc_receiver_filter);
	int match_can_filter(CAN_DEV_PORT_E can_port, const struct can_frame_t raw_can_frame);
	CAN_IPC_RECEIVER_FILTER_T *return_can_ipc_receiver_filter(const CAN_DEV_PORT_E can_port);
	void work_queue_task();
	void receive_ipc_can_task();
	void create_work_queue_task();
	void create_receive_ipc_can_task();
	void pause_work_queue_task();
	void pause_receive_ipc_can_task();
	void resume_work_queue_task();
	void resume_receive_ipc_can_task();
	void get_raw_can_data(const can_port_target_t &can_port_target);
};

#endif // __CAN_IPC_RECEIVER_HPP__