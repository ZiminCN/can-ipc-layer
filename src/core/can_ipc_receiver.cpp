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
#include "ret_code_def.h"

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
	can_ipc_receiver_filter.can_ipc_filter_mask_map.clear();
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
	// of the CAN ID 3: [can_ipc_filter_mask_map] emplace the CAN MASK, if return error,
	// increase the count value of the CAN MASK

	CAN_IPC_RECEIVER_FILTER_T *can_ipc_receiver_filter = this->return_can_ipc_receiver_filter(
		static_cast<CAN_DEV_PORT_E>(can_ipc_filter.can_port));
	if (can_ipc_receiver_filter == nullptr) {
		return -RET_CODE_INVALID_ARG;
	}

	auto filter_result = can_ipc_receiver_filter->can_ipc_filter_map.emplace(
		can_ipc_filter.can_filter_id, can_ipc_filter);
	if (filter_result.second == false) {
		LOG_ERROR("Add CAN Filter failed, the Filter CAN ID is: ["
			  << static_cast<uint32_t>(can_ipc_filter.can_filter_id)
			  << "], CAN Mask is: ["
			  << static_cast<uint32_t>(can_ipc_filter.can_filter_mask) << "].");
		return -RET_CODE_INVALID_ARG;
	}

	auto id_result = can_ipc_receiver_filter->can_ipc_filter_id_map.emplace(
		can_ipc_filter.can_filter_mask, 1);
	LOG_DEBUG("id_result.mask: " << static_cast<uint32_t>(can_ipc_filter.can_filter_mask));
	LOG_DEBUG("id_result.count: " << static_cast<uint32_t>(
			  can_ipc_receiver_filter
				  ->can_ipc_filter_id_map[can_ipc_filter.can_filter_mask]));
	// if return false, means this can id already exist, so increase the count value by 1
	if (id_result.second == false) {
		id_result.first->second += 1;
	}

	auto mask_result = can_ipc_receiver_filter->can_ipc_filter_mask_map.emplace(
		can_ipc_filter.can_filter_mask, 1);
	LOG_DEBUG("mask_result.mask: " << static_cast<uint32_t>(can_ipc_filter.can_filter_mask));
	LOG_DEBUG("mask_result.count: " << static_cast<uint32_t>(
			  can_ipc_receiver_filter
				  ->can_ipc_filter_mask_map[can_ipc_filter.can_filter_mask]));
	// if return false, means this can id already exist, so increase the count value by 1
	if (mask_result.second == false) {
		mask_result.first->second += 1;
	}

	return RET_CODE_SUCCESS;
}

int CAN_IPC_RECEIVER::deregister_can_filter(const CAN_IPC_FILTER_T &can_ipc_filter)
{
	// 1: [can_ipc_filter_mask_map] Based on the obtained CAN ID, reduce the count of the
	// corresponding CAN MASK. If the CAN MASK count is 0, erase this key.

	// 2: [can_ipc_filter_id_map] Based on the obtained CAN ID, reduce the count of the
	// corresponding CAN ID. If the CAN ID count is 0, erase this key.

	// 3: [can_ipc_filter_map] Based on the count of the obtained CAN ID, if the count of the
	// CAN ID is 0, erase this filter.

	CAN_IPC_RECEIVER_FILTER_T *can_ipc_receiver_filter = this->return_can_ipc_receiver_filter(
		static_cast<CAN_DEV_PORT_E>(can_ipc_filter.can_port));
	if (can_ipc_receiver_filter == nullptr) {
		return -RET_CODE_INVALID_ARG;
	}

	auto mask_it = can_ipc_receiver_filter->can_ipc_filter_mask_map.find(
		can_ipc_filter.can_filter_mask);
	if ((mask_it != can_ipc_receiver_filter->can_ipc_filter_mask_map.end()) &&
	    (mask_it->second != 0)) {
		mask_it->second -= 1;
	} else if ((mask_it != can_ipc_receiver_filter->can_ipc_filter_mask_map.end()) &&
		   (mask_it->second == 0)) {
		can_ipc_receiver_filter->can_ipc_filter_mask_map.erase(
			can_ipc_filter.can_filter_mask);
		LOG_INFO("Erase can mask: "
			 << static_cast<uint32_t>(can_ipc_filter.can_filter_mask));
	} else {
		return -RET_CODE_INVALID_ARG;
	}

	auto id_it =
		can_ipc_receiver_filter->can_ipc_filter_id_map.find(can_ipc_filter.can_filter_id);
	if ((id_it != can_ipc_receiver_filter->can_ipc_filter_id_map.end()) &&
	    (id_it->second != 0)) {
		id_it->second -= 1;
	}

	if ((id_it != can_ipc_receiver_filter->can_ipc_filter_id_map.end()) &&
	    (id_it->second == 0)) {
		auto filter_it = can_ipc_receiver_filter->can_ipc_filter_map.find(
			can_ipc_filter.can_filter_id);
		if (filter_it != can_ipc_receiver_filter->can_ipc_filter_map.end()) {
			can_ipc_receiver_filter->can_ipc_filter_map.erase(
				can_ipc_filter.can_filter_id);
			can_ipc_receiver_filter->can_ipc_filter_id_map.erase(
				can_ipc_filter.can_filter_id);
			LOG_DEBUG("Erase CAN Filter with CAN id: "
				  << static_cast<uint32_t>(can_ipc_filter.can_filter_id));
		}
	} else {
		return -RET_CODE_INVALID_ARG;
	}

	return RET_CODE_SUCCESS;
}

int CAN_IPC_RECEIVER::match_can_filter(uint32_t can_port, uint32_t raw_can_id)
{
	(void)can_port;
	(void)raw_can_id;
	// 1: raw_can_id & all CAN MASK to match the CAN ID.
	// 2: if matched, add the corresponding filter's callback function to the work queue.

	// uint32_t match_can_id = 0x00U;

	// for (auto &pair : can_ipc_filter_mask_map) {
	// 	match_can_id = (raw_can_id) & (pair.first);
	// 	auto it = can_ipc_filter_map.find(match_can_id);
	// 	if (it != can_ipc_filter_map.end()) {
	// 		// add work queue
	// 	}
	// }

	return RET_CODE_SUCCESS;
}
