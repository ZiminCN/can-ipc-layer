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

#include "can_ipc_layer.h"
#include "ret_code_def.h"

#include "can_ipc_layer_impl.hpp"
#include "message_log.hpp"
#include <iostream>
#include <memory>

// 定义不透明句柄的实际内容
struct can_ipc_layer_handle_t {
	std::unique_ptr<CAN_IPC_LAYER_IMPL> impl;
	std::string impl_last_error = "None";

	can_ipc_layer_handle_t(std::unique_ptr<CAN_IPC_LAYER_IMPL> ptr) : impl(std::move(ptr))
	{
	}

	~can_ipc_layer_handle_t()
	{
		LOG_DEBUG("can_ipc_layer_handle_t impl_last_error is " << impl_last_error);
	}

	// 设置错误信息
	void set_error(const std::string &error)
	{
		impl_last_error = error;
		std::cerr << "Error: " << error << std::endl;
	}
};

// 线程局部的错误信息
thread_local std::string can_ipc_layer_last_error;

// C接口实现
extern "C" {

// 兼容C接口，需要显式分配
can_ipc_layer_handle_t *can_ipc_layer_create()
{
	try {
		LOG_DEBUG("can_ipc_layer_create.");
		std::unique_ptr<CAN_IPC_LAYER_IMPL> impl = CAN_IPC_LAYER_IMPL::getInstance();
		return new can_ipc_layer_handle_t(std::move(impl));
	} catch (const std::exception &e) {
		can_ipc_layer_last_error = e.what();
		return nullptr;
	}
}

// 兼容C接口，需要显式销毁
void can_ipc_layer_destroy(can_ipc_layer_handle_t **handle)
{
	if (handle && *handle) {
		delete *handle;
		*handle = nullptr;
		LOG_DEBUG("can_ipc_layer_destroy.");
	}
}

int can_send(can_ipc_layer_handle_t **handle, const CAN_PORT_E can_port, const can_frame_t *frame)
{
	if (handle && *handle) {
		int ret = (*handle)->impl->lib_can_send(can_port, *frame);
		return ret;
	}

	LOG_ERROR("can_send error.");
	return -RET_CODE_INVALID_ARG;
}

int can_add_filter(can_ipc_layer_handle_t **handle, const can_filter_t *can_filter,
		   const can_rx_callback_t can_rx_callback)
{
	if (handle && *handle) {
		int ret =
			(*handle)->impl->lib_can_register_can_filter(*can_filter, can_rx_callback);
		return ret;
	}

	LOG_ERROR("can_add_filter error.");
	return -RET_CODE_INVALID_ARG;
}

int can_remove_filter(can_ipc_layer_handle_t **handle, const can_filter_t *can_filter)
{
	if (handle && *handle) {
		int ret = (*handle)->impl->lib_can_deregister_can_filter(*can_filter);
		return ret;
	}

	LOG_ERROR("can_remove_filter error.");
	return -RET_CODE_INVALID_ARG;
}

void can_ipc_receiver_port_enable(can_ipc_layer_handle_t **handle, const CAN_PORT_E can_port)
{
	if (handle && *handle) {
		(*handle)->impl->lib_enable_can_receiver_port(can_port);
	}
}

void can_ipc_receiver_start(can_ipc_layer_handle_t **handle)
{
	if (handle && *handle) {
		(*handle)->impl->lib_start_can_ipc_receiver();
	}
}

void can_ipc_receiver_pause(can_ipc_layer_handle_t **handle)
{
	if (handle && *handle) {
		(*handle)->impl->lib_pause_can_ipc_receiver();
	}
}

void can_ipc_receiver_resume(can_ipc_layer_handle_t **handle)
{
	if (handle && *handle) {
		(*handle)->impl->lib_resume_can_ipc_receiver();
	}
}

//! test func
void test_can_send(can_ipc_layer_handle_t **handle)
{
	if (handle && *handle) {
		(*handle)->impl->lib_test_can_send();
	}
}

} // extern "C"
