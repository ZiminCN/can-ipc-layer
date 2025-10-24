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

cd deb_build/usr/local/lib/can_ipc_layer
ln -s libcan_ipc_layer.so.0.0.3 libcan_ipc_layer.so.0
ln -s libcan_ipc_layer.so.0 libcan_ipc_layer.so
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
# 更新动态库缓存
ldconfig
EOF
chmod +x deb_build/DEBIAN/postinst

cat > deb_build/DEBIAN/prerm << EOF
#!/bin/bash
# 清理符号链接
rmdir /usr/local/lib/can_ipc_layer 2>/dev/null || true
rmdir /usr/local/include/can_ipc_layer 2>/dev/null || true
ldconfig
EOF
chmod +x deb_build/DEBIAN/prerm

dpkg-deb --build deb_build ${DEB_PKG_NAME}_${DEB_PKG_VERSION}_arm64.deb

rm -rf deb_build

echo "DEB package ${DEB_PKG_NAME}_${DEB_PKG_VERSION}_arm64.deb created successfully."