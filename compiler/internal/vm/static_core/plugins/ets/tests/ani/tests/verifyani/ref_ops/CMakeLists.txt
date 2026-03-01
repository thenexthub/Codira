# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ani_verify_add_gtest(ani_verify_test_reference_delete
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_reference_delete_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_reference_delete_test.ets
)

ani_verify_add_generated_tests(ani_verify_test_get
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_get_test.cpp.j2
    PARAMS "null: Suff=Null"
           "undefined: Suff=Undefined"
)

ani_verify_add_generated_tests(ani_verify_test_reference_is
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_reference_is_test.cpp.j2
    J2_ETS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_reference_is_test.ets.j2
    PARAMS "null: suff=null,Suff=Null,type=null,get_suff=Null"
           "nullish_value: suff=nullish_value,Suff=NullishValue,type=null,get_suff=Null"
           "undefined: suff=undefined,Suff=Undefined,type=undefined,get_suff=Undefined"
)

ani_verify_add_generated_tests(ani_verify_test_reference
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_reference_equals.cpp.j2
    J2_ETS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_reference_equals.ets.j2
    PARAMS "equals: Suff=Equals,suff=equals"
           "strict_equals: Suff=StrictEquals,suff=strict_equals"
)
