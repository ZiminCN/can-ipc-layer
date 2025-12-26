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
#include "can_ipc_layer/can_struct_define.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

std::atomic<bool> stop_test_flag{false};

// signal notify
std::mutex cmtx;
std::condition_variable cv;
bool cv_ready = false;
SOCKET_RETURN_ACK ack_info;
SOCKET_ORDER_E socket_ack_order;

int temp_server_socket = -1;

void connect_to_server()
{
	temp_server_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (temp_server_socket == -1) {
		std::cerr << "connect_to_server socket error!" << std::endl;
		throw std::runtime_error("Cannot create socket");
	}

	struct sockaddr_un addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	std::cout << "ipc_can_socket_server_path is " << ipc_can_socket_server_path << std::endl;
	strncpy(addr.sun_path, ipc_can_socket_server_path, sizeof(addr.sun_path));

	if (connect(temp_server_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		std::cerr << "connect_to_server connect error!" << std::endl;
		close(temp_server_socket);
		temp_server_socket = -1;
		throw std::runtime_error("Cannot create socket client");
	}

	std::cout << "Connect to Server at: " << ipc_can_socket_server_path << std::endl;
}

void socket_send_can_data()
{
	if (temp_server_socket == -1) {
		std::cout << "Error server socket: " << temp_server_socket << std::endl;
	}

	SOCKET_PACKAGE_T socket_package = {};
	socket_package.socket_magic_code = SOCKET_MAGIC_CODE;
	socket_package.socket_order = SOCKET_ORDER_E::SOCKET_ORDER_AS_SEND;
	socket_package.can_port = CAN_PORT_E::CAN_PORT_5;

	can_frame_t can_frame = {};
	can_frame.id = 0x2FF;
	can_frame.dlc = can_bytes_to_dlc(64);
	can_frame.flags = CAN_MODE_FD;
	std::memset(&can_frame.data, 0x3B, can_dlc_to_bytes(can_frame.dlc));

	socket_package.can_frame = can_frame;

	if (write(temp_server_socket, &socket_package, sizeof(socket_package)) !=
	    sizeof(socket_package)) {
		std::cout << "Client Write [socket_send_can_data] Error!" << std::endl;
	}
}

void socket_add_can_filter()
{
	if (temp_server_socket == -1) {
		std::cout << "Error server socket: " << temp_server_socket << std::endl;
	}

	uint32_t temp_can_filter_id[32] = {0x100};
	uint8_t temp_can_filter_count = 1;

	SOCKET_PACKAGE_T socket_package = {};
	socket_package.socket_magic_code = SOCKET_MAGIC_CODE;
	socket_package.socket_order = SOCKET_ORDER_E::SOCKET_ORDER_AS_ADD_FILTER;
	socket_package.can_port = CAN_PORT_E::CAN_PORT_5;

	socket_package.can_filter_cnt = temp_can_filter_count;
	std::memcpy(socket_package.can_filter_id, temp_can_filter_id, sizeof(temp_can_filter_id));

	if (write(temp_server_socket, &socket_package, sizeof(socket_package)) !=
	    sizeof(socket_package)) {
		std::cout << "Clite Write [socket_add_can_filter] Error!" << std::endl;
	}

	std::cout << "Wait for socket_add_can_filter Ack!" << std::endl;

	{
		std::unique_lock<std::mutex> lock(cmtx);
		cv.wait(lock, [] { return cv_ready; });
	}

	std::cout << "Get socket_add_can_filter Ack!" << std::endl;

	{
		std::unique_lock<std::mutex> lock(cmtx);
		cv_ready = false;
		if (socket_ack_order != SOCKET_ORDER_E::SOCKET_ORDER_AS_ADD_FILTER) {
			std::cout << "Get a error Ack order." << std::endl;
		}

		std::cout << "socket_add_can_filter ack is: " << static_cast<int>(ack_info)
			  << std::endl;
	}
}

void socket_remove_can_filter()
{
	if (temp_server_socket == -1) {
		std::cout << "Error server socket: " << temp_server_socket << std::endl;
	}

	uint32_t temp_can_filter_id[32] = {0x100};
	uint8_t temp_can_filter_count = 1;

	SOCKET_PACKAGE_T socket_package = {};
	socket_package.socket_magic_code = SOCKET_MAGIC_CODE;
	socket_package.socket_order = SOCKET_ORDER_E::SOCKET_ORDER_AS_REMOVE_FILTER;
	socket_package.can_port = CAN_PORT_E::CAN_PORT_5;
	socket_package.can_filter_cnt = temp_can_filter_count;
	memcpy(socket_package.can_filter_id, temp_can_filter_id, sizeof(temp_can_filter_id));

	if (write(temp_server_socket, &socket_package, sizeof(socket_package)) !=
	    sizeof(socket_package)) {
		std::cout << "Clite Write [socket_remove_can_filter] Error!" << std::endl;
	}

	std::cout << "Wait for socket_remove_can_filter Ack!" << std::endl;

	{
		std::unique_lock<std::mutex> lock(cmtx);
		cv.wait(lock, [] { return cv_ready; });
	}

	std::cout << "Get socket_remove_can_filter Ack!" << std::endl;

	{
		std::unique_lock<std::mutex> lock(cmtx);
		cv_ready = false;
		if (socket_ack_order != SOCKET_ORDER_E::SOCKET_ORDER_AS_REMOVE_FILTER) {
			std::cout << "Get a error Ack order." << std::endl;
		}

		std::cout << "socket_remove_can_filter ack is: " << static_cast<int>(ack_info)
			  << std::endl;
	}
}

void receive_thread()
{
	std::cout << "Start receive thread." << std::endl;

	while (1) {
		SOCKET_PACKAGE_T socket_package = {};
		ssize_t bytes_read =
			read(temp_server_socket, &socket_package, sizeof(socket_package));
		if (bytes_read == -1) {
			throw std::runtime_error(
				"[receive_thread] socket client connection break!");
		}

		if (socket_package.socket_magic_code != SOCKET_MAGIC_CODE) {
			std::cout << "[receive_thread] Read a invalied data." << std::endl;
		}

		switch (socket_package.socket_order) {
		case SOCKET_ORDER_E::SOCKET_ORDER_AS_RECEIVE: {
			std::cout << "socket_package.can_frame.id is 0x"
				  << static_cast<uint32_t>(socket_package.can_frame.id)
				  << std::endl;
			std::cout << "socket_package.can_frame.dlc is "
				  << static_cast<int>(socket_package.can_frame.dlc) << std::endl;
			std::cout << "socket_package.can_frame.flags is "
				  << static_cast<int>(socket_package.can_frame.flags) << std::endl;

			std::cout << "socket_package.can_frame.data is: [";
			for (int i = 0; i < can_dlc_to_bytes(socket_package.can_frame.dlc); i++) {
				std::cout << std::hex
					  << static_cast<int>(socket_package.can_frame.data[i])
					  << " ";
			}
			std::cout << std::dec << "]" << std::endl;

			break;
		}
		case SOCKET_ORDER_E::SOCKET_ORDER_AS_ACK: {
			std::cout << "[receive_thread] Get Ack!" << std::endl;

			{
				std::lock_guard<std::mutex> lock(cmtx);
				cv_ready = true;
				socket_ack_order = socket_package.socket_ack_for_order;
				ack_info = socket_package.socket_ack;
			}

			cv.notify_one();
			break;
		}
		default: {
			break;
		}
		}
	}
}

int main()
{
	std::cout << "Start Test Local main" << std::endl;

	std::cout << "Connect to Server" << std::endl;

	connect_to_server();

	bool loop_flag = true;
	uint8_t loop_cnt = 0;

	auto last_tnow = std::chrono::high_resolution_clock::now();
	auto ns_last_epoch =
		std::chrono::duration_cast<std::chrono::nanoseconds>(last_tnow.time_since_epoch())
			.count();

	std::thread receive_task{receive_thread};
	receive_task.detach();

	socket_add_can_filter();

	while (loop_flag) {
		if (loop_cnt >= 3) {
			loop_flag = false;
		}

		std::cout << "Send can data" << std::endl;

		socket_send_can_data();

		// TODO(zimin): record time interval and print it.
		auto since_now = std::chrono::high_resolution_clock::now();

		auto ns_since_epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(
					      since_now.time_since_epoch())
					      .count();

		auto diff_epoch = ns_since_epoch - ns_last_epoch;

		ns_last_epoch = ns_since_epoch;

		std::cout << "时间戳: " << ns_since_epoch << std::endl;
		std::cout << "间隔时间: " << diff_epoch / 1000 << "us" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		loop_cnt += 1;
	}

	socket_remove_can_filter();

	return 0;
}