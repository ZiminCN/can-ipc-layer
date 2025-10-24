# Copyright (c) Direct Drive Technology Co., Ltd. All rights reserved.
# Author: Zi Min <jianming.zeng@directdrivetech.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#!bin/bash

CURRENT_DIR=$(pwd)
SAFE_DEBUG_DIR="/home/sunrise/rdk-s100-ws/can_ipc_abstraction_layer"
echo "当前目录: $CURRENT_DIR"
echo "目标目录: $SAFE_DEBUG_DIR"

if [ "$CURRENT_DIR" != "$SAFE_DEBUG_DIR" ]; then
    echo "错误：当前目录与目标目录不一致！"
    echo "请切换到目录: $SAFE_DEBUG_DIR"
    exit 1
fi

rm -rf ./build
rm -rf ./debug/build

echo "清理完成"
