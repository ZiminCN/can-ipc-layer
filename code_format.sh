#!bin/bash

clang-format -i debug/*.c*
clang-format -i include/can_ipc_layer/*.h*
clang-format -i src/api/*.c*
clang-format -i src/core/*.h*
clang-format -i src/core/*.c*
clang-format -i src/utils/*.h*
clang-format -i src/utils/*.c*

