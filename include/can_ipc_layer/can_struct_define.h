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
#ifndef __CAN_STRUCT_DEFINE_HPP__
#define __CAN_STRUCT_DEFINE_HPP__

#include <cstdint>

/**
 * @brief CAN port enumeration
 * @note Select a CAN port from the S100 MCU expansion board
 * @note Default value is CAN_PORT_5
 */
enum class CAN_PORT_E {
	CAN_PORT_5 = 0,
	CAN_PORT_6,
	CAN_PORT_7,
	CAN_PORT_8,
	CAN_PORT_9,
};

/**
 * @name CAN frame definitions
 * @{
 */

/**
 * @brief Bit mask for a standard (11-bit) CAN identifier.
 */
#define CAN_STD_ID_MASK 0x7FFU

/**
 * @brief Bit mask for an extended (29-bit) CAN identifier.
 */
#define CAN_EXT_ID_MASK 0x1FFFFFFFU

/**
 * @brief Maximum data length code for CAN 2.0A/2.0B.
 */
#define CAN_MAX_DLC 8U

/**
 * @brief Maximum data length code for CAN FD.
 */
#define CANFD_MAX_DLC 15U

/**
 * @cond INTERNAL_HIDDEN
 * Internally calculated maximum data length
 */
#define CAN_MAX_DLEN   8U
#define CANFD_MAX_DLEN 64U

/**
 * @name CAN frame flags
 * @anchor CAN_FRAME_FLAGS
 *
 * @{
 */

/** Normal mode. */
#define CAN_MODE_NORMAL 0

/** Controller allows transmitting/receiving CAN FD frames. */
#define CAN_MODE_FD 1

/** @} */

#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))

#define __ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/**
 * @brief Convert from Data Length Code (DLC) to the number of data bytes
 *
 * @param dlc Data Length Code (DLC).
 *
 * @retval Number of bytes.
 */
static inline uint8_t can_dlc_to_bytes(uint8_t dlc)
{
	static const uint8_t dlc_table[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};

	return dlc_table[MIN(dlc, __ARRAY_SIZE(dlc_table) - 1)];
}

/**
 * @brief Convert from number of bytes to Data Length Code (DLC)
 *
 * @param num_bytes Number of bytes.
 *
 * @retval Data Length Code (DLC).
 */
static inline uint8_t can_bytes_to_dlc(uint8_t num_bytes)
{
	return num_bytes <= 8	 ? num_bytes
	       : num_bytes <= 12 ? 9
	       : num_bytes <= 16 ? 10
	       : num_bytes <= 20 ? 11
	       : num_bytes <= 24 ? 12
	       : num_bytes <= 32 ? 13
	       : num_bytes <= 48 ? 14
				 : 15;
}

/**
 * @brief CAN frame structure
 */
struct can_frame_t {
	/** Standard (11-bit) or extended (29-bit) CAN identifier. */
	uint32_t id;
	/** Data Length Code (DLC) indicating data length in bytes. */
	uint8_t dlc;
	/** Flags. @see @ref CAN_FRAME_FLAGS. */
	uint8_t flags;

	/** The frame payload data. */
	union {
		/** Payload data accessed as unsigned 8 bit values. */
		uint8_t data[CANFD_MAX_DLEN];
		/** Payload data accessed as unsigned 32 bit values. */
		uint32_t data_32[DIV_ROUND_UP(CANFD_MAX_DLEN, sizeof(uint32_t))];
	};
};

/**
 * @brief CAN filter structure
 */
struct can_filter_t {
	/** CAN identifier to match. */
	uint32_t id;
	/** CAN identifier matching mask. If a bit in this mask is 0, the value
	 * of the corresponding bit in the ``id`` field is ignored by the filter.
	 */
	uint32_t mask;
	/** Flags. @see @ref CAN_FILTER_FLAGS. */
	uint8_t flags;
};

/**
 * @brief Defines the application callback handler function signature for receiving.
 *
 * @param dev       Pointer to the device structure for the driver instance.
 * @param frame     Received frame.
 * @param user_data User data provided when the filter was added.
 */
typedef void (*can_rx_callback_t)(CAN_PORT_E can_port, struct can_frame_t *frame, void *user_data);

#endif // __CAN_STRUCT_DEFINE_HPP__