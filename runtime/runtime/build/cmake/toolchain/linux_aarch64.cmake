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

# platform
set(PLATFORM_NAME "linux_aarch64")

# environment info
set(OS "linux")

set(CPU_CORE "arm64")
set(CPU_FAMILY "arm")
set(CPU_TYPE "armarch8")
set(MEM_TYPE "mem")
set(BYTE_ORDER "le")
set(COMPILER "gnu")
set(ELF_CLASS "ELF64")

# build tools
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_LINKER "ld")
set(CMAKE_AR "ar")
set(CMAKE_STRIP "strip")
set(CMAKE_NM "nm")
set(CMAKE_OBJCOPY "objcopy")
set(CMAKE_OBJDUMP "objdump")
set(CMAKE_ASM_COMPILER "${CMAKE_C_COMPILER}")

# compile flags
set(CMAKE_C_FLAGS
    "-Wall \
    -Wextra \
    -Wfloat-equal \
    -Wvla \
    -Wswitch-default \
    -Wshadow \
    -Wunused \
    -Wundef \
    -Wdate-time \
    -Wformat=2 \
    -fgnu89-inline \
    -fno-common \
    -fno-strict-aliasing \
    -fno-builtin \
    -fno-omit-frame-pointer \
    -mlittle-endian \
    -march=armv8-a \
    -mtune=cortex-a72 \
    -pipe \
    -fPIC \
    -fstack-protector-strong \
    --param=ssp-buffer-size=4 \
    -fsigned-char"
)

set(CMAKE_CXX_FLAGS
    "-Wall \
    -Wfloat-equal \
    -Wformat=2 \
    -fno-common \
    -fno-strict-aliasing \
    -fno-builtin \
    -fno-omit-frame-pointer \
    -mlittle-endian \
    -march=armv8-a \
    -mtune=cortex-a72 \
    -pipe \
    -fPIC \
    -fstack-protector-strong \
    --param=ssp-buffer-size=4 \
    -fno-exceptions \
    -fsigned-char"
)

# Attention we need to clear CMAKE_C_FLAGS_DEBUG and CMAKE_C_FLAGS_RELEASE
# otherwise cmake will add some default compile option which we may not want

# compile flags for debug version only
if("${BUILDING_STAGE}" STREQUAL "asan")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize-recover=address")
    set(CMAKE_C_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-recover=address")
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
   "GLIBC_VERSION_CODE"
   "VOS_OS_VER=VOS_LINUX"
   "VOS_HARDWARE_PLATFORM=VOS_ARM"
   "MRT_HARDWARE_PLATFORM=MRT_ARM"
   "VOS_WORDSIZE=64"
   "VOS_CPU_TYPE=VOS_CORTEXA72"
   "VOS_BYTE_ORDER=VOS_LITTLE_ENDIAN"
   "VOS_BUILD_RELEASE"
   "MRT_LINUX"
)

# link flags
set(CMAKE_SHARED_LINKER_FLAGS
    "-rdynamic \
    -Wl,-EL \
    -Wl,-z,relro,-z,noexecstack \
    -Wl,-z,now \
    -lpthread"
)

# ar flags
set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> rcD <TARGET> <OBJECTS>")

# misc settings

set(MSG_LENGTH_BYTE 4)
set(BOUNDSCHECK_DLL_PREFIX "lib")
set(BOUNDSCHECK_C_FORMAT "lib")
