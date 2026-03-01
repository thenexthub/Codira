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

ani_verify_add_generated_tests(ani_verify_test_string_get
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_string_get_size_test.cpp.j2
    PARAMS "utf8_size: format=UTF8"
           "utf16_size: format=UTF16"
)

ani_verify_add_generated_tests(ani_verify_test_string_get
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_string_get_substr_test.cpp.j2
    PARAMS "utf8_substr: format=utf8,Format=UTF8,type=char"
           "utf16_substr: format=utf16,Format=UTF16,type=uint16_t"
)

ani_verify_add_generated_tests(ani_verify_test_string_get
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_string_get_test.cpp.j2
    PARAMS "utf8: format=utf8,Format=UTF8,type=char"
           "utf16: format=utf16,Format=UTF16,type=uint16_t"
)

ani_verify_add_generated_tests(ani_verify_test_string_new
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_string_new_test.cpp.j2
    PARAMS "utf8: format=utf8,Format=UTF8,type=char"
           "utf16: format=utf16,Format=UTF16,type=uint16_t"
)