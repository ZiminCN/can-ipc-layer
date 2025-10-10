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

#ifndef __CAN_IPC_RECEIVER_HPP__
#define __CAN_IPC_RECEIVER_HPP__
#include "can_struct_define.h"

#include "message_log.hpp"
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

/**
 * @brief can receiver filter struct
 * @note hash table key: can id
 * @note hash table value: callback function (only one callback function is allowed for each can id)
 */
typedef struct {
	uint32_t can_filter_id;
	uint32_t can_filter_mask;
	can_rx_callback_t can_filter_callback;
} CAN_IPC_RECEIVER_FILTER_T;

class CAN_IPC_RECEIVER
{
      public:
	CAN_IPC_RECEIVER()
	{
		LOG_DEBUG("CAN_IPC_RECEIVER impl init.");
		this->init_can_filter();
	};
	~CAN_IPC_RECEIVER() = default;
	static std::unique_ptr<CAN_IPC_RECEIVER> getInstance();
	int register_can_filter(const CAN_IPC_RECEIVER_FILTER_T &can_ipc_receiver_filter);
	int deregister_can_filter(const CAN_IPC_RECEIVER_FILTER_T &can_ipc_receiver_filter);

      private:
	static std::unique_ptr<CAN_IPC_RECEIVER> Instance;

	// can mask hash table: can mask(key value) + can mask count
	// can id hash table: can id(key value) + can id count
	// can filter hash table: can id(key value) + CAN_IPC_RECEIVER_FILTER_T(include callback
	// func interface)
	static inline std::unordered_map<uint32_t, int> can_ipc_filter_mask_map{};
	static inline std::unordered_map<uint32_t, int> can_ipc_filter_id_map{};
	static inline std::unordered_map<uint32_t, CAN_IPC_RECEIVER_FILTER_T> can_ipc_filter_map{};

	void init_can_filter();
	int match_can_filter(uint32_t raw_can_id);
};

#endif // __CAN_IPC_RECEIVER_HPP__