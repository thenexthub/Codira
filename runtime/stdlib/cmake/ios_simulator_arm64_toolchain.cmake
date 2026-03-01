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
include("${CMAKE_DIR}/darwin_toolchain.cmake")
 
set(CMAKE_SYSTEM_NAME "ios")
set(CMAKE_SYSTEM_PROCESSOR "aarch64")
set(TRIPLE arm64-apple-ios11-simulator)
set(CXX_COMPATIABLE_TRIPLE arm64-apple-ios12-simulator)
set(TARGET_TRIPLE_DIRECTORY_PREFIX ios_simulator_aarch64)
 
add_compile_options(--target=${TRIPLE})
add_link_options(--target=${TRIPLE})
 
set(IOS ON)
set(IOS_PLATFORM SIMULATOR)
set(IOS_PLATFORM_LOCATION "iPhoneSimulator.platform")
set(CMAKE_IOS_DEVELOPER_ROOT "/Applications/Xcode.app/Contents/Developer/Platforms/${IOS_PLATFORM_LOCATION}/Developer")
set(CMAKE_IOS_SDK_ROOT "${CMAKE_IOS_DEVELOPER_ROOT}/SDKs/iPhoneSimulator17.5.sdk")
set(CMAKE_OSX_SYSROOT "${CMAKE_IOS_DEVELOPER_ROOT}/SDKs/iPhoneSimulator17.5.sdk")