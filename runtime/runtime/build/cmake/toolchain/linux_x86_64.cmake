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

# toolchain for linux12_x86_64

# platform
set(PLATFORM_NAME "linux12_x86_64")

# environment info
set(OS "linux")

set(CPU_CORE "x86_64")
set(CPU_FAMILY "x86")
set(CPU_TYPE "i686")
set(MEM_TYPE "mem")
set(BYTE_ORDER "le")
set(COMPILER "gnu")

set(CMAKE_ASM_COMPILER_WORKS 1)
set(CMAKE_ASM_ABI_COMPILED 1)

# build tools
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

# compile flags for common
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
    -fstack-protector-strong \
    -fno-exceptions \
    -fPIC"
)

# if building stage is publish, add CMAKE_C_FLAGS
if("${BUILDING_STAGE}" STREQUAL "publish")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-strong")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
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
set(CMAKE_C_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")
set(CMAKE_CXX_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")

set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -D_FORTIFY_SOURCE=2 -O2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -D_FORTIFY_SOURCE=2 -O2")

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
   "MRT_LINUX"
)

# link flags
set(CMAKE_SHARED_LINKER_FLAGS
    "-m64 \
    -rdynamic \
    -Wl,-z,noexecstack \
    -Wl,-z,relro \
    -Wl,-z,now \
    -lpthread"
)

# ar flags
set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> rcD <TARGET> <OBJECTS>")