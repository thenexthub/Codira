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

ani_verify_add_generated_tests(ani_verify_test_fixed_array_new
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_fixed_array_new_test.cpp.j2
    PARAMS "boolean: type=boolean,Type=Boolean"  
           "byte: type=byte,Type=Byte"
           "char: type=char,Type=Char"
           "double: type=double,Type=Double"
           "float: type=float,Type=Float"
           "int: type=int,Type=Int"
           "long: type=long,Type=Long"
           "short: type=short,Type=Short"
)
