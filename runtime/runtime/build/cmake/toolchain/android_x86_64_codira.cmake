# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

# Toolchain
set(CMAKE_SYSTEM_NAME Linux)
string(TOLOWER "${CMAKE_HOST_SYSTEM_NAME}" cmake_host_system_name)
set(ANDROID_ROOT "$ENV{ANDROID_ROOT}")
set(ANDROID_ARCH "$ENV{ANDROID_ARCH}")
set(ANDROID_OS "linux")
set(ANDROID_TARGET_API "$ENV{ANDROID_TARGET_API}")
set(CMAKE_C_COMPILER "${ANDROID_ROOT}/llvm/prebuilt/${cmake_host_system_name}-x86_64/bin/${ANDROID_ARCH}-${ANDROID_OS}-${ANDROID_TARGET_API}-clang")
set(CMAKE_ASM_COMPILER "${ANDROID_ROOT}/llvm/prebuilt/${cmake_host_system_name}-x86_64/bin/${ANDROID_ARCH}-${ANDROID_OS}-${ANDROID_TARGET_API}-clang")
set(CMAKE_CXX_COMPILER "${ANDROID_ROOT}/llvm/prebuilt/${cmake_host_system_name}-x86_64/bin/${ANDROID_ARCH}-${ANDROID_OS}-${ANDROID_TARGET_API}-clang++")
set(CMAKE_AR "${ANDROID_ROOT}/llvm/prebuilt/${cmake_host_system_name}-x86_64/bin/llvm-ar")
set(CMAKE_RANLIB "${ANDROID_ROOT}/llvm/prebuilt/${cmake_host_system_name}-x86_64/bin/llvm-ranlib")

set(OS "linux")
set(CPU_FAMILY "x86")
set(CPU_CORE "x86_64")
set(PLATFORM_NAME "ANDROID_x86_64_cangjie")
# The compile flags for common
set(CMAKE_C_FLAGS
    "-Wall \
    -Wextra \
    -Wformat=2 \
    -Wpointer-arith \
    -Wdate-time \
    -Wfloat-equal \
    -Wswitch-default \
    -Wshadow \
    -Wvla \
    -Wunused \
    -Wundef \
    -m64 \
    -fno-strict-aliasing \
    -fno-omit-frame-pointer \
    -fgnu89-inline \
    -fsigned-char \
    -fno-common \
    -fno-exceptions \
    -fno-rtti \
    -fstack-protector-strong \
    -fPIC"
)

set(CMAKE_CXX_FLAGS
    "-Wall \
    -Wextra \
    -Wformat=2 \
    -Wpointer-arith \
    -Wdate-time \
    -Wfloat-equal \
    -Wswitch-default \
    -Wshadow \
    -Wvla \
    -Wunused \
    -Wundef \
    -m64 \
    -fno-strict-aliasing \
    -fno-omit-frame-pointer \
    -fsigned-char \
    -fno-common \
    -fno-exceptions \
    -fno-rtti \
    -fstack-protector-strong \
    -fPIC"
)

add_compile_definitions(
    "_LARGEFILE_SOURCE"
    "_FILE_OFFSET_BITS=64"
    "VOS_OS_VER=VOS_LINUX"
    "VOS_HARDWARE_PLATFORM=VOS_X86"
    "MRT_HARDWARE_PLATFORM=MRT_X86"
    "VOS_CPU_TYPE=VOS_ARM"
    "VOS_WORDSIZE=64"
    "__WORDSIZE=64"
    "VOS_BYTE_ORDER=VOS_LITTLE_ENDIAN"
    "USE___THREAD"
    "$<$<CONFIG:Debug>:VOS_BUILD_DEBUG=1>"
    "$<$<CONFIG:Release>:VOS_BUILD_RELEASE=1>"
    "TLS_COMMON_DYNAMIC"
    "CODIRA"
    "MRT_LINUX"
    "__ANDROID__"
)

# The link flags
set(CMAKE_SHARED_LINKER_FLAGS
    "-rdynamic \
    -Wl,-z,noexecstack \
    -Wl,-z,relro \
    -Wl,-z,now"
)

# The ar flags
set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> rcD <TARGET> <OBJECTS>")