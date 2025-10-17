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

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>

std::atomic<bool> stop_test_flag{false};

void get_can_data_callback(struct can_frame_t *frame, void *user_data)
{
	(void)frame;
	(void)user_data;

	std::cout << "Trigger get_can_data_callback..." << std::endl;
	std::cout << ">>>>>>>>>>>> Get CAN Data <<<<<<<<<<<<" << std::endl;

	std::cout << "Get CAN ID: [" << std::hex << frame->id << "]." << std::endl;
	std::cout << "Get CAN DLC: [" << static_cast<int>(frame->dlc) << "]." << std::endl;
	std::cout << "Get CAN FLAGS: [" << static_cast<int>(frame->flags) << "]." << std::endl;

	std::cout << "Get CAN Data: [";
	for (int it = 0; it < can_dlc_to_bytes(frame->dlc); it++) {
		std::cout << static_cast<int>(frame->data[it]) << " ";
	}
	std::cout << "]." << std::endl;

	// stop_test_flag.store(true);
}

int main()
{
	std::cout << "============ Debug Test Start! ============" << std::endl;
	can_ipc_layer_handle_t *can_ipc_layer_handle = can_ipc_layer_create();

	std::cout << "============ Test Add CAN Filter Func ============" << std::endl;
	struct can_filter_t can_rx_filter;
	can_rx_filter.id = 0x000;
	can_rx_filter.mask = 0x000;
	can_rx_filter.can_port = CAN_PORT_E::CAN_PORT_5;
	can_add_filter(&can_ipc_layer_handle, &can_rx_filter, get_can_data_callback);

	std::cout << "============ Test Start Receive Data Func ============" << std::endl;
	can_ipc_receiver_port_enable(&can_ipc_layer_handle, CAN_PORT_E::CAN_PORT_5);
	can_ipc_receiver_start(&can_ipc_layer_handle);

	std::cout << "============ Test Basic Send Data Func ============" << std::endl;
	std::cout << ">>>>>>>>>>>> Send Data ID: 0x82 <<<<<<<<<<<<" << std::endl;
	test_can_send(&can_ipc_layer_handle);

	std::cout << "============ Test Send Data Func ============" << std::endl;
	std::cout << ">>>>>>>>>>>> Send Data ID: 0x123 <<<<<<<<<<<<" << std::endl;
	struct can_frame_t tx_frame;
	tx_frame.id = 0x123;
	tx_frame.dlc = can_bytes_to_dlc(8);
	tx_frame.flags = CAN_MODE_FD;
	memset(tx_frame.data, 0xFF, sizeof(tx_frame.data));
	can_send(&can_ipc_layer_handle, CAN_PORT_E::CAN_PORT_5, &tx_frame);

	int timer_count = 0;

	// while((timer_count <= 10) && (!stop_test_flag.load())){
	while (timer_count <= 100) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		timer_count += 1;
	}

	// std::cout << "============ Test Remove CAN Filter Func ============" << std::endl;
	// can_remove_filter(&can_ipc_layer_handle, &can_rx_filter);

	can_ipc_layer_destroy(&can_ipc_layer_handle);
	std::cout << "============ Debug Test End ============" << std::endl;
	return 0;
}