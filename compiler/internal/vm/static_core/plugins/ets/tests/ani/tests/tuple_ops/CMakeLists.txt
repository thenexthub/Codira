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

ani_add_gtest(ani_test_tuplevalue_getnumberofitem
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getnumberofitem_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getnumberofitem_test.ets
)

ani_add_gtest(ani_test_tuplevalue_getitem_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_boolean_test.ets
)

ani_add_gtest(ani_test_tuplevalue_getitem_char
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_char_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_char_test.ets
)

ani_add_gtest(ani_test_tuplevalue_getitem_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_byte_test.ets
)

ani_add_gtest(ani_test_tuplevalue_getitem_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_short_test.ets
)

ani_add_gtest(ani_test_tuplevalue_getitem_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_int_test.ets
)

ani_add_gtest(ani_test_tuplevalue_getitem_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_long_test.ets
)

ani_add_gtest(ani_test_tuplevalue_getitem_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_float_test.ets
)

ani_add_gtest(ani_test_tuplevalue_getitem_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_double_test.ets
)

ani_add_gtest(ani_test_tuplevalue_getitem_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_ref_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_getitem_ref_test.ets
)

ani_add_gtest(ani_test_tuplevalue_setitem_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_boolean_test.ets
)

ani_add_gtest(ani_test_tuplevalue_setitem_char
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_char_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_char_test.ets
)

ani_add_gtest(ani_test_tuplevalue_setitem_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_byte_test.ets
)

ani_add_gtest(ani_test_tuplevalue_setitem_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_short_test.ets
)

ani_add_gtest(ani_test_tuplevalue_setitem_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_int_test.ets
)

ani_add_gtest(ani_test_tuplevalue_setitem_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_long_test.ets
)

ani_add_gtest(ani_test_tuplevalue_setitem_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_float_test.ets
)

ani_add_gtest(ani_test_tuplevalue_setitem_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_double_test.ets
)

ani_add_gtest(ani_test_tuplevalue_setitem_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_ref_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tuplevalue_setitem_ref_test.ets
)
