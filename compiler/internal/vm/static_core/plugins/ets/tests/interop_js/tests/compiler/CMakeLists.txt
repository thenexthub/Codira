# Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

if(NOT PANDA_MINIMAL_VIXL AND PANDA_COMPILER_ENABLE)
    panda_add_gtest(
        CONTAINS_MAIN
        NAME interop_compiler_unit_tests
        SOURCES
            ${PANDA_ROOT}/compiler/tests/unit_test.cpp
            interop_intrinsics_opt_test.cpp
            interop_peepholes_test.cpp
        LIBRARIES
        arkcompiler arkbase arkruntime
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
        DEPS_TARGETS
            arkstdlib
    )
    add_dependencies(ets_interop_tests interop_compiler_unit_tests_gtests)
endif()
