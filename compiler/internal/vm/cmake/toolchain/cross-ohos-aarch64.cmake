#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

set(TOOLCHAIN_CLANG_ROOT "" CACHE STRING "Path to clang-<version> directory")
set(TOOLCHAIN_SYSROOT    "" CACHE STRING "Path to sysroot")

set(PANDA_TRIPLET aarch64-linux-ohos)
set(PANDA_SYSROOT ${TOOLCHAIN_SYSROOT})

set(CMAKE_SYSTEM_NAME OHOS)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_PREFIX_PATH ${TOOLCHAIN_SYSROOT})
set(CMAKE_C_COMPILER_TARGET ${PANDA_TRIPLET})
set(CMAKE_CXX_COMPILER_TARGET ${PANDA_TRIPLET})
set(CMAKE_ASM_COMPILER_TARGET ${PANDA_TRIPLET})

link_libraries(unwind)

if(NOT PANDA_TOOLCHAIN_INITIALIZED)
    # Toolchain file can be sourced multiple times with different sets of variables.
    # Prevent failures if TOOLCHAIN_CLANG_ROOT is not set this time but still fail
    # if TOOLCHAIN_CLANG_ROOT is not specified at all.
    if(NOT TOOLCHAIN_CLANG_ROOT)
        set(CMAKE_AR           "/bin/false" CACHE FILEPATH "")
        set(CMAKE_RANLIB       "/bin/false" CACHE FILEPATH "")
        set(CMAKE_C_COMPILER   "/bin/false" CACHE FILEPATH "")
        set(CMAKE_CXX_COMPILER "/bin/false" CACHE FILEPATH "")
    else()
        set(CMAKE_AR           "${TOOLCHAIN_CLANG_ROOT}/bin/llvm-ar" CACHE FILEPATH "" FORCE)
        set(CMAKE_RANLIB       "${TOOLCHAIN_CLANG_ROOT}/bin/llvm-ranlib" CACHE FILEPATH "" FORCE)
        set(CMAKE_C_COMPILER   "${TOOLCHAIN_CLANG_ROOT}/bin/clang"   CACHE FILEPATH "" FORCE)
        set(CMAKE_CXX_COMPILER "${TOOLCHAIN_CLANG_ROOT}/bin/clang++" CACHE FILEPATH "" FORCE)
    endif()
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --rtlib=compiler-rt -fuse-ld=lld" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} --rtlib=compiler-rt -fuse-ld=lld"    CACHE STRING "" FORCE)
    # Do not add linker flags multiple times when variables persist across toolchain file invocation.
    set(PANDA_TOOLCHAIN_INITIALIZED ON CACHE STRING "" FORCE)
endif()
