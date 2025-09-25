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

#include "can_ipc_layer/can_ipc_layer.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

int main()
{
	std::cout << "Debug Test Start!" << std::endl;
	can_ipc_layer_handle_t *can_ipc_layer_handle = can_ipc_layer_create();
	std::cout << "Test Basic Send Data Func..." << std::endl;
	test_can_send(&can_ipc_layer_handle);

	std::cout << "Test Send Data Func..." << std::endl;
	struct can_frame_t tx_frame;
	tx_frame.id = 0x123;
	tx_frame.dlc = can_bytes_to_dlc(8);
	tx_frame.flags = CAN_MODE_FD;
	memset(tx_frame.data, 0xFF, sizeof(tx_frame.data));
	can_send(&can_ipc_layer_handle, CAN_PORT_E::CAN_PORT_5, &tx_frame);

	// std::cout << "Test Basic Send Data Func..." << std::endl;

	can_ipc_layer_destroy(&can_ipc_layer_handle);
	std::cout << "Debug Test End!" << std::endl;
	return 0;
}