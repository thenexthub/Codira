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

cmake_minimum_required(VERSION 3.14.1)

project(codethread C CXX)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DCODETHREAD_DEBUG=1)
    add_definitions(-DMRT_DEBUG=1)
endif ()

if(${TARGET} STREQUAL "linux_x86_64")
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
    include(../../build/cmake/toolchain/linux_x86_64.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="")
elseif(${TARGET} STREQUAL "linux_x86_64_gcov")
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
    include(../../build/cmake/toolchain/linux_x86_64_gcov.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="")
elseif(${TARGET} STREQUAL "linux_x86_64_fuzz")
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
    include(../../build/cmake/toolchain/linux_x86_64_fuzz.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="")
elseif(${TARGET} STREQUAL "linux_x86_64_cangjie")
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
    include(../../build/cmake/toolchain/linux_x86_64_cangjie.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="linux_x86_64")
elseif(${TARGET} STREQUAL "linux_aarch64")
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR "aarch64")
    include(../../build/cmake/toolchain/linux_aarch64.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="")
elseif(${TARGET} STREQUAL "linux_aarch64_cangjie")
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR "aarch64")
    include(../../build/cmake/toolchain/linux_aarch64_cangjie.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="linux_aarch64")
elseif(${TARGET} STREQUAL "ohos_aarch64_cangjie")
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR "aarch64")
    include(../../build/cmake/toolchain/ohos_aarch64_cangjie.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="linux_ohos_aarch64")
elseif(${TARGET} STREQUAL "ohos_arm_cangjie")
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR "arm")
    include(../../build/cmake/toolchain/ohos_arm_cangjie.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="linux_ohos_arm")
elseif(${TARGET} STREQUAL "ohos_x86_64_cangjie")
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
    include(../../build/cmake/toolchain/ohos_x86_64_cangjie.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="linux_ohos_x86_64")
elseif(${TARGET} STREQUAL "android_aarch64_cangjie")
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR "aarch64")
    include(../../build/cmake/toolchain/config.cmake)
    include(../../build/cmake/toolchain/${TARGET}.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="linux_android_aarch64")
elseif(${TARGET} STREQUAL "android_x86_64_cangjie")
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
    include(../../build/cmake/toolchain/config.cmake)
    include(../../build/cmake/toolchain/${TARGET}.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="linux_android_x86_64")
elseif(${TARGET} STREQUAL "windows_x86_64")
    set(CMAKE_SYSTEM_NAME "Windows")
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
    include(../../build/cmake/toolchain/windows_x86_64.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="windows_x86_64")
elseif(${TARGET} STREQUAL "cross_windows_x86_64")
    set(CMAKE_SYSTEM_NAME "Windows")
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
    include(../../build/cmake/toolchain/cross_windows_x86_64.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="windows_x86_64")
elseif(${TARGET} STREQUAL "macos_x86_64")
    set(CMAKE_SYSTEM_NAME "Darwin")
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
    include(../../build/cmake/toolchain/macos_x86_64.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="macos_x86_64")
elseif(${TARGET} STREQUAL "macos_aarch64")
    set(CMAKE_SYSTEM_NAME "Darwin")
    set(CMAKE_SYSTEM_PROCESSOR "aarch64")
    include(../../build/cmake/toolchain/macos_aarch64.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="macos_aarch64")
elseif(${TARGET} STREQUAL "ios_cangjie")
    set(CMAKE_SYSTEM_NAME "Darwin")
    set(CMAKE_SYSTEM_PROCESSOR "aarch64")
    include(../../build/cmake/toolchain/ios_cangjie.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="ios_cangjie")
elseif(${TARGET} STREQUAL "ios_simulator_aarch64_cangjie")
    set(CMAKE_SYSTEM_NAME "Darwin")
    set(CMAKE_SYSTEM_PROCESSOR "aarch64")
    include(../../build/cmake/toolchain/ios_simulator_aarch64_cangjie.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="ios_simulator_aarch64_cangjie")
elseif(${TARGET} STREQUAL "ios_simulator_x86_64_cangjie")
    set(CMAKE_SYSTEM_NAME "Darwin")
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
    include(../../build/cmake/toolchain/ios_simulator_x86_64_cangjie.cmake)
    ADD_DEFINITIONS(-DTRACE_FOLDER_PREFIX="ios_simulator_x86_64_cangjie")
endif()
include_directories(${PROJECT_SOURCE_DIR}/../../src)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    include_directories(${PROJECT_SOURCE_DIR}/../../src/os/Windows)
endif()
include_directories(${PROJECT_SOURCE_DIR}/../../third_party/third_party_bounds_checking_function/include)

set(SANITIZER_SUBPATH "")
if ("${BUILDING_STAGE}" STREQUAL "codeasan")
    add_compile_definitions(CODIRA_ASAN_SUPPORT)
    add_compile_definitions(CODIRA_SANITIZER_SUPPORT)
    add_compile_definitions(GENERAL_ASAN_SUPPORT_INTERFACE)
    set(SANITIZER_SUBPATH "/asan")
elseif ("${BUILDING_STAGE}" STREQUAL "codetsan")
    add_compile_definitions(CODIRA_TSAN_SUPPORT)
    add_compile_definitions(CODIRA_SANITIZER_SUPPORT)
    set(SANITIZER_SUBPATH "/tsan")
elseif ("${BUILDING_STAGE}" STREQUAL "codehwasan")
    add_compile_definitions(CODIRA_HWASAN_SUPPORT)
    add_compile_definitions(CODIRA_SANITIZER_SUPPORT)
    add_compile_definitions(GENERAL_ASAN_SUPPORT_INTERFACE)
    set(SANITIZER_SUBPATH "/hwasan")
endif ()

if (RUNTIME_BACKWARD_PTRAUTH_CFI MATCHES 1)
    add_compile_definitions(ENABLE_BACKWARD_PTRAUTH_CFI)
endif ()

if (RUNTIME_FORWARD_PTRAUTH_CFI MATCHES 1)
    add_compile_definitions(ENABLE_FORWARD_PTRAUTH_CFI)
endif ()

ADD_SUBDIRECTORY(src)
