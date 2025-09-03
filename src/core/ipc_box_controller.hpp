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

#ifndef __IPC_BOX_CONTROLLER_H__
#define __IPC_BOX_CONTROLLER_H__

#include <memory>

class IPC_BOX_CONTROLLER
{
      public:
	IPC_BOX_CONTROLLER() = default;
	~IPC_BOX_CONTROLLER() = default;
	static std::unique_ptr<IPC_BOX_CONTROLLER> getInstance();

      private:
	static std::unique_ptr<IPC_BOX_CONTROLLER> Instance;
};

#endif // __IPC_BOX_CONTROLLER_H__