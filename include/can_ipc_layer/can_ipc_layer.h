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

#ifndef __LIBCAN_IPC_LAYER_H__
#define __LIBCAN_IPC_LAYER_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "can_struct_define.h"

typedef struct can_ipc_layer_handle_t *can_ipc_layer_handle;

// C接口函数

/**
 * @brief Create a CAN IPC layer handle.
 * @return A pointer to the created handle, or nullptr on failure.
 */
can_ipc_layer_handle can_ipc_layer_create();

/**
 * @brief Destroy a CAN IPC layer handle.
 * @param handle A pointer to the handle to be destroyed.
 */
void can_ipc_layer_destroy(can_ipc_layer_handle_t **handle);

/**
 * @brief Send CAN frame data.
 * @param handle A pointer to the CAN IPC layer handle.
 * @param can_port The CAN port to send data on.
 * @param frame A pointer to the CAN frame data to be sent.
 * @return 0 on success, negative value on failure. refer to @see @arg RET_CODE_DEF
 */
int can_send(can_ipc_layer_handle_t **handle, const CAN_PORT_E can_port, const can_frame_t *frame);

/**
 * @brief Add CAN filter.
 * @param handle A pointer to the CAN IPC layer handle.
 * @param can_filter CAN filter structure
 * @param can_rx_callback CAN filter application callback handle
 * @return 0 on success, negative value on failure. refer to @see @arg RET_CODE_DEF
 */
int can_add_filter(can_ipc_layer_handle_t **handle, const can_filter_t *can_filter,
		   const can_rx_callback_t can_rx_callback);

/**
 * @brief Remove CAN filter.
 * @param handle A pointer to the CAN IPC layer handle.
 * @param can_filter CAN filter structure
 * @return 0 on success, negative value on failure. refer to @see @arg RET_CODE_DEF
 */
int can_remove_filter(can_ipc_layer_handle_t **handle, const can_filter_t *can_filter);

//! test func
void test_can_send(can_ipc_layer_handle_t **handle);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __LIBCAN_IPC_LAYER_H__