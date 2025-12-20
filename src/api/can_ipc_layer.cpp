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

#include "ret_code_def.h"

#include <time.h>

#include "can_ipc_layer_impl.hpp"
#include "ipc_socket_controller.hpp"
#include "message_log.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

void stop_test_thread()
{
	uint8_t stop_count = 0;
	std::shared_ptr<ipc_can::socket_shell::IPC_SOCKET_CONTROLLER> ipc_socket_impl =
		ipc_can::socket_shell::IPC_SOCKET_CONTROLLER::getInstance();

	while (1) {

		if (stop_count > 180) {
			ipc_socket_impl->stop();
			break;
		}

		// LOG_DEBUG("stop idle...");

		std::this_thread::sleep_for(std::chrono::seconds(1));
		stop_count += 1;
	}
}

void server_thread()
{
	std::shared_ptr<ipc_can::socket_shell::IPC_SOCKET_CONTROLLER> ipc_socket_impl =
		ipc_can::socket_shell::IPC_SOCKET_CONTROLLER::getInstance();

	bool ret = ipc_socket_impl->start_controller();
	if (ret != true) {
		LOG_ERROR("CAN IPC Socket Service Controller create error!");
	}
}

int main()
{
	// start ipc can socket controller
	LOG_INFO("Start IPC CAN Socket Service !");

	std::shared_ptr<ipc_can::socket_shell::IPC_SOCKET_CONTROLLER> ipc_socket_impl =
		ipc_can::socket_shell::IPC_SOCKET_CONTROLLER::getInstance();

	std::thread stop(stop_test_thread);
	std::thread server(server_thread);

	stop.detach();
	server.join();

	ipc_socket_impl->deinit_can_dev();

	std::this_thread::sleep_for(std::chrono::seconds(1));

	LOG_INFO("Finish IPC CAN Socket Service !");

	return 0;
}
