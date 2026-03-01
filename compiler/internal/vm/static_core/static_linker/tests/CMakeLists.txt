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

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

panda_add_gtest(
    NAME static_linker_tests
    SOURCES
        linker_test.cpp
    LIBRARIES
        arklinker arkruntime arkassembler
    INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/assembler"
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
    TEST_RUN_DIR
        "${CMAKE_CURRENT_BINARY_DIR}"
    DEPS_TARGETS
        arkstdlib
)

file(COPY data DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

set(test_asm_files
    data/single/exceptions.pa
    data/single/lnp_dedup.pa
    data/single/lit_array.pa
    data/single/unresolved_global.pa
    data/single/hello_world.pa
    data/multi/bad_ffield_type/1.pa
    data/multi/bad_ffield_type/2.pa
    data/multi/fmethod_overloaded/1.pa
    data/multi/fmethod_overloaded/2.pa
    data/multi/ffield/1.pa
    data/multi/ffield/2.pa
    data/multi/ffield_overloaded/1.pa
    data/multi/ffield_overloaded/2.pa
    data/multi/fmethod/1.pa
    data/multi/fmethod/2.pa
    data/multi/bad_fmethod_name/1.pa
    data/multi/bad_fmethod_name/2.pa
    data/multi/bad_class_redefinition/1.pa
    data/multi/bad_class_redefinition/2.pa
    data/multi/dedup_field/1.pa
    data/multi/dedup_field/2.pa
    data/multi/dedup_method/1.pa
    data/multi/dedup_method/2.pa
    data/multi/bad_fmethod_overloaded/1.pa
    data/multi/bad_fmethod_overloaded/2.pa
    data/multi/bad_ffield/1.pa
    data/multi/bad_ffield/2.pa
    data/multi/fmethod_overloaded_2/4.pa
    data/multi/fmethod_overloaded_2/1.pa
    data/multi/fmethod_overloaded_2/2.pa
    data/multi/fmethod_overloaded_2/3.pa

    data/single/lit_array.gold
    data/single/exceptions.gold
    data/single/unresolved_global.gold
    data/single/hello_world.gold
    data/single/lnp_dedup.gold
    data/multi/fmethod_overloaded/out.gold
    data/multi/ffield/out.gold
    data/multi/ffield_overloaded/out.gold
    data/multi/fmethod/out.gold
    data/multi/dedup_field/out.gold
    data/multi/dedup_method/out.gold
    data/multi/fmethod_overloaded_2/out.gold
)

add_custom_target(static_linker_tests_gen_abc DEPENDS ${test_asm_files})
add_dependencies(static_linker_tests static_linker_tests_gen_abc arkruntime)
