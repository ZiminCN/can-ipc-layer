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
#ifndef __CAN_STRUCT_INTERNAL_DEFINE_HPP__
#define __CAN_STRUCT_INTERNAL_DEFINE_HPP__

/** @defgroup CAN_IPC_INS0_CANX_CHANX CAN IPC channel name
 * @{
 */
// channel_name is channel name of can-ipc. For example, "can5_ins0ch4" or
// "bypass". Please refer to the following reference website
// 'https://developer.d-robotics.cc/rdk_doc/rdk_s/Advanced_development/mcu_development/S100/mcu_ipc/'
// for this name.
#define CAN_IPC_INS0_CAN5_CHAN4 "can5_ins0ch4"
#define CAN_IPC_INS0_CAN6_CHAN6 "can6_ins0ch6"
#define CAN_IPC_INS0_CAN7_CHAN7 "can7_ins0ch7"
#define CAN_IPC_INS0_CAN8_CHAN2 "can8_ins0ch2"
#define CAN_IPC_INS0_CAN9_CHAN3 "can9_ins0ch3"
/**
 * @}
 */

/** @defgroup CAN_DEV_PORT_E CAN DEV Port Enumeration
 * @{
 */
typedef enum {
	CAN_DEV_PORT_5 = 5,
	CAN_DEV_PORT_6 = 6,
	CAN_DEV_PORT_7 = 7,
	CAN_DEV_PORT_8 = 8,
	CAN_DEV_PORT_9 = 9,
} CAN_DEV_PORT_E;
/**
 * @}
 */

/**
 * @brief config CAN IPC instance port
 * @param can_port_instance CAN IPC instance port name, refer to @see @arg
 * CAN_IPC_INS0_CANX_CHANX
 * @param can_dev_port CAN device port, refer to @see @arg CAN_DEV_PORT_E
 * @note  Default value is CAN_IPC_INSTANCE_CAN5
 */
struct CAN_IPC_CONFIG_T {
	std::string can_port_instance;
	CAN_DEV_PORT_E can_dev_port;

	CAN_IPC_CONFIG_T()
		: can_port_instance(CAN_IPC_INS0_CAN5_CHAN4),
		  can_dev_port(CAN_DEV_PORT_E::CAN_DEV_PORT_5)
	{
	}

	CAN_IPC_CONFIG_T(const std::string &instance, const CAN_DEV_PORT_E &can_port)
		: can_port_instance(instance), can_dev_port(can_port)
	{
	}
};

#endif // __CAN_STRUCT_INTERNAL_DEFINE_HPP__