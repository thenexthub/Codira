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

gen_file(TARGET test_number_subtypes_gen_cpp
    TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/number_subtypes.cpp.erb
    OUTPUT ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes.cpp
    REQUIRES ${CMAKE_CURRENT_SOURCE_DIR}/../number_subtypes.rb
)

gen_file(TARGET test_number_subtypes_gen_ets
    TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/number_subtypes.ets.erb
    OUTPUT ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes.ets
    REQUIRES ${CMAKE_CURRENT_SOURCE_DIR}/../number_subtypes.rb
)

panda_ets_interop_js_gtest(ets_interop_js_test_number_subtypes_ts_to_sts
    CPP_SOURCES ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes.cpp
    ETS_SOURCES ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes.ets
    TS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/module.ts
)

add_dependencies(ets_interop_js_test_number_subtypes_ts_to_sts test_number_subtypes_gen_cpp)
add_dependencies(ets_interop_js_test_number_subtypes_ts_to_sts test_number_subtypes_gen_ets)
