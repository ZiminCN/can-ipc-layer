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

#ifndef __CAN_IPC_LAYER_IMPL_HPP__
#define __CAN_IPC_LAYER_IMPL_HPP__
#include "can_ipc_receiver.hpp"
#include "can_ipc_sender.hpp"
#include "ipc_box_controller.hpp"
#include <iostream>
#include <memory>

class CAN_IPC_LAYER_IMPL
{
      public:
	CAN_IPC_LAYER_IMPL() = default;
	~CAN_IPC_LAYER_IMPL() = default;
	static std::unique_ptr<CAN_IPC_LAYER_IMPL> getInstance();

      private:
	static std::unique_ptr<CAN_IPC_LAYER_IMPL> Instance;
	std::unique_ptr<CAN_IPC_RECEIVER> can_ipc_receiver_handle = CAN_IPC_RECEIVER::getInstance();
	std::unique_ptr<CAN_IPC_SENDER> can_ipc_sender_handle = CAN_IPC_SENDER::getInstance();
	std::unique_ptr<IPC_BOX_CONTROLLER> ipc_box_controller_handle =
		IPC_BOX_CONTROLLER::getInstance();
};

#endif // __CAN_IPC_LAYER_IMPL_HPP__