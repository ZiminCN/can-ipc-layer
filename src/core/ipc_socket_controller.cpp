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

#include "ipc_socket_controller.hpp"

#include <arpa/inet.h>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace ipc_can
{

namespace socket_shell
{

std::shared_ptr<IPC_SOCKET_CONTROLLER> &IPC_SOCKET_CONTROLLER::getInstance()
{
	static std::shared_ptr<IPC_SOCKET_CONTROLLER> Instance =
		std::make_shared<IPC_SOCKET_CONTROLLER>();
	return Instance;
}

void IPC_SOCKET_CONTROLLER::init_can_dev()
{
	this->can_ipc_layer_impl->init_can_dev();
}

void IPC_SOCKET_CONTROLLER::deinit_can_dev()
{
	this->can_ipc_layer_impl->deinit_can_dev();
}

bool IPC_SOCKET_CONTROLLER::start_controller()
{
	// remove any existing socket files that may exist
	if (access(this->_run_socket_path.c_str(), F_OK) != -1) {
		if (unlink(this->_run_socket_path.c_str()) == -1) {
			LOG_ERROR("Failed to remove existing socket file: " << strerror(errno));
			return false;
		} else {
			LOG_DEBUG("Unlink success.");
		}
	} else {
		LOG_DEBUG("Access: Do not unlink file because file not exists!");
	}

	this->_service_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (this->_service_socket == -1) {
		LOG_ERROR("Socket Create Error!");
		return false;
	}

	// bind to file-system socket
	std::memset(&this->_service_addr, 0x00, sizeof(this->_service_addr));
	this->_service_addr.sun_family = AF_UNIX;
	strncpy(this->_service_addr.sun_path, this->_run_socket_path.c_str(),
		sizeof(this->_service_addr.sun_path) - 1);
	this->_service_addr.sun_path[sizeof(this->_service_addr.sun_path) - 1] = '\0';

	// mkdir target dir path
	size_t last_path_slash = this->_run_socket_path.find_last_of('/');
	if (last_path_slash != std::string::npos) {
		std::string father_run_socket_path =
			this->_run_socket_path.substr(0, last_path_slash);

		LOG_DEBUG("father_run_socket_path is " << father_run_socket_path);

		if (access(father_run_socket_path.c_str(), F_OK) == -1) {
			mkdir(father_run_socket_path.c_str(), 0755);
		}
	}

	if (bind(this->_service_socket, (struct sockaddr *)&this->_service_addr,
		 sizeof(this->_service_addr)) == -1) {
		LOG_ERROR("Bind Error for " << this->_service_socket);
		close(this->_service_socket);
		return false;
	}

	// max client ended number is 10
	if (listen(this->_service_socket, 10) == -1) {
		LOG_ERROR("Listen Error " << this->_service_socket);
		close(this->_service_socket);
		return false;
	}

	// set permission about service socket
	if (chmod(this->_run_socket_path.c_str(), 0666) != 0) {
		LOG_ERROR("chmod failed, can not set permission for "
			  << this->_run_socket_path.c_str());
		close(this->_service_socket);
		return false;
	}

	LOG_INFO("SERVER: Singleton server started (PID: " << getpid() << ")");
	LOG_INFO("SERVER: Socket " << this->_run_socket_path.c_str());
	LOG_INFO("SERVER: Ready to accept client connections...");

	// SIGHUP/SIGPIPE/SIGCHLD
	signal(SIGINT, IPC_SOCKET_CONTROLLER::signal_handler);
	signal(SIGTERM, IPC_SOCKET_CONTROLLER::signal_handler);

	// start receiver
	this->direct_enable_can_receiver_port(CAN_PORT_E::CAN_PORT_5);
	this->direct_start_can_ipc_receiver();

	this->accept_connections();

	return true;
}

// TODO(zimin): add can port.
void IPC_SOCKET_CONTROLLER::socket_receive_callback(struct can_frame_t *frame, int socket_index)
{
	std::shared_ptr<IPC_SOCKET_CONTROLLER> controller = IPC_SOCKET_CONTROLLER::getInstance();
	SOCKET_PACKAGE_T socket_package = {};

	int client_socket = static_cast<int>(socket_index);
	// check client index is register in the client manager
	auto client_manager =
		controller->socket_client_manager->socket_client_index.find(client_socket);

	LOG_DEBUG("SOcket Client Manager Contents:");
	if (controller->socket_client_manager->socket_client_index.empty()) {
		LOG_DEBUG("socket_client_index (Empty)");
		return;
	}

	for (const auto& [client_index, can_ids] : controller->socket_client_manager->socket_client_index) {
		LOG_DEBUG("Client Index: " << client_index);
		LOG_DEBUG("	CAN IDs (" << can_ids.size() << "): ");
		
		if (can_ids.empty()) {
			LOG_DEBUG("(None)");
		} else {
		for (const auto& id : can_ids) {
			std::cout << "0x" << std::hex << id << std::dec << " ";
		}
		std::cout << std::endl;
		}
		std::cout << std::endl;
    	}

	if (client_manager == controller->socket_client_manager->socket_client_index.end()) {
		LOG_DEBUG("target client index is disconnected.");
		return;
	}

	socket_package.socket_magic_code = SOCKET_MAGIC_CODE;
	socket_package.socket_order = SOCKET_ORDER_E::SOCKET_ORDER_AS_RECEIVE;

	std::memcpy(&socket_package.can_frame, frame, sizeof(can_frame_t));
	LOG_DEBUG("Receive Callback Order");
	if (write(client_socket, &socket_package, sizeof(socket_package)) !=
	    sizeof(socket_package)) {
		LOG_ERROR("Error write: " << client_socket << "with can_frame id: " << frame->id);
	}
}

void IPC_SOCKET_CONTROLLER::direct_test_can_send()
{
	return this->can_ipc_layer_impl->lib_test_can_send();
}

int IPC_SOCKET_CONTROLLER::direct_can_send(const CAN_PORT_E can_port, const can_frame_t &frame)
{
	return this->can_ipc_layer_impl->lib_can_send(can_port, frame);
}

int IPC_SOCKET_CONTROLLER::direct_can_register_can_filter(const can_filter_t &can_filter,
							  const can_rx_callback_t &can_rx_callback)
{
	return this->can_ipc_layer_impl->lib_can_register_can_filter(can_filter, can_rx_callback);
}

int IPC_SOCKET_CONTROLLER::direct_can_deregister_can_filter(const can_filter_t &can_filter)
{
	return this->can_ipc_layer_impl->lib_can_deregister_can_filter(can_filter);
}

void IPC_SOCKET_CONTROLLER::direct_enable_can_receiver_port(const CAN_PORT_E can_port)
{
	return this->can_ipc_layer_impl->lib_enable_can_receiver_port(can_port);
}

void IPC_SOCKET_CONTROLLER::accept_connections()
{
	while (this->_is_running.load()) {
		int client_socket = accept(this->_service_socket, nullptr, nullptr);
		if (client_socket == -1) {
			if (!this->_is_running.load()) {
				LOG_INFO("SERVER: Accept interrupted by shutdown.");
			} else {
				LOG_ERROR("SERVER: Accept error!");
			}
			break;
		}

		LOG_INFO("SERVER: New client connected!");
		std::thread client_thread(IPC_SOCKET_CONTROLLER::handle_client, client_socket);
		client_thread.detach();
	}
}

void IPC_SOCKET_CONTROLLER::stop()
{
	this->_is_running.store(false);
	// 关键：关闭服务端socket以中断accept
	if (this->_service_socket != -1) {
		shutdown(this->_service_socket, SHUT_RDWR); // 先shutdown，更优雅
		close(this->_service_socket);
		this->_service_socket = -1;
	}
}

ssize_t IPC_SOCKET_CONTROLLER::read_full(int client_socket, void *buf, size_t n)
{
	uint8_t *ptr = static_cast<uint8_t *>(buf);
	size_t total_read = 0;

	while (total_read < n) {
		ssize_t nread = read(client_socket, ptr + total_read, n - total_read);
		if (nread <= 0) {
			if (nread == -1) {
				if (errno == EINTR) {
					continue;
				}

				return nread;
			}

			return total_read > 0 ? total_read : 0;
		}

		total_read += nread;
	}

	return total_read;
}

void IPC_SOCKET_CONTROLLER::direct_start_can_ipc_receiver()
{
	if (this->can_ipc_layer_impl == nullptr) {
		LOG_ERROR("Invalied point!");
	}

	return this->can_ipc_layer_impl->lib_start_can_ipc_receiver();
}

void IPC_SOCKET_CONTROLLER::direct_pause_can_ipc_receiver()
{
	return this->can_ipc_layer_impl->lib_pause_can_ipc_receiver();
}

void IPC_SOCKET_CONTROLLER::direct_resume_can_ipc_receiver()
{
	return this->can_ipc_layer_impl->lib_resume_can_ipc_receiver();
}

void IPC_SOCKET_CONTROLLER::handle_client(int client_socket)
{
	std::shared_ptr<IPC_SOCKET_CONTROLLER> controller = IPC_SOCKET_CONTROLLER::getInstance();

	while (controller->_is_running.load()) {
		// socket package header decode
		SOCKET_PACKAGE_T socket_package = {};

		ssize_t package_read =
			read_full(client_socket, &socket_package, sizeof(socket_package));
		if (package_read <= 0) {
			break;
		}

		SOCKET_ORDER_E target_socket_order = socket_package.socket_order;
		CAN_PORT_E target_can_port = socket_package.can_port;

		// process client socket package
		if (socket_package.socket_magic_code != SOCKET_MAGIC_CODE) {
			LOG_ERROR("Invalid socket magic code: 0x"
				  << std::hex << socket_package.socket_magic_code << std::endl);
			return;
		}

		switch (target_socket_order) {
		case SOCKET_ORDER_E::SOCKET_ORDER_AS_SEND: {
			can_frame_t temp_can_frame = {};
			std::memcpy(&temp_can_frame, &socket_package.can_frame,
				    sizeof(can_frame_t));

			controller->direct_can_send(target_can_port, temp_can_frame);
			LOG_DEBUG("Send Order");
			break;
		}
		case SOCKET_ORDER_E::SOCKET_ORDER_AS_ADD_FILTER: {
			// stop receiver first
			controller->direct_pause_can_ipc_receiver();

			can_filter_t temp_filter = {};
			temp_filter.can_port = socket_package.can_port;
			temp_filter.id = socket_package.can_filter_id;
			temp_filter.id_cnt = socket_package.can_filter_cnt;
			temp_filter.socket_index = client_socket;

			// add socket client to manager
			controller->client_manager_register(temp_filter);

			int ret = controller->direct_can_register_can_filter(
				temp_filter, controller->socket_receive_callback);
			if (ret == 0) {
				LOG_INFO("Add Filter success!");
				socket_package.socket_order = SOCKET_ORDER_E::SOCKET_ORDER_AS_ACK;
				socket_package.socket_ack = SOCKET_RETURN_ACK::ACK_OKAY;
				socket_package.socket_ack_for_order = target_socket_order;
			} else {
				LOG_WARNING("Add Filter failed!");
				socket_package.socket_order = SOCKET_ORDER_E::SOCKET_ORDER_AS_ACK;
				socket_package.socket_ack = SOCKET_RETURN_ACK::ACK_ERROR;
				socket_package.socket_ack_for_order = target_socket_order;
			}

			if (write(client_socket, &socket_package, sizeof(socket_package)) !=
			    sizeof(socket_package)) {
				LOG_WARNING("Error write with ACK for add can filter");
			}

			// start receiver
			controller->direct_resume_can_ipc_receiver();
			LOG_DEBUG("Add Filter Order");
			break;
		}
		case SOCKET_ORDER_E::SOCKET_ORDER_AS_REMOVE_FILTER: {
			controller->direct_pause_can_ipc_receiver();

			can_filter_t temp_filter = {};
			temp_filter.can_port = socket_package.can_port;
			temp_filter.id = socket_package.can_filter_id;
			temp_filter.id_cnt = socket_package.can_filter_cnt;
			temp_filter.socket_index = client_socket;

			// remove socket client to manager
			controller->client_manager_deregister(temp_filter);

			int ret = controller->direct_can_deregister_can_filter(temp_filter);
			if (ret == 0) {
				LOG_INFO("Remove Filter success!");
				socket_package.socket_order = SOCKET_ORDER_E::SOCKET_ORDER_AS_ACK;
				socket_package.socket_ack = SOCKET_RETURN_ACK::ACK_OKAY;
				socket_package.socket_ack_for_order = target_socket_order;
			} else {
				LOG_WARNING("Remove Filter failed!");
				socket_package.socket_order = SOCKET_ORDER_E::SOCKET_ORDER_AS_ACK;
				socket_package.socket_ack = SOCKET_RETURN_ACK::ACK_ERROR;
				socket_package.socket_ack_for_order = target_socket_order;
			}

			if (write(client_socket, &socket_package, sizeof(socket_package)) !=
			    sizeof(socket_package)) {
				LOG_WARNING("Error write with ACK for remove can filter");
			}

			controller->direct_resume_can_ipc_receiver();
			LOG_DEBUG("Remove Filter Order");
			break;
		}
		default: {
			LOG_WARNING("Invailed order.");
			break;
		}
		}
	}

	// clean socket client index
	controller->clean_socket_client_index(client_socket);
	close(client_socket);
	LOG_INFO("IPC Socket: Client[ " << static_cast<int>(client_socket) << " ] Disconnected.");
}

void IPC_SOCKET_CONTROLLER::signal_handler(int sig)
{
	std::shared_ptr<IPC_SOCKET_CONTROLLER> controller = IPC_SOCKET_CONTROLLER::getInstance();

	(void)sig;
	LOG_INFO("Shutting down...");
	controller->_is_running.store(false);
	close(controller->_service_socket);
	unlink(controller->_run_socket_path.c_str()); // 清理套接字文件

	exit(0);
}

void IPC_SOCKET_CONTROLLER::init_socket_client_manager()
{
	this->socket_client_manager->socket_client_index.clear();
}

void IPC_SOCKET_CONTROLLER::client_manager_register(can_filter_t can_filter)
{
	if (!this->socket_client_manager) {
		LOG_ERROR("Error, Invalid can filter");
		return;
	}

	auto &can_id_set =
		this->socket_client_manager->socket_client_index[can_filter.socket_index];
	for (uint32_t it = 0; it < can_filter.id_cnt; it++) {
		can_id_set.insert(can_filter.id[it]);
	}
}

void IPC_SOCKET_CONTROLLER::client_manager_deregister(can_filter_t can_filter)
{
	auto it = this->socket_client_manager->socket_client_index.find(can_filter.socket_index);
	if (it == this->socket_client_manager->socket_client_index.end()) {
		return;
	}

	auto &can_id_set =
		this->socket_client_manager->socket_client_index[can_filter.socket_index];
	for (uint32_t it = 0; it < can_filter.id_cnt; it++) {
		can_id_set.erase(can_filter.id[it]);
	}

	if (can_id_set.empty()) {
		this->clean_socket_client_index(can_filter.socket_index);
	}
}

void IPC_SOCKET_CONTROLLER::clean_socket_client_index(int socket_index)
{
	this->socket_client_manager->socket_client_index.erase(socket_index);
}

}; // namespace socket_shell

}; // namespace ipc_can
