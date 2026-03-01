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
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
if("${CMAKE_HOST_SYSTEM_PROCESSOR}" STREQUAL "AMD64")
    set(CMAKE_HOST_SYSTEM_PROCESSOR x86_64)
endif()

set(TRIPLE aarch64-linux-ohos)
set(OHOS ON)

# We add --target option for clang only since gcc does not support --target option.
# In case of gcc, cross compilation requires a target-specific gcc (a cross compiler).
add_compile_options(--target=${TRIPLE})
add_link_options(--target=${TRIPLE})

add_compile_definitions(__ohos__)
add_compile_definitions(OPENSSL_ARM64_PLATFORM)

add_compile_options(-mbranch-protection=pac-ret)

set(CMAKE_C_FLAGS "-fno-emulated-tls ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-fno-emulated-tls ${CMAKE_CXX_FLAGS}")
set(CMAKE_RANLIB "${CODIRA_TARGET_TOOLCHAIN}/llvm-ranlib")

set(TARGET_TRIPLE_DIRECTORY_PREFIX "linux_ohos_aarch64")

set(LINKER_OPTION_PREFIX)
