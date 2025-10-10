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

#include "can_ipc_layer_impl.hpp"

#include <hobot_can_hal.h>

std::unique_ptr<CAN_IPC_LAYER_IMPL> CAN_IPC_LAYER_IMPL::Instance =
	std::make_unique<CAN_IPC_LAYER_IMPL>();

std::unique_ptr<CAN_IPC_LAYER_IMPL> CAN_IPC_LAYER_IMPL::getInstance()
{
	return std::move(CAN_IPC_LAYER_IMPL::Instance);
}

void CAN_IPC_LAYER_IMPL::init_can_dev()
{
	// init can api
	int ret = 0;
	ret = canInit();
	if (ret < 0) {
		LOG_ERROR("canInit error!"
			  << " return value is [" << ret << "].");
	}
}

void CAN_IPC_LAYER_IMPL::deinit_can_dev()
{
	canDeInit();
}

void CAN_IPC_LAYER_IMPL::lib_test_can_send()
{
	this->can_ipc_sender_handle->test_send_can_frame();
}

CAN_DEV_PORT_E CAN_IPC_LAYER_IMPL::lib_get_can_dev_port(const CAN_PORT_E can_port)
{
	switch(can_port){
		case CAN_PORT_E::CAN_PORT_5:{
			return CAN_DEV_PORT_E::CAN_DEV_PORT_5;
			break;
		}
		case CAN_PORT_E::CAN_PORT_6:{
			return CAN_DEV_PORT_E::CAN_DEV_PORT_5;
			break;
		}
		case CAN_PORT_E::CAN_PORT_7:{
			return CAN_DEV_PORT_E::CAN_DEV_PORT_5;
			break;
		}
		case CAN_PORT_E::CAN_PORT_8:{
			return CAN_DEV_PORT_E::CAN_DEV_PORT_5;
			break;
		}
		case CAN_PORT_E::CAN_PORT_9:{
			return CAN_DEV_PORT_E::CAN_DEV_PORT_5;
			break;
		}
		default:{
			LOG_WARNING("Invalid CAN Port, use CAN Port 5 as default.");
			return CAN_DEV_PORT_E::CAN_DEV_PORT_5;
			break;
		}
	}

	return CAN_DEV_PORT_E::CAN_DEV_PORT_5;
}

CAN_IPC_CONFIG_T CAN_IPC_LAYER_IMPL::lib_set_can_ipc_handle(const CAN_PORT_E can_port)
{
	CAN_IPC_CONFIG_T temp_can_ipc_instance_impl;
	switch (can_port) {
	case CAN_PORT_E::CAN_PORT_5: {
		temp_can_ipc_instance_impl.can_port_instance = CAN_IPC_INS0_CAN5_CHAN4;
		temp_can_ipc_instance_impl.can_dev_port = CAN_DEV_PORT_E::CAN_DEV_PORT_5;
		break;
	};
	case CAN_PORT_E::CAN_PORT_6: {
		temp_can_ipc_instance_impl.can_port_instance = CAN_IPC_INS0_CAN6_CHAN6;
		temp_can_ipc_instance_impl.can_dev_port = CAN_DEV_PORT_E::CAN_DEV_PORT_6;
		break;
	};
	case CAN_PORT_E::CAN_PORT_7: {
		temp_can_ipc_instance_impl.can_port_instance = CAN_IPC_INS0_CAN7_CHAN7;
		temp_can_ipc_instance_impl.can_dev_port = CAN_DEV_PORT_E::CAN_DEV_PORT_7;
		break;
	};
	case CAN_PORT_E::CAN_PORT_8: {
		temp_can_ipc_instance_impl.can_port_instance = CAN_IPC_INS0_CAN8_CHAN2;
		temp_can_ipc_instance_impl.can_dev_port = CAN_DEV_PORT_E::CAN_DEV_PORT_8;
		break;
	};
	case CAN_PORT_E::CAN_PORT_9: {
		temp_can_ipc_instance_impl.can_port_instance = CAN_IPC_INS0_CAN9_CHAN3;
		temp_can_ipc_instance_impl.can_dev_port = CAN_DEV_PORT_E::CAN_DEV_PORT_9;
		break;
	};
	// default use CAN Port 5.
	default: {
		temp_can_ipc_instance_impl.can_port_instance = CAN_IPC_INS0_CAN5_CHAN4;
		temp_can_ipc_instance_impl.can_dev_port = CAN_DEV_PORT_E::CAN_DEV_PORT_5;
		break;
	};
	}

	return temp_can_ipc_instance_impl;
}

int CAN_IPC_LAYER_IMPL::lib_can_send(const CAN_PORT_E can_port, const can_frame_t &frame)
{
	(void)can_port;
	(void)frame;

	CAN_IPC_CONFIG_T temp_can_ipc_instance_impl = lib_set_can_ipc_handle(can_port);

	int ret = this->can_ipc_sender_handle->send_can_data(&temp_can_ipc_instance_impl, frame);

	return ret;
}

int CAN_IPC_LAYER_IMPL::lib_can_register_can_filter(const can_filter_t &can_filter,
						    const can_rx_callback_t &can_rx_callback)
{
	(void)can_filter;
	(void)can_rx_callback;

	CAN_IPC_FILTER_T can_ipc_receiver_filter = {
		.can_filter_id = can_filter.id,
		.can_filter_mask = can_filter.mask,
		.can_port = this->lib_get_can_dev_port(can_filter.can_port),
		.can_filter_callback = can_rx_callback,
	};
	return this->can_ipc_receiver_handle->register_can_filter(can_ipc_receiver_filter);
}

int CAN_IPC_LAYER_IMPL::lib_can_deregister_can_filter(const can_filter_t &can_filter)
{
	(void)can_filter;

	CAN_IPC_FILTER_T can_ipc_receiver_filter = {
		.can_filter_id = can_filter.id,
		.can_filter_mask = can_filter.mask,
		.can_port = this->lib_get_can_dev_port(can_filter.can_port),
		.can_filter_callback = NULL,
	};
	return this->can_ipc_receiver_handle->deregister_can_filter(can_ipc_receiver_filter);
}
