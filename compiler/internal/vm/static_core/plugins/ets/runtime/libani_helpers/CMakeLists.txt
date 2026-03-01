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

set(ANI_HELPERS_SOURCES
    ani_signature_builder.cpp
    concurrency_helpers.cpp
    external_array_buffer.cpp
)

if (PANDA_ETS_INTEROP_JS)
    list(APPEND ANI_HELPERS_SOURCES
        interop_js/arkts_esvalue.cpp
        interop_js/arkts_interop_js_api.cpp
        interop_js/hybridgref_ani.cpp
        interop_js/hybridgref_napi.cpp
    )
endif()

panda_add_library(ani_helpers SHARED ${ANI_HELPERS_SOURCES})

panda_target_include_directories(ani_helpers PUBLIC ${PANDA_ROOT}/plugins/ets/runtime/ani)

if (PANDA_ETS_INTEROP_JS)
    panda_target_link_libraries(ani_helpers arkbase)
    panda_target_link_libraries(ani_helpers arkruntime)
    if(PANDA_TARGET_OHOS)
        panda_target_link_libraries(ani_helpers ace_napi.z -Wl,--no-undefined)
    else()
        panda_target_include_directories(ani_helpers SYSTEM PUBLIC ${NAPI_HEADERS_PATH})
    endif()
endif()
