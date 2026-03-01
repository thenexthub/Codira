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

if(PANDA_COMPILER_ENABLE)
    panda_ets_add_gtest(
        CONTAINS_MAIN
        NAME ets_tests_compiler_optimizer
        SOURCES
            ${PANDA_ROOT}/compiler/tests/unit_test.cpp
            ets_loop_idioms_test.cpp
            string_flat_check_test.cpp
        LIBRARIES
            arkcompiler arkruntime
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
    )
endif()
