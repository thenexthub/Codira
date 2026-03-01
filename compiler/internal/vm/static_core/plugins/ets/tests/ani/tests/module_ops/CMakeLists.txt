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

ani_add_gtest(ani_test_module_find_function
    ETS_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.json
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/module_find_function_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/module_find_function_test.ets
)

ani_add_gtest(ani_test_module_find_variable
    ETS_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.json
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/module_find_variable_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/module_find_variable_test.ets
)

# NOTE(ypigunova) enable tests. Issue: #23593
if (PANDA_TARGET_ARM32)
    return()
endif()

ani_add_gtest(ani_test_module_find_combination
    ETS_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.json
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/module_find_combination_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/module_find_combination_test.ets
)
