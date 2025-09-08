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
