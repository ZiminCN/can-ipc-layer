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

#ifndef __CAN_IPC_SENDER_HPP__
#define __CAN_IPC_SENDER_HPP__
#include <iostream>
#include <memory>
#include "message_log.hpp"

class CAN_IPC_SENDER
{
      public:
	CAN_IPC_SENDER()
	{
		LOG_DEBUG("CAN_IPC_SENDER impl init.");
	};
	~CAN_IPC_SENDER() = default;
	static std::unique_ptr<CAN_IPC_SENDER> getInstance();
	void test_send_can_frame();

      private:
	static std::unique_ptr<CAN_IPC_SENDER> Instance;
};

#endif // __CAN_IPC_SENDER_HPP__