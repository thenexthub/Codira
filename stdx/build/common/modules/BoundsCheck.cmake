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

cmake_minimum_required(VERSION 3.16.5)
project(boundscheck)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(SECURE_CFLAG_FOR_SHARED_LIBRARY "-fstack-protector-all")

# It is not expect libboundscheck to be instrumented by asan or hwasan
# otherwise it will generate false positive everywhere
if(CODIRA_SANITIZER_SUPPORT_ENABLED)
    get_directory_property(boundscheck_COMPILE_OPTIONS  COMPILE_OPTIONS)
    list(REMOVE_ITEM boundscheck_COMPILE_OPTIONS  "-fsanitize=address" "-fsanitize=hwaddress")
    set_directory_properties(PROPERTIES COMPILE_OPTIONS "${boundscheck_COMPILE_OPTIONS}")
endif()

if (MINGW)
    set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
    set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
    set(WARNING_FLAGS "-w")
    set(SECURE_CFLAG_FOR_SHARED_LIBRARY "-static ${SECURE_CFLAG_FOR_SHARED_LIBRARY}")
else()
    if(NOT DARWIN)
        set(SECURE_LDFLAG_FOR_SHARED_LIBRARY "-Wl,-z,relro,-z,now,-z,noexecstack")
    endif()
endif()
set(STRIP_FLAG "-s")
set(CMAKE_C_FLAGS "${SECURE_CFLAG_FOR_SHARED_LIBRARY} ${WARNING_FLAGS}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_C_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")

if(CLANG_TARGET_TRIPLE)
    # Add --target option for clang only since gcc does not support --target option.
    # In case of gcc, cross compilation requires a target-specific gcc (a cross compiler).
    add_compile_options(--target=${CLANG_TARGET_TRIPLE})
    add_link_options(--target=${CLANG_TARGET_TRIPLE})
endif()

if(CODIRA_CMAKE_SYSROOT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${CODIRA_CMAKE_SYSROOT}")
endif()

if (MINGW)
    # MSVCRT already provides most of secure functions,
    # so only files listed below need to be compiled from Huawei secure c library,
    # to avoid "multiple definition" error.
    # Refer to offiical manual of Huawei secure c library for more information.
    list(APPEND SRC_FILE src/memset_s.c src/snprintf_s.c src/vsnprintf_s.c src/secureprintoutput_a.c)
else()
    aux_source_directory(./src SRC_FILE)
endif()

# Build shared library.
add_library(boundscheck SHARED ${SRC_FILE})
add_library(boundscheck-static STATIC ${SRC_FILE})
target_include_directories(boundscheck PRIVATE ./include)
target_include_directories(boundscheck-static PRIVATE ./include)
target_link_options(boundscheck PRIVATE ${SECURE_LDFLAG_FOR_SHARED_LIBRARY} ${STRIP_FLAG})