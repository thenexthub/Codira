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

ani_add_gtest(ani_test_object_get_type
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_type_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_type_test.ets
)

ani_add_gtest(ani_test_object_ops_object_new
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_new_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_new_test.ets
)

ani_add_gtest(ani_test_object_get_field_by_name_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_boolean_test.ets
)

ani_add_gtest(ani_test_object_get_field_by_name_char
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_char_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_char_test.ets
)

ani_add_gtest(ani_test_object_get_field_by_name_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_byte_test.ets
)

ani_add_gtest(ani_test_object_get_field_by_name_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_short_test.ets
)

ani_add_gtest(ani_test_object_get_field_by_name_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_int_test.ets
)

ani_add_gtest(ani_test_object_get_field_by_name_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_long_test.ets
)

ani_add_gtest(ani_test_object_get_field_by_name_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_float_test.ets
)

ani_add_gtest(ani_test_object_get_field_by_name_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_double_test.ets
)

ani_add_gtest(ani_test_object_get_field_by_name_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_ref_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_by_name_ref_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_field_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_boolean_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_field_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_byte_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_field_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_short_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_field_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_int_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_field_char
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_char_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_char_test.ets
)

ani_add_gtest(ani_test_object_set_field_by_name_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_boolean_test.ets
)

ani_add_gtest(ani_test_object_set_field_by_name_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_byte_test.ets
)

ani_add_gtest(ani_test_object_set_field_by_name_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_short_test.ets
)

ani_add_gtest(ani_test_object_set_field_by_name_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_int_test.ets
)

ani_add_gtest(ani_test_object_set_field_by_name_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_long_test.ets
)

ani_add_gtest(ani_test_object_set_field_by_name_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_float_test.ets
)

ani_add_gtest(ani_test_object_set_field_by_name_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_double_test.ets
)

ani_add_gtest(ani_test_object_set_field_by_name_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_ref_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_ref_test.ets
)

ani_add_gtest(ani_test_object_set_field_by_name_char
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_char_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_by_name_char_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_field_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_long_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_field_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_float_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_field_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_double_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_field_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_ref_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_field_ref_test.ets
)

ani_add_gtest(ani_test_object_ops_object_set_field_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_boolean_test.ets
)

ani_add_gtest(ani_test_object_ops_object_set_field_char
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_char_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_char_test.ets
)

ani_add_gtest(ani_test_object_ops_object_set_field_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_byte_test.ets
)

ani_add_gtest(ani_test_object_ops_object_set_field_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_short_test.ets
)

ani_add_gtest(ani_test_object_ops_object_set_field_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_int_test.ets
)

ani_add_gtest(ani_test_object_ops_object_set_field_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_long_test.ets
)


ani_add_gtest(ani_test_object_ops_object_set_field_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_float_test.ets
)

ani_add_gtest(ani_test_object_ops_object_set_field_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_double_test.ets
)

ani_add_gtest(ani_test_object_ops_object_set_field_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_ref_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_field_ref_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_property_by_name_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_boolean_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_property_by_name_char
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_char_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_char_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_property_by_name_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_byte_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_property_by_name_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_short_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_property_by_name_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_int_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_property_by_name_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_long_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_property_by_name_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_float_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_property_by_name_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_double_test.ets
)

ani_add_gtest(ani_test_object_ops_object_get_property_by_name_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_ref_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_get_property_by_name_ref_test.ets
)

ani_add_gtest(ani_test_object_ops_call_object_method_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_boolean_test.ets
)

ani_add_gtest(ani_test_object_ops_call_object_method_char
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_char_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_char_test.ets
)

ani_add_gtest(ani_test_object_ops_call_object_method_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_byte_test.ets
)

ani_add_gtest(ani_test_object_ops_call_object_method_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_short_test.ets
)

ani_add_gtest(ani_test_object_ops_call_object_method_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_int_test.ets
)

ani_add_gtest(ani_test_object_ops_call_object_method_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_long_test.ets
)

ani_add_gtest(ani_test_object_ops_call_object_method_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_float_test.ets
)

ani_add_gtest(ani_test_object_ops_call_object_method_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_double_test.ets
)

ani_add_gtest(ani_test_object_ops_object_call_method_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_ref_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_ref_test.ets
)

ani_add_gtest(ani_test_object_ops_call_object_method_void
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_void_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_object_method_void_test.ets
)

ani_add_gtest(ani_test_object_instance_of
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_instance_of_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_instance_of_test.ets
)

ani_add_gtest(ani_test_object_ops_object_call_method_by_name_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_boolean_test.ets
)

ani_add_gtest(ani_test_object_ops_object_call_method_by_name_char
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_char_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_char_test.ets
)

ani_add_gtest(ani_test_object_ops_object_call_method_by_name_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_byte_test.ets
)

ani_add_gtest(ani_test_object_ops_object_call_method_by_name_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_short_test.ets
)

ani_add_gtest(ani_test_object_ops_object_call_method_by_name_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_int_test.ets
)

ani_add_gtest(ani_test_object_ops_object_call_method_by_name_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_long_test.ets
)

ani_add_gtest(ani_test_object_ops_object_call_method_by_name_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_float_test.ets
)

ani_add_gtest(ani_test_object_ops_object_call_method_by_name_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_double_test.ets
)

ani_add_gtest(ani_test_object_ops_object_call_method_by_name_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_ref_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_ref_test.ets
)

ani_add_gtest(ani_test_object_ops_object_call_method_by_name_void
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_void_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_call_method_by_name_void_test.ets
)

ani_add_gtest(ani_test_object_set_property_by_name_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_boolean_test.ets
)

ani_add_gtest(ani_test_object_set_property_by_name_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_byte_test.ets
)

ani_add_gtest(ani_test_object_set_property_by_name_char
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_char_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_char_test.ets
)

ani_add_gtest(ani_test_object_set_property_by_name_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_short_test.ets
)

ani_add_gtest(ani_test_object_set_property_by_name_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_int_test.ets
)

ani_add_gtest(ani_test_object_set_property_by_name_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_long_test.ets
)

ani_add_gtest(ani_test_object_set_property_by_name_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_float_test.ets
)

ani_add_gtest(ani_test_object_set_property_by_name_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_double_test.ets
)

ani_add_gtest(ani_test_object_set_property_by_name_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_ref_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/object_set_property_by_name_ref_test.ets
)
