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

set(UTIL_SOURCES
)

set(UTIL_TESTS_SOURCES
    ${VERIFICATION_SOURCES_DIR}/util/tests/environment.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/lazy_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/addr_map_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/tagged_index_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/flags.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/obj_pool_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/index_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/optional_ref_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/saturated_enum_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/shifted_vector_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/str_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/struct_field_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/callable_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/function_traits_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/parser/tests/parser_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/parser/tests/charset_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/enum_tag_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/int_tag_test.cpp
)

set(UTIL_RAPIDCHECK_TESTS_SOURCES
    ${VERIFICATION_SOURCES_DIR}/util/tests/environment.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/bit_vector_property_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/set_operations_property_test.cpp
    ${VERIFICATION_SOURCES_DIR}/util/tests/tagged_index_property_test.cpp
)
