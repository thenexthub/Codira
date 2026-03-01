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

# toolchain for cross_windows_x86_64

# platform
set(PLATFORM_NAME "cross_windows_x86_64")

# environment info
set(OS "windows")

set(CPU_CORE "x86_64")
set(CPU_FAMILY "x86")
set(CPU_TYPE "i686")
set(MEM_TYPE "mem")
set(BYTE_ORDER "le")
set(COMPILER "gnu")
set(FWD_PLATFORM "MCCA")

set(CMAKE_ASM_COMPILER_WORKS 1)
set(CMAKE_ASM_ABI_COMPILED 1)

# build tools
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_ASM_COMPILER "clang")
set(CMAKE_LINKER "x86_64-w64-mingw32-ld")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")

# compile flags for common
set(CMAKE_C_FLAGS
    "--target=x86_64-pc-windows-gnu \
    -Wall \
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
    -D_WIN32_WINNT=_WIN32_WINNT_VISTA \
    -Wno-inconsistent-dllimport \
    -Wno-pointer-to-int-cast \
    -fuse-ld=lld \
    --rtlib=compiler-rt"
)

set(CMAKE_CXX_FLAGS
    "--target=x86_64-pc-windows-gnu \
    -Wall \
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
    -D_WIN32_WINNT=_WIN32_WINNT_VISTA \
    -Wno-inconsistent-dllimport \
    -fno-exceptions \
    -Wno-pointer-to-int-cast \
    -fuse-ld=lld \
    -stdlib=libc++ \
    --rtlib=compiler-rt"
)

if("${BUILDING_STAGE}")
    set(CMAKE_C_FLAGS_DEBUG "-g -Wframe-larger-than=2048")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -Wframe-larger-than=2048")
endif()

# compile flags for debug version only
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_DEBUG "-g -gdwarf-4")

# compile flags for release version only
set(CMAKE_C_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")
set(CMAKE_CXX_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")

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
    "VOS_OS_VER=VOS_WINDOWS"
    "VOS_HARDWARE_PLATFORM=VOS_X86"
    "MRT_HARDWARE_PLATFORM=MRT_WINDOWS_X86"
    "VOS_CPU_TYPE=VOS_X86"
    "VOS_WORDSIZE=64"
    "VOS_BYTE_ORDER=VOS_LITTLE_ENDIAN"
    "USE___THREAD"
    "DOPRA_LIB_LOCK"
    "$<$<CONFIG:Debug>:VOS_BUILD_DEBUG=1>"
    "$<$<CONFIG:Release>:VOS_BUILD_RELEASE=1>"
    "_CRT_RAND_S"
    "MRT_WINDOWS"
    "TLS_COMMON_DYNAMIC"
    "CODIRA"
)

# link flags
set(CMAKE_SHARED_LINKER_FLAGS
    "--target=x86_64-pc-windows-gnu \
    -m64"
)

link_libraries(ssp pthread ws2_32)

# ar flags
set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> rcD <TARGET> <OBJECTS>")
