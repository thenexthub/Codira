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

# toolchain for ohos_x86
set(CMAKE_SYSTEM_NAME Linux)

# platform
set(PLATFORM_NAME "ohos_x86_64_cangjie")

# environment info
set(OS "linux")
set(OS_VER "ohos")
set(CPU_CORE "x86_64")
set(CPU_FAMILY "x86")
set(CPU_TYPE "i686")
set(MEM_TYPE "mem")
set(BYTE_ORDER "le")
set(COMPILER "gnu")
set(FWD_PLATFORM "MCCA")

set(CMAKE_ASM_COMPILER_WORKS 1)
set(CMAKE_ASM_ABI_COMPILED 1)

string(TOLOWER "${CMAKE_HOST_SYSTEM_NAME}" cmake_host_system_name)
string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" cmake_host_system_processor)
if("${cmake_host_system_processor}" MATCHES "arm64")
    set(cmake_host_system_processor aarch64)
endif()
if("${cmake_host_system_processor}" MATCHES "amd64")
    set(cmake_host_system_processor x86_64)
endif()
set(EXECUTABLE_EXTENSION)
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set(EXECUTABLE_EXTENSION ".exe")
endif()
# build tools
set(CMAKE_C_COMPILER "$ENV{OHOS_ROOT}/prebuilts/clang/ohos/${cmake_host_system_name}-${cmake_host_system_processor}/llvm/bin/clang${EXECUTABLE_EXTENSION}")
set(CMAKE_ASM_COMPILER "$ENV{OHOS_ROOT}/prebuilts/clang/ohos/${cmake_host_system_name}-${cmake_host_system_processor}/llvm/bin/clang${EXECUTABLE_EXTENSION}")
set(CMAKE_CXX_COMPILER "$ENV{OHOS_ROOT}/prebuilts/clang/ohos/${cmake_host_system_name}-${cmake_host_system_processor}/llvm/bin/clang++${EXECUTABLE_EXTENSION}")
set(CMAKE_AR "$ENV{OHOS_ROOT}/prebuilts/clang/ohos/${cmake_host_system_name}-${cmake_host_system_processor}/llvm/bin/llvm-ar${EXECUTABLE_EXTENSION}")
set(CMAKE_RANLIB "$ENV{OHOS_ROOT}/prebuilts/clang/ohos/${cmake_host_system_name}-${cmake_host_system_processor}/llvm/bin/llvm-ranlib${EXECUTABLE_EXTENSION}")

# compile flags for common
set(CMAKE_C_FLAGS
    "-Wdeprecated-copy -fno-strict-aliasing --param=ssp-buffer-size=4  -flto -fvisibility=default -fsanitize=cfi -fno-sanitize=cfi-nvcall,cfi-icall \
     -Wno-builtin-macro-redefined -D__DATE__= -D__TIME__= -D__TIMESTAMP__= -funwind-tables -fcolor-diagnostics \
     -fmerge-all-constants -Xclang -mllvm -Xclang -instcombine-lower-dbg-declare=0 -no-canonical-prefixes \
     -ffunction-sections -fno-short-enums --target=x86_64-linux-ohos -Wextra -Wthread-safety \
     -fdata-sections -ffunction-sections -fno-omit-frame-pointer -g2 -ggnu-pubnames -fno-common -Wheader-hygiene \
     -Wstring-conversion -Wtautological-overlap-compare -fPIC -fgnu89-inline -Wfloat-equal"
)

set(CMAKE_CXX_FLAGS
    "-Wdeprecated-copy -fno-strict-aliasing --param=ssp-buffer-size=4  -flto -fvisibility=default -fsanitize=cfi -fno-sanitize=cfi-nvcall,cfi-icall \
     -Wno-builtin-macro-redefined -D__DATE__= -D__TIME__= -D__TIMESTAMP__= -funwind-tables -fcolor-diagnostics \
     -fmerge-all-constants -Xclang -mllvm -Xclang -instcombine-lower-dbg-declare=0 -no-canonical-prefixes \
     -ffunction-sections -fno-short-enums --target=x86_64-linux-ohos -Wextra -Wthread-safety \
     -fdata-sections -ffunction-sections -fno-omit-frame-pointer -g2 -ggnu-pubnames -fno-common -Wheader-hygiene \
     -Wstring-conversion -Wtautological-overlap-compare -fPIC -Wfloat-equal -fno-exceptions"
)

set(OHOS_INCLUDE "-I$ENV{OHOS_ROOT}/third_party/openssl/include")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OHOS_INCLUDE} --sysroot=$ENV{OHOS_ROOT}/out/sdk/obj/third_party/musl/sysroot")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OHOS_INCLUDE} --sysroot=$ENV{OHOS_ROOT}/out/sdk/obj/third_party/musl/sysroot")

if("${DEBUG_INFO}" STREQUAL "INFO")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

# if building stage is publish, add CMAKE_C_FLAGS
if("${BUILDING_STAGE}" STREQUAL "publish")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto -fvisibility=default -fsanitize=cfi -fno-sanitize=cfi-nvcall,cfi-icall -mbranch-protection=pac-ret")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -fvisibility=default -fsanitize=cfi -fno-sanitize=cfi-nvcall,cfi-icall -mbranch-protection=pac-ret")
endif()

# compile flags for debug version only
if("${BUILDING_STAGE}" STREQUAL "asan")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize-recover=address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-recover=address")
    set(CMAKE_C_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -gdwarf-4")
else()
    set(CMAKE_C_FLAGS_DEBUG "-g -Wframe-larger-than=2048")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -Wframe-larger-than=2048 -gdwarf-4")
endif()

# compile flags for release version only
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O2")

# assemble flags
set(CMAKE_ASM_FLAGS
    "${CMAKE_C_FLAGS}"
)

# Attention we need to clear CMAKE_ASM_FLAGS_DEBUG and CMAKE_ASM_FLAGS_RELEASE
# otherwise cmake will add some default compile option which we may not want

# assemble flags for debug version only
set(CMAKE_ASM_FLAGS_DEBUG "")

# assemble flags for release version only
set(CMAKE_ASM_FLAGS_RELEASE "")

# compile definitions
# ohos platform openssl need specific OPENSSL_ARM64_PLATFORM macro
add_compile_definitions(
    "_LARGEFILE_SOURCE"
    "_FILE_OFFSET_BITS=64"
    "VOS_OS_VER=VOS_LINUX"
    "VOS_HARDWARE_PLATFORM=VOS_X86"
    "MRT_HARDWARE_PLATFORM=MRT_X86"
    "VOS_CPU_TYPE=VOS_X86"
    "VOS_WORDSIZE=64"
    "VOS_BYTE_ORDER=VOS_LITTLE_ENDIAN"
    "USE___THREAD"
    "$<$<CONFIG:Debug>:VOS_BUILD_DEBUG=1>"
    "$<$<CONFIG:Release>:VOS_BUILD_RELEASE=1>"
    "TLS_COMMON_DYNAMIC"
    "CODIRA"
    "OPENSSL_X86_64_PLATFORM"
    "__OHOS__"
    "MRT_LINUX"
)

# link flags
set(CMAKE_SHARED_LINKER_FLAGS
    "-rdynamic \
    -Wl,-z,noexecstack \
    -Wl,-z,relro \
    -Wl,-z,now \
    -lpthread"
)

# ar flags
set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> rcD <TARGET> <OBJECTS>")
