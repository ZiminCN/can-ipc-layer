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
VALUE_CAN_HAL_DEBUG_LEVEL=6

if [ ! -d "build" ]; then
    echo "Build directory not found. Please build the project first."
    exit 1
fi


if [ -d "deb_build" ]; then
    echo "There is a folder with the deb_build name. Please clean up this folder first."
    exit 1
fi

LIB_FILE=$(find build -name "libcan_ipc_layer.so.*.*.*" | head -1)
if [ -z "$LIB_FILE" ]; then
    echo "Shared object file not found in build directory."
    exit 1
fi

BIN_DIR=$(find build -type d -name "bin" | head -1)
if [ -z "$BIN_DIR" ]; then
    echo "bin directory not found in build directory."
    exit 1
fi

DEB_PKG_NAME="can-ipc-layer"
DEB_PKG_VERSION=$(basename "$LIB_FILE" | sed 's/libcan_ipc_layer.so.//')
MAINTAINER="Zi Min <jianming.zeng@directdrivetech.com>"

echo "Building DEB Version: $DEB_PKG_VERSION"

rm -rf deb_build
mkdir -p deb_build/DEBIAN
mkdir -p deb_build/usr/local/lib/can_ipc_layer
mkdir -p deb_build/usr/local/include/can_ipc_layer

cp "$LIB_FILE" deb_build/usr/local/lib/can_ipc_layer/
cp -r include/can_ipc_layer/* deb_build/usr/local/include/can_ipc_layer/
cp -r config/ deb_build/usr/local/include/can_ipc_layer/
cp -r "$BIN_DIR" deb_build/usr/local/lib/can_ipc_layer/

cd deb_build/usr/local/lib/can_ipc_layer
LIB_BASENAME=$(basename "$LIB_FILE")
MAJOR_VERSION=$(echo "$LIB_BASENAME" | sed 's/libcan_ipc_layer\.so\.\([0-9]\+\)\..*/\1/')
ln -s "$LIB_BASENAME" "libcan_ipc_layer.so.$MAJOR_VERSION"
ln -s "libcan_ipc_layer.so.$MAJOR_VERSION" "libcan_ipc_layer.so"
cd - > /dev/null

cat > deb_build/DEBIAN/control << EOF
Package: $DEB_PKG_NAME
Version: $DEB_PKG_VERSION
Section: libs
Priority: optional
Architecture: arm64
Maintainer: $MAINTAINER
Description: CAN Bus IPC Communication Layer Library
 CAN bus inter-process communication library for embedded systems.
EOF

if [ ! -f "deb_build/DEBIAN/control" ]; then
    echo "Failed to create control file."
    exit 1
fi

cat > deb_build/DEBIAN/postinst << EOF
#!/bin/bash
if [ -f "/usr/lib/systemd/system/ipc_can_socket_server.service" ]; then
    systemctl stop ipc_can_socket_server.service
    systemctl disable ipc_can_socket_server.service
    # rm -f /usr/lib/systemd/system/ipc_can_socket_server.service
    echo "clean old files"
fi
cp /usr/local/lib/can_ipc_layer/ipc_can_socket_server.service /usr/lib/systemd/system/
# 更新动态库缓存
echo "/usr/local/lib/can_ipc_layer" > /etc/ld.so.conf.d/can_ipc_layer.conf
ldconfig
systemctl daemon-reload || true
systemctl start ipc_can_socket_server.service || true
systemctl enable ipc_can_socket_server.service || true
EOF
chmod +x deb_build/DEBIAN/postinst

cat > deb_build/DEBIAN/prerm << EOF
#!/bin/bash
# 清理符号链接
systemctl stop ipc_can_socket_server.service || true
systemctl disable ipc_can_socket_server.service || true
rm -f /usr/lib/systemd/system/ipc_can_socket_server.service
rm -f /etc/ld.so.conf.d/can_ipc_layer.conf
rm -rf /usr/local/lib/can_ipc_layer/
rm -rf /usr/local/include/can_ipc_layer/
rmdir /usr/local/lib/can_ipc_layer/* 2>/dev/null || true
rmdir /usr/local/include/can_ipc_layer/* 2>/dev/null || true
ldconfig
systemctl daemon-reload || true
EOF

chmod +x deb_build/DEBIAN/prerm

cat > deb_build/usr/local/lib/can_ipc_layer/create_icp_can_socket_server.sh << EOF
export CAN_HAL_DEBUG_LEVEL=$VALUE_CAN_HAL_DEBUG_LEVEL
exec /usr/local/lib/can_ipc_layer/bin/can_ipc_socket_daemon
EOF

cat > deb_build/usr/local/lib/can_ipc_layer/ipc_can_socket_server.service << EOF
[Unit]
Description=IPC CAN Server Service
After=network.target

[Service]
Type=simple
# 替换为你的实际用户（非root更安全）
User=sunrise
# 替换为你的frpc实际路径
WorkingDirectory=/usr/local/lib/can_ipc_layer/
ExecStart=/bin/bash /usr/local/lib/can_ipc_layer/create_icp_can_socket_server.sh
Restart=on-failure
RestartSec=30
TimeoutStopSec=3
KillMode=process
SendSIGKILL=yes

# 日志配置（可选但推荐）
StandardOutput=journal
StandardError=journal
SyslogIdentifier=frpc

[Install]
WantedBy=multi-user.target
EOF

chmod +x deb_build/usr/local/lib/can_ipc_layer/create_icp_can_socket_server.sh


dpkg-deb --build deb_build ${DEB_PKG_NAME}_${DEB_PKG_VERSION}_arm64.deb

rm -rf deb_build

echo "DEB package ${DEB_PKG_NAME}_${DEB_PKG_VERSION}_arm64.deb created successfully."