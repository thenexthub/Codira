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

ani_add_gtest(ani_test_var_ops_variable_get_value_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_boolean_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_get_value_char
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_char_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_char_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_get_value_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_byte_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_get_value_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_short_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_get_value_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_float_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_get_value_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_long_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_get_value_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_int_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_get_value_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_double_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_get_value_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_ref_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_get_value_ref_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_set_value_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_boolean_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_set_value_char
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_char_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_char_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_set_value_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_byte_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_set_value_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_short_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_set_value_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_int_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_set_value_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_long_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_set_value_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_float_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_set_value_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_double_test.ets
)

ani_add_gtest(ani_test_var_ops_variable_set_value_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_ref_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/variable_set_value_ref_test.ets
)
