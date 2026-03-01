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

set(PANDA_TRIPLET x86_64-w64-mingw32)
set(PANDA_SYSROOT /usr/${PANDA_TRIPLET})

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_PREFIX_PATH ${PANDA_SYSROOT})
set(CMAKE_C_COMPILER_TARGET ${PANDA_TRIPLET})
set(CMAKE_CXX_COMPILER_TARGET ${PANDA_TRIPLET})

# NB! Do not use "win32" threading model, it does not provide
# std::thread and std::mutex. Use "posix" instead.
set(MINGW_THREADING_MODEL posix)
set(MINGW_CXX_BIN_NAME ${PANDA_TRIPLET}-g++-${MINGW_THREADING_MODEL})

find_program(MINGW_CXX_BIN ${MINGW_CXX_BIN_NAME})
if("${MINGW_CXX_BIN}" STREQUAL "MINGW_CXX_BIN-NOTFOUND")
    message(FATAL_ERROR "Unable to find MinGW ${MINGW_CXX_BIN_NAME}")
endif()

execute_process(COMMAND ${MINGW_CXX_BIN} -dumpversion
                OUTPUT_VARIABLE MINGW_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE)

set(MINGW_CXX_INC /usr/lib/gcc/${PANDA_TRIPLET}/${MINGW_VERSION}/include/c++)

add_compile_options(
    -isystem ${MINGW_CXX_INC}
    -I ${MINGW_CXX_INC}/${PANDA_TRIPLET} # For #include <bits/...>
    --sysroot=${PANDA_SYSROOT}
    --target=${PANDA_TRIPLET}
)

# here we need to strip debuginfo due to the debuginfo is too large otherwise we will encounter the following issue:
# lib/libes2panda-public.a(es2panda_lib.cpp.obj):es2panda_lib.cpp:(.debug_info+0x16): relocation truncated to fit: IMAGE_REL_AMD64_SECREL against `.debug_line'
# clang: error: linker command failed with exit code 1 (use -v to see invocation)
# previously we considered to switch to lld, however lld will complain  duplicated symbol issues caused by static linking, so currently we just strip debuginfo

# NB! For Windows we link everything statically (incl. standard library, pthread, etc.):
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/lib/gcc/${PANDA_TRIPLET}/${MINGW_VERSION} -Wl,--strip-debug -static-libstdc++ -static-libgcc -Wl,-Bstatic")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/lib/gcc/${PANDA_TRIPLET}/${MINGW_VERSION} -Wl,--strip-debug -Wl,--allow-multiple-definition -static-libstdc++ -static-libgcc -Bstatic -Wl,--image-base=0x10000000 -Wl,--disable-stdcall-fixup")

include(${CMAKE_CURRENT_LIST_DIR}/common.cmake)
set_c_compiler(clang-14)
set_cxx_compiler(clang++-14)
set(PANDA_TARGET_WINDOWS ON)
