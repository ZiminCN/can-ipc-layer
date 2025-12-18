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

#include "can_ipc_receiver.hpp"
#include "hobot_can_hal.h"
#include "ret_code_def.h"

#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <memory>
#include <sstream>

// NOTE[x]: [Core] Hash table(message info; callback info;)
// NOTE[]: [Core] message receive filter task(blockade)
// NOTE[]: [Core] callback work queue(blockade)
// NOTE[]: [Core] Weak pointer. Use weak point to detecting whether the target object is still
// alive. NOTE[]: [Core] thread pool(parallel execution)

// TODO[x]: [Func] register can filter with "can id" + "can mask" + "callback function"
// TODO[x]: [Func] de-register can filter with "can id" + "can mask" + "callback function"
// TODO[x]: [Func] hash table Add, delete, check and revise

void CAN_IPC_RECEIVER::clear_hash_table(CAN_IPC_RECEIVER_FILTER_T &can_ipc_receiver_filter)
{
	can_ipc_receiver_filter.can_ipc_filter_id_map.clear();
	can_ipc_receiver_filter.can_ipc_filter_map.clear();
}

void CAN_IPC_RECEIVER::init_can_filter()
{
	this->clear_hash_table(*this->get_can_ipc_receiver_port_5_filter().get());
	this->clear_hash_table(*this->get_can_ipc_receiver_port_6_filter().get());
	this->clear_hash_table(*this->get_can_ipc_receiver_port_7_filter().get());
	this->clear_hash_table(*this->get_can_ipc_receiver_port_8_filter().get());
	this->clear_hash_table(*this->get_can_ipc_receiver_port_9_filter().get());
	this->get_can_ipc_receiver_port_5_filter().get()->can_port = CAN_DEV_PORT_E::CAN_DEV_PORT_5;
	this->get_can_ipc_receiver_port_6_filter().get()->can_port = CAN_DEV_PORT_E::CAN_DEV_PORT_6;
	this->get_can_ipc_receiver_port_7_filter().get()->can_port = CAN_DEV_PORT_E::CAN_DEV_PORT_7;
	this->get_can_ipc_receiver_port_8_filter().get()->can_port = CAN_DEV_PORT_E::CAN_DEV_PORT_8;
	this->get_can_ipc_receiver_port_9_filter().get()->can_port = CAN_DEV_PORT_E::CAN_DEV_PORT_9;
}

CAN_IPC_RECEIVER_FILTER_T *
CAN_IPC_RECEIVER::return_can_ipc_receiver_filter(const CAN_DEV_PORT_E can_port)
{
	switch (can_port) {
	case CAN_DEV_PORT_E::CAN_DEV_PORT_5: {
		return this->get_can_ipc_receiver_port_5_filter().get();
		break;
	}
	case CAN_DEV_PORT_E::CAN_DEV_PORT_6: {
		return this->get_can_ipc_receiver_port_6_filter().get();
		break;
	}
	case CAN_DEV_PORT_E::CAN_DEV_PORT_7: {
		return this->get_can_ipc_receiver_port_7_filter().get();
		break;
	}
	case CAN_DEV_PORT_E::CAN_DEV_PORT_8: {
		return this->get_can_ipc_receiver_port_8_filter().get();
		break;
	}
	case CAN_DEV_PORT_E::CAN_DEV_PORT_9: {
		return this->get_can_ipc_receiver_port_9_filter().get();
		break;
	}
	default: {
		LOG_WARNING("Invalid CAN Port.");
		return nullptr;
		break;
	}
	}

	return nullptr;
}

int CAN_IPC_RECEIVER::register_can_filter(const CAN_IPC_FILTER_T &can_ipc_filter)
{
	// 1: [can_ipc_filter_map] emplace the CAN ID and struct.
	// 2: [can_ipc_filter_id_map] emplace the CAN ID, if return error, increase the count value
	// of the CAN ID

	if (can_ipc_filter.can_filter_id_cnt == 0) {
		LOG_ERROR("CAN Filter ID count is zero.");
		return -RET_CODE_INVALID_ARG;
	}

	if (can_ipc_filter.can_filter_id == nullptr) {
		LOG_ERROR("CAN Filter ID pointer is null.");
		return -RET_CODE_INVALID_ARG;
	}

	for (uint32_t it = 0; it < can_ipc_filter.can_filter_id_cnt; it++) {
		LOG_DEBUG("Input can_ipc_filter.can_filter_id["
			  << it << "]: [" << can_ipc_filter.can_filter_id[it] << "]");
	}
	LOG_DEBUG("Input can_ipc_filter.can_filter_id_cnt is " << can_ipc_filter.can_filter_id_cnt);

	CAN_IPC_RECEIVER_FILTER_T *can_ipc_receiver_filter = this->return_can_ipc_receiver_filter(
		static_cast<CAN_DEV_PORT_E>(can_ipc_filter.can_port));
	if (can_ipc_receiver_filter == nullptr) {
		return -RET_CODE_INVALID_ARG;
	}

	for (uint32_t it = 0; it < can_ipc_filter.can_filter_id_cnt; it++) {
		auto filter_result = can_ipc_receiver_filter->can_ipc_filter_map.emplace(
			can_ipc_filter.can_filter_id[it], can_ipc_filter);
		if (filter_result.second == false) {
			LOG_ERROR("Add CAN Filter failed, the Filter CAN ID is: ["
				  << static_cast<uint32_t>(can_ipc_filter.can_filter_id[it])
				  << "].");
			return -RET_CODE_INVALID_ARG;
		}
	}

	for (uint32_t it = 0; it < can_ipc_filter.can_filter_id_cnt; it++) {
		auto id_result = can_ipc_receiver_filter->can_ipc_filter_id_map.emplace(
			can_ipc_filter.can_filter_id[it], 1);
		// if return false, means this can id already exist, so increase the count value by
		// 1
		if (id_result.second == false) {
			id_result.first->second += 1;
		}
	}

	return RET_CODE_SUCCESS;
}

int CAN_IPC_RECEIVER::deregister_can_filter(const CAN_IPC_FILTER_T &can_ipc_filter)
{
	// 1: [can_ipc_filter_id_map] Based on the obtained CAN ID, reduce the count of the
	// corresponding CAN ID. If the CAN ID count is 0, erase this key.

	// 2: [can_ipc_filter_map] Based on the count of the obtained CAN ID, if the count of the
	// CAN ID is 0, erase this filter.

	if (can_ipc_filter.can_filter_id_cnt == 0) {
		LOG_ERROR("CAN Filter ID count is zero.");
		return -RET_CODE_INVALID_ARG;
	}

	if (can_ipc_filter.can_filter_id == nullptr) {
		LOG_ERROR("CAN Filter ID pointer is null.");
		return -RET_CODE_INVALID_ARG;
	}

	CAN_IPC_RECEIVER_FILTER_T *can_ipc_receiver_filter = this->return_can_ipc_receiver_filter(
		static_cast<CAN_DEV_PORT_E>(can_ipc_filter.can_port));
	if (can_ipc_receiver_filter == nullptr) {
		return -RET_CODE_INVALID_ARG;
	}

	for (uint32_t it = 0; it < can_ipc_filter.can_filter_id_cnt; it++) {
		auto id_it = can_ipc_receiver_filter->can_ipc_filter_id_map.find(
			can_ipc_filter.can_filter_id[it]);
		if ((id_it != can_ipc_receiver_filter->can_ipc_filter_id_map.end()) &&
		    (id_it->second != 0)) {
			id_it->second -= 1;
		}

		if ((id_it != can_ipc_receiver_filter->can_ipc_filter_id_map.end()) &&
		    (id_it->second == 0)) {
			auto filter_it = can_ipc_receiver_filter->can_ipc_filter_map.find(
				can_ipc_filter.can_filter_id[it]);
			if (filter_it != can_ipc_receiver_filter->can_ipc_filter_map.end()) {
				can_ipc_receiver_filter->can_ipc_filter_map.erase(
					can_ipc_filter.can_filter_id[it]);
				can_ipc_receiver_filter->can_ipc_filter_id_map.erase(
					can_ipc_filter.can_filter_id[it]);
				LOG_DEBUG("Erase CAN Filter with CAN id: " << static_cast<uint32_t>(
						  can_ipc_filter.can_filter_id[it]));
			}
		} else {
			return -RET_CODE_INVALID_ARG;
		}
	}

	return RET_CODE_SUCCESS;
}

int CAN_IPC_RECEIVER::match_can_filter(CAN_DEV_PORT_E can_port,
				       const struct can_frame_t raw_can_frame)
{
	// if raw can id matched can filter, add the corresponding filter's callback function to the
	// work queue.

	CAN_IPC_RECEIVER_FILTER_T *can_ipc_receiver_filter =
		this->return_can_ipc_receiver_filter(static_cast<CAN_DEV_PORT_E>(can_port));
	if (can_ipc_receiver_filter == nullptr) {
		return -RET_CODE_INVALID_ARG;
	}

	auto it = can_ipc_receiver_filter->can_ipc_filter_map.find(raw_can_frame.id);
	if (it != can_ipc_receiver_filter->can_ipc_filter_map.end()) {
		// add work queue
		CAN_IPC_FILTER_T &filter = it->second;

		std::function<void()> cb_func = [filter, raw_can_frame]() {
			can_frame_t cb_frame = raw_can_frame;
			filter.can_filter_callback(&cb_frame, filter.socket_index);
		};

		this->work_queue_handle->enqueue(cb_func);
	}

	return RET_CODE_SUCCESS;
}

// TODO: modified as smart pointer to avoid memory leak
void CAN_IPC_RECEIVER::get_raw_can_data(const can_port_target_t &can_port_target)
{
// single reception of multiple can frames

// 16000/80 = 200 packets
#define SINGLE_RX_BUF_SIZE 16000

	struct pack_info pack = {
		.soc_ts = static_cast<uint64_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch())
				.count()),
		.data_num = 0,
		.mcu_ts = static_cast<uint64_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch())
				.count()),
		.length = SINGLE_RX_BUF_SIZE / sizeof(struct canframe),
		.unused = 0,
		.unused_1 = 0,
	};

	struct canframe *rx_frame = (struct canframe *)malloc(SINGLE_RX_BUF_SIZE);

	if (rx_frame == NULL) {
		LOG_ERROR("malloc memory fail!");
		return;
	}

	memset(rx_frame, 0, SINGLE_RX_BUF_SIZE);
	canRecvMsgFrame(can_port_target.target_instance, rx_frame, &pack);

	// decode multiple can frames
	struct canframe *single_canframe = rx_frame;

	for (uint32_t i = 0; i < pack.data_num; i++) {

		if (single_canframe->len == 0)
			continue;

		if (single_canframe != NULL) {
			struct can_frame_t can_frame = {
				.id = single_canframe->canid,
				.dlc = can_bytes_to_dlc(single_canframe->len),
				.flags = single_canframe->can_type,
				.data = 0,
			};

			memcpy(can_frame.data, single_canframe->data, single_canframe->len);

			this->match_can_filter(
				static_cast<CAN_DEV_PORT_E>(can_port_target.can_port_index),
				can_frame);

			single_canframe++;
		}
	}

	free(rx_frame);
}

void CAN_IPC_RECEIVER::work_queue_task()
{
	while (this->work_task_queue_running_.load()) {

		//! for test
		LOG_DEBUG("Current work queue task count: ["
			  << static_cast<int>(this->work_queue_handle->size()) << "].");

		// set the scope for the mutex lock
		{
			std::unique_lock<std::mutex> lock(this->work_task_queue_paused_mutex_);
			this->work_task_queue_paused_condition_.wait(lock, [this]() {
				// if task paused, blocking task.
				return (!this->work_task_queue_paused_);
			});
		}

		std::function<void()> task = this->work_queue_handle->dequeue_blocking();

		if (this->work_task_queue_running_.load() && task) {
			try {
				task();
			} catch (const std::exception &e) {
				LOG_ERROR("Callback exception in CAN_IPC_RECEIVER: [" << e.what()
										      << "].");
			}
		}

		// std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void CAN_IPC_RECEIVER::receive_ipc_can_task()
{
	char ctarget_can_port5_instance[16];
	char ctarget_can_port6_instance[16];
	char ctarget_can_port7_instance[16];
	char ctarget_can_port8_instance[16];
	char ctarget_can_port9_instance[16];

	strcpy(ctarget_can_port5_instance,
	       this->get_can_ipc_port_5_instance()->can_port_instance.c_str());
	can_port_target_t can_port_5_target = {
		.target_instance = ctarget_can_port5_instance,
		.can_port_index =
			static_cast<uint8_t>(this->get_can_ipc_port_5_instance()->can_dev_port),
	};

	strcpy(ctarget_can_port6_instance,
	       this->get_can_ipc_port_6_instance()->can_port_instance.c_str());
	can_port_target_t can_port_6_target = {
		.target_instance = ctarget_can_port6_instance,
		.can_port_index =
			static_cast<uint8_t>(this->get_can_ipc_port_6_instance()->can_dev_port),
	};

	strcpy(ctarget_can_port7_instance,
	       this->get_can_ipc_port_7_instance()->can_port_instance.c_str());
	can_port_target_t can_port_7_target = {
		.target_instance = ctarget_can_port7_instance,
		.can_port_index =
			static_cast<uint8_t>(this->get_can_ipc_port_7_instance()->can_dev_port),
	};

	strcpy(ctarget_can_port8_instance,
	       this->get_can_ipc_port_8_instance()->can_port_instance.c_str());
	can_port_target_t can_port_8_target = {
		.target_instance = ctarget_can_port8_instance,
		.can_port_index =
			static_cast<uint8_t>(this->get_can_ipc_port_8_instance()->can_dev_port),
	};

	strcpy(ctarget_can_port9_instance,
	       this->get_can_ipc_port_9_instance()->can_port_instance.c_str());
	can_port_target_t can_port_9_target = {
		.target_instance = ctarget_can_port9_instance,
		.can_port_index =
			static_cast<uint8_t>(this->get_can_ipc_port_9_instance()->can_dev_port),
	};

	while (this->receive_ipc_can_task_running_.load()) {
		// set the scope for the mutex lock
		{
			std::unique_lock<std::mutex> lock(this->receive_ipc_can_task_paused_mutex_);
			this->receive_ipc_can_task_paused_condition_.wait(lock, [this]() {
				// if task paused, blocking task.
				return (!this->receive_ipc_can_task_paused_);
			});
		}

		if (this->is_received_can_5_port.load()) {
			this->get_raw_can_data(can_port_5_target);
		}
		if (this->is_received_can_6_port.load()) {
			this->get_raw_can_data(can_port_6_target);
		}
		if (this->is_received_can_7_port.load()) {
			this->get_raw_can_data(can_port_7_target);
		}
		if (this->is_received_can_8_port.load()) {
			this->get_raw_can_data(can_port_8_target);
		}
		if (this->is_received_can_9_port.load()) {
			this->get_raw_can_data(can_port_9_target);
		}
		// std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void CAN_IPC_RECEIVER::create_work_queue_task()
{
	this->work_task_queue_paused_.store(false);
	this->work_task_queue_running_.store(true);
	this->get_work_queue_task() = std::thread(&CAN_IPC_RECEIVER::work_queue_task, this);
	this->get_work_queue_task().detach();
}

void CAN_IPC_RECEIVER::create_receive_ipc_can_task()
{
	this->receive_ipc_can_task_paused_.store(false);
	this->receive_ipc_can_task_running_.store(true);
	this->get_receive_ipc_can_task() =
		std::thread(&CAN_IPC_RECEIVER::receive_ipc_can_task, this);
	this->get_receive_ipc_can_task().detach();
}

void CAN_IPC_RECEIVER::pause_work_queue_task()
{
	this->work_task_queue_paused_.store(true);
}

void CAN_IPC_RECEIVER::pause_receive_ipc_can_task()
{
	this->receive_ipc_can_task_paused_.store(true);
}

void CAN_IPC_RECEIVER::resume_work_queue_task()
{
	{
		std::lock_guard<std::mutex> lock(this->work_task_queue_paused_mutex_);
		this->work_task_queue_paused_.store(false);
	}
	this->work_task_queue_paused_condition_.notify_one();
}

void CAN_IPC_RECEIVER::resume_receive_ipc_can_task()
{
	{
		std::lock_guard<std::mutex> lock(this->receive_ipc_can_task_paused_mutex_);
		this->receive_ipc_can_task_paused_.store(false);
	}
	this->receive_ipc_can_task_paused_condition_.notify_one();
}

void CAN_IPC_RECEIVER::enable_can_receiver_port(const CAN_IPC_CONFIG_T *can_ipc_config)
{
	std::unique_ptr<CAN_IPC_CONFIG_T> can_ipc_port_config =
		std::make_unique<CAN_IPC_CONFIG_T>(*can_ipc_config);
	switch (can_ipc_config->can_dev_port) {
	case CAN_DEV_PORT_E::CAN_DEV_PORT_5: {
		this->is_received_can_5_port.store(true);
		this->get_can_ipc_port_5_instance() = std::move(can_ipc_port_config);
		break;
	}
	case CAN_DEV_PORT_E::CAN_DEV_PORT_6: {
		this->is_received_can_6_port.store(true);
		this->get_can_ipc_port_6_instance() = std::move(can_ipc_port_config);
		break;
	}
	case CAN_DEV_PORT_E::CAN_DEV_PORT_7: {
		this->is_received_can_7_port.store(true);
		this->get_can_ipc_port_7_instance() = std::move(can_ipc_port_config);
		break;
	}
	case CAN_DEV_PORT_E::CAN_DEV_PORT_8: {
		this->is_received_can_8_port.store(true);
		this->get_can_ipc_port_8_instance() = std::move(can_ipc_port_config);
		break;
	}
	case CAN_DEV_PORT_E::CAN_DEV_PORT_9: {
		this->is_received_can_9_port.store(true);
		this->get_can_ipc_port_9_instance() = std::move(can_ipc_port_config);
		break;
	}
	// default as can 5 port
	default: {
		this->is_received_can_5_port.store(true);
		this->get_can_ipc_port_5_instance() = std::move(can_ipc_port_config);
		break;
	}
	}
}

void CAN_IPC_RECEIVER::start_can_ipc_receiver()
{
	this->work_queue_handle->clear_queue();
	this->create_work_queue_task();
	this->create_receive_ipc_can_task();
}

void CAN_IPC_RECEIVER::pause_can_ipc_receiver()
{
	this->pause_receive_ipc_can_task();
	this->pause_work_queue_task();
}

void CAN_IPC_RECEIVER::resume_can_ipc_receiver()
{
	this->resume_work_queue_task();
	this->resume_receive_ipc_can_task();
}