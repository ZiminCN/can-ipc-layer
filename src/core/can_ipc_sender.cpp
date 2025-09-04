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

#include "can_ipc_sender.hpp"
#include <hobot_can_hal.h>
#include <sys/time.h>

#define MAX_RX_BUF_SIZE 4000
#define BSWAP_32(x) \
	(uint32_t)((((uint32_t)(x) & 0xff000000) >> 24) | \
	(((uint32_t)(x) & 0x00ff0000) >> 8) | \
	(((uint32_t)(x) & 0x0000ff00) << 8) | \
	(((uint32_t)(x) & 0x000000ff) << 24) \
)

std::unique_ptr<CAN_IPC_SENDER> CAN_IPC_SENDER::Instance = std::make_unique<CAN_IPC_SENDER>();

std::unique_ptr<CAN_IPC_SENDER> CAN_IPC_SENDER::getInstance()
{
	return std::move(CAN_IPC_SENDER::Instance);
}

void CAN_IPC_SENDER::test_c_can_send_frame_data()
{
	// init can api
	int ret = 0;
	ret = canInit();
	if(ret < 0){
	}
}
