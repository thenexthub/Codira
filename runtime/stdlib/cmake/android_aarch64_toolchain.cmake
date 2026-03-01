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
 
get_filename_component(CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include("${CMAKE_DIR}/linux_toolchain.cmake")
 
set(CMAKE_SYSTEM_NAME "Android")
set(CMAKE_SYSTEM_PROCESSOR "aarch64")
if(NOT CMAKE_ANDROID_API)
    set(CMAKE_ANDROID_API 31)
    message(STATUS "Android API level is not set, use default setting: ${CMAKE_ANDROID_API}")
endif()
set(CMAKE_ANDROID_ARCH_ABI "arm64-v8a")
string(TOLOWER ${CMAKE_HOST_SYSTEM_NAME} lower_host_os)
set(CODIRA_TARGET_TOOLCHAIN "${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/${lower_host_os}-${CMAKE_HOST_SYSTEM_PROCESSOR}/bin")
set(TRIPLE aarch64-linux-android${CMAKE_ANDROID_API})	
if(CMAKE_ANDROID_API EQUAL 26)
    set(TARGET_TRIPLE_DIRECTORY_PREFIX linux_android_aarch64)
else()
    set(TARGET_TRIPLE_DIRECTORY_PREFIX linux_android${CMAKE_ANDROID_API}_aarch64)
endif()
 
# Variable ANDROID will be set by CMake. Custom variables are not necessary here.