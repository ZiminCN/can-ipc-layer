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
#ifndef __IPC_SOCKET_CONTROLLER_HPP__
#define __IPC_SOCKET_CONTROLLER_HPP__

#include "can_struct_define.h"

#include "can_ipc_layer_impl.hpp"
#include "ipc_socket_controller.hpp"
#include "message_log.hpp"
#include <cstring>
#include <set>
#include <sys/socket.h>
#include <sys/un.h>
#include <unordered_map>

namespace ipc_can
{

namespace socket_shell
{

class IPC_SOCKET_CONTROLLER
{
      public:
	IPC_SOCKET_CONTROLLER()
	{
		LOG_DEBUG("IPC_SOCKET_CONTROLLER impl init.");
		this->init_socket_client_manager();
		this->_is_running.store(true);
	}
	~IPC_SOCKET_CONTROLLER() = default;
	static std::shared_ptr<IPC_SOCKET_CONTROLLER> &getInstance();
	void init_can_dev();
	void deinit_can_dev();
	bool start_controller(void);
	void direct_test_can_send();
	int direct_can_send(const CAN_PORT_E can_port, const can_frame_t &frame);
	int direct_can_register_can_filter(const can_filter_t &can_filter,
					   const can_rx_callback_t &can_rx_callback);
	int direct_can_deregister_can_filter(const can_filter_t &can_filter);
	void direct_enable_can_receiver_port(const CAN_PORT_E can_port);
	void stop();
	IPC_SOCKET_CONTROLLER(const IPC_SOCKET_CONTROLLER &) = delete;
	IPC_SOCKET_CONTROLLER &operator=(const IPC_SOCKET_CONTROLLER &) = delete;

      private:
	std::atomic<bool> _is_running{false};
	inline const static std::string _run_socket_path = ipc_can_socket_server_path;
	const static uint8_t SOCKET_CAN_DATA_BUFFER_SIZE = 128;
	inline static int _service_socket = {};
	inline static struct sockaddr_un _service_addr = {};
	static std::shared_ptr<IPC_SOCKET_CONTROLLER> Instance;
	std::unique_ptr<CAN_IPC_LAYER_IMPL> &can_ipc_layer_impl = CAN_IPC_LAYER_IMPL::getInstance();

	// process client order
	static void handle_client(int client_socket);
	static void signal_handler(int sig);

	// get client connection
	void accept_connections();

	static ssize_t read_full(int socket, void *buf, size_t n);

	void direct_start_can_ipc_receiver();
	void direct_pause_can_ipc_receiver();
	void direct_resume_can_ipc_receiver();

	static void socket_receive_callback(struct can_frame_t *frame, int socket_index);

	struct CAN_IPC_SOCKET_CLIENT_MANAGER_T {
		// key: socket client index, value: can id vector
		std::unordered_map<int, std::set<uint32_t>> socket_client_index;
	};

	inline static std::unique_ptr<CAN_IPC_SOCKET_CLIENT_MANAGER_T> socket_client_manager =
		std::make_unique<CAN_IPC_SOCKET_CLIENT_MANAGER_T>();
	void init_socket_client_manager();
	void client_manager_register(can_filter_t can_filter);
	void client_manager_deregister(can_filter_t can_filter);
	void clean_socket_client_index(int socket_index);
};

}; // namespace socket_shell

}; // namespace ipc_can

#endif // __IPC_SOCKET_CONTROLLER_HPP__