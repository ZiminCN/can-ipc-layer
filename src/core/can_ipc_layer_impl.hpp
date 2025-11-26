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
#ifndef __CAN_IPC_LAYER_IMPL_HPP__
#define __CAN_IPC_LAYER_IMPL_HPP__
#include "can_struct_define.h"

#include "can_ipc_receiver.hpp"
#include "can_ipc_sender.hpp"
#include "can_struct_internal_define.hpp"
#include "ipc_box_controller.hpp"
#include "message_log.hpp"
#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>

// Make sure all instances are completely closed before explicitly closing candev.

class CAN_IPC_LAYER_IMPL
{
      public:
	CAN_IPC_LAYER_IMPL()
	{
		std::lock_guard<std::mutex> lock(this->can_dev_mutex);
		if ((this->can_dev_ref_count)++ == 0) {
			LOG_DEBUG("CAN_IPC_LAYER_IMPL init can dev.");
			this->init_can_dev();
		}
	};
	~CAN_IPC_LAYER_IMPL()
	{
		std::lock_guard<std::mutex> lock(this->can_dev_mutex);
		if (--(can_dev_ref_count) == 0) {
			LOG_DEBUG("CAN_IPC_LAYER_IMPL deinit can dev.");
			this->deinit_can_dev();
		}
	};
	CAN_IPC_LAYER_IMPL(const CAN_IPC_LAYER_IMPL &) = delete;
	CAN_IPC_LAYER_IMPL &operator=(const CAN_IPC_LAYER_IMPL &) = delete;
	static std::unique_ptr<CAN_IPC_LAYER_IMPL> getInstance();

	void lib_test_can_send();
	int lib_can_send(const CAN_PORT_E can_port, const can_frame_t &frame);
	int lib_can_register_can_filter(const can_filter_t &can_filter,
					const can_rx_callback_t &can_rx_callback);
	int lib_can_deregister_can_filter(const can_filter_t &can_filter);

	void lib_enable_can_receiver_port(const CAN_PORT_E can_port);
	void lib_start_can_ipc_receiver();
	void lib_pause_can_ipc_receiver();
	void lib_resume_can_ipc_receiver();

      private:
	static std::unique_ptr<CAN_IPC_LAYER_IMPL> Instance;
	mutable std::mutex can_dev_mutex;
	static inline std::atomic<int> can_dev_ref_count{0};
	std::unique_ptr<CAN_IPC_RECEIVER> &can_ipc_receiver_handle =
		CAN_IPC_RECEIVER::getInstance();
	std::unique_ptr<CAN_IPC_SENDER> &can_ipc_sender_handle = CAN_IPC_SENDER::getInstance();
	std::unique_ptr<IPC_BOX_CONTROLLER> &ipc_box_controller_handle =
		IPC_BOX_CONTROLLER::getInstance();

	std::unique_ptr<CAN_IPC_CONFIG_T> can_ipc_config = std::make_unique<CAN_IPC_CONFIG_T>();
	void init_can_dev();
	void deinit_can_dev();
	CAN_DEV_PORT_E lib_get_can_dev_port(const CAN_PORT_E can_port);

	CAN_IPC_CONFIG_T lib_set_can_ipc_handle(CAN_PORT_E can_port);
	bool copy_ipc_config_data();
};

#endif // __CAN_IPC_LAYER_IMPL_HPP__