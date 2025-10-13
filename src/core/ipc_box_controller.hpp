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
#ifndef __IPC_BOX_CONTROLLER_H__
#define __IPC_BOX_CONTROLLER_H__

#include "message_log.hpp"
#include <iostream>
#include <memory>

class IPC_BOX_CONTROLLER
{
      public:
	IPC_BOX_CONTROLLER()
	{
		LOG_DEBUG("IPC_BOX_CONTROLLER impl init.");
	};
	~IPC_BOX_CONTROLLER() = default;
	static std::unique_ptr<IPC_BOX_CONTROLLER> &getInstance()
	{
		static std::unique_ptr<IPC_BOX_CONTROLLER> Instance =
			std::make_unique<IPC_BOX_CONTROLLER>();
		return Instance;
	};

      private:
};

#endif // __IPC_BOX_CONTROLLER_H__