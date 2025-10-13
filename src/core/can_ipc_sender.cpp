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
#include "hobot_can_hal.h"

#include "message_log.hpp"
#include <chrono>
#include <cstdint>
#include <sys/time.h>

#define MAX_RX_BUF_SIZE 4000
#define BSWAP_32(x)                                                                                \
	(uint32_t)((((uint32_t)(x)&0xff000000) >> 24) | (((uint32_t)(x)&0x00ff0000) >> 8) |        \
		   (((uint32_t)(x)&0x0000ff00) << 8) | (((uint32_t)(x)&0x000000ff) << 24))

typedef struct {
	char *target;
	int canid;
} test_param;

void CAN_IPC_SENDER::test_send_can_frame()
{
	test_param test_params;
	uint32_t canid = 5;

	std::string target = "can5_ins0ch4";
	char ctarget[16];
	strcpy(ctarget, target.c_str());

	test_params.target = ctarget;
	test_params.canid = canid;

	//!
	struct pack_info pack = {
		.soc_ts = 0,
		.data_num = 0,
		.mcu_ts = 0,
		.length = 0,
		.unused = 0,
		.unused_1 = 0,
	};
	uint8_t Can_au8Sdu8bytes[64U] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xFF};
	struct canframe frame = {
		.time_stamp = 0,
		.canid = BSWAP_32(0x82U),
		.count = 100,
		.can_type = 1,
		.can_channel = static_cast<uint8_t>(test_params.canid),
		.len = 8,
		.data = 0,
	};
	memset(frame.data, 0x00, sizeof(frame.data));
	uint32_t frame_num = 1;

	pack.data_num = frame_num;
	pack.length = pack.data_num;

	memcpy(frame.data, Can_au8Sdu8bytes, frame.len);

	int ret = canSendMsgFrame(test_params.target, &frame, &pack);
	if (ret < 0) {
		LOG_ERROR("canSendMsgFrame failed! ret is [ " << ret << " ].");
	}
}

// single can frame send function
int CAN_IPC_SENDER::send_can_data(CAN_IPC_CONFIG_T *can_ipc_config, const can_frame_t &frame)
{
	(void)can_ipc_config;
	(void)frame;

	char ctarget_instance[16];
	strcpy(ctarget_instance, can_ipc_config->can_port_instance.c_str());
	can_port_target_t can_port_target = {
		.target_instance = ctarget_instance,
		.can_port_index = static_cast<uint8_t>(can_ipc_config->can_dev_port),
	};

	struct pack_info pack = {
		.soc_ts = 0,
		.data_num = 1,
		.mcu_ts = 0,
		.length = 1,
		.unused = 0,
		.unused_1 = 0,
	};

	struct canframe tx_frame = {
		// use steady clock to get time stamp, rather than system clock.
		.time_stamp = static_cast<uint64_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now().time_since_epoch())
				.count()),
		// byte order reversal
		.canid = BSWAP_32(static_cast<uint32_t>(frame.id)),
		// i dont know what is count, so just set it to 1, you can refer to
		// /usr/hobot/include/canhal/hobot_can_hal.h
		.count = 1,
		// CANType_Can = 0, CANType_Canfd = 1
		.can_type = frame.flags,
		.can_channel = static_cast<uint8_t>(can_ipc_config->can_dev_port),
		.len = can_dlc_to_bytes(frame.dlc),
		.data = 0,
	};
	memcpy(tx_frame.data, frame.data, can_dlc_to_bytes(frame.dlc));

	int ret = canSendMsgFrame(can_port_target.target_instance, &tx_frame, &pack);

	return ret;
}
