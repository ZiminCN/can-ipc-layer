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
	if(ret < 0){
		LOG_ERROR("canInit error!" << " return value is [" << ret << "].");
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
