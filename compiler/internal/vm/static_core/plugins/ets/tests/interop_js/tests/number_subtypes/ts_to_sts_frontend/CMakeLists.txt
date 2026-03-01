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

set(INTEROP_TESTS_GENERATED_DIR "${PANDA_BINARY_ROOT}/plugins/ets/tests/ets_interop_js/generated")

set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

gen_file(TARGET frontend_test_number_subtypes_gen_cpp
    TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/number_subtypes_frontend.cpp.erb
    OUTPUT ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes_frontend.cpp
    REQUIRES ${CMAKE_CURRENT_SOURCE_DIR}/../number_subtypes.rb
)

gen_file(TARGET frontend_test_number_subtypes_gen_ets
    TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/number_subtypes_frontend.ets.erb
    OUTPUT ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes_frontend.ets
    REQUIRES ${CMAKE_CURRENT_SOURCE_DIR}/../number_subtypes.rb
)

panda_ets_interop_js_gtest(ets_interop_js_frontend_test_number_subtypes_ts_to_sts_frontend
    CPP_SOURCES ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes_frontend.cpp
    ETS_SOURCES ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes_frontend.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/module_frontend.js
)

add_dependencies(ets_interop_js_frontend_test_number_subtypes_ts_to_sts_frontend frontend_test_number_subtypes_gen_cpp)
add_dependencies(ets_interop_js_frontend_test_number_subtypes_ts_to_sts_frontend frontend_test_number_subtypes_gen_ets)
