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

cmake_minimum_required(VERSION 3.14.1 FATAL_ERROR)

project(plugin_ets_sdk_native)

set(ETS_SDK_NATIVE_SRC
    main.cpp
    api/Util.cpp
    api/ani_textencoder.cpp
    api/ani_textencoder_helper.cpp
    api/ani_textdecoder.cpp
    api/ani_stringdecoder.cpp
    api/ani_xmlpullparser.cpp
    ${PANDA_ROOT}/plugins/ets/stdlib/native/core/stdlib_ani_helpers.cpp
)

set(ETS_SDK_SO_LOC "${PANDA_BINARY_ROOT}/lib")

panda_add_library(ets_sdk_native SHARED ${ETS_SDK_NATIVE_SRC})
panda_target_sources(ets_sdk_native PRIVATE ${ETS_SDK_NATIVE_SRC})
set_target_properties(ets_sdk_native PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${ETS_SDK_SO_LOC})
panda_target_link_libraries(ets_sdk_native arkruntime)
if(PANDA_TARGET_OHOS)
    panda_target_link_libraries(ets_sdk_native uv)
else()
    set(NAPI_HEADERS_PATH ${PANDA_ROOT}/third_party/nodejs/node-v18.13.0-linux-x64/include/node)
    panda_target_include_directories(ets_sdk_native SYSTEM PRIVATE ${NAPI_HEADERS_PATH})
endif()
panda_target_include_directories(ets_sdk_native PRIVATE ${PANDA_ROOT}/plugins/ets/sdk/native/)
panda_target_include_directories(ets_sdk_native PRIVATE ${PANDA_ROOT}/plugins/ets/runtime/ani/)

add_dependencies(ets_sdk_prereqs ets_sdk_native)
