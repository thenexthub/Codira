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

set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

# NOTE(sarychevkonstantin): failing test, enable when fixed. Issue #29854
if (NOT PANDA_ETS_INTEROP_JS)
    panda_ets_interop_js_gtest(ets_interop_ts_to_ets_concurrency
        CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_interop_concurrency.cpp
        ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/eworker_interop_tests.ets
                    ${CMAKE_CURRENT_SOURCE_DIR}/taskpool_interop_tests.ets
        TS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../concurrency_helpers.ts
        ETS_CONFIG ${ETS_CONFIG}
        PACKAGE_NAME
        concurrentcy_tests
    )
endif()
