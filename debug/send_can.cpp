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

#include <atomic>
#include <canhal/hobot_can_hal.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

#define MAX_RX_BUF_SIZE 4000
#define BSWAP_32(x)                                                                                \
	(uint32_t)((((uint32_t)(x)&0xff000000) >> 24) | (((uint32_t)(x)&0x00ff0000) >> 8) |        \
		   (((uint32_t)(x)&0x0000ff00) << 8) | (((uint32_t)(x)&0x000000ff) << 24))

typedef struct {
	char *target;
	int canid;
} test_param;

void test_can_send()
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
		std::cout << "canSendMsgFrame failed! ret is [ " << ret << " ]." << std::endl;
	}
}

int main(void)
{
	std::cout << "Send CAN Data!" << std::endl;

	int ret = canInit();
	if (ret != 0) {
		std::cout << "canInit error!" << std::endl;
	}

	test_can_send();

	canDeInit();

	return 0;
}
