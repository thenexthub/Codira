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

ani_verify_add_generated_tests(ani_verify_test_function_call
    ETS_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.json
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_function_call_test.cpp.j2
    J2_ETS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_function_call_test.ets.j2
    PARAMS "boolean: Type=Boolean,type=boolean,ret_type=boolean,ret=z,wrong_ret=int,wrong_ret_short=i,value=true"
           "char: Type=Char,type=char,ret_type=char,ret=c,wrong_ret=int,wrong_ret_short=i,value=c'a'"
           "byte: Type=Byte,type=byte,ret_type=byte,ret=b,wrong_ret=int,wrong_ret_short=i,value=1"
           "short: Type=Short,type=short,ret_type=short,ret=s,wrong_ret=int,wrong_ret_short=i,value=1"
           "int: Type=Int,type=int,ret_type=int,ret=i,wrong_ret=byte,wrong_ret_short=b,value=1"
           "long: Type=Long,type=long,ret_type=long,ret=l,wrong_ret=int,wrong_ret_short=i,value=1"
           "float: Type=Float,type=float,ret=f,ret_type=float,wrong_ret=int,wrong_ret_short=i,value=1"
           "double: Type=Double,type=double,ret=d,ret_type=double,wrong_ret=int,wrong_ret_short=i,value=1"
           "ref: Type=Ref,type=ref,ret=C{std.core.Object},ret_type=Object,wrong_ret=int,wrong_ret_short=i,value=new Object()"
)
