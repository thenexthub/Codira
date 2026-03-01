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
    NO_CORES
    NAME sampling_profiler_test
    SOURCES
        sampling_profiler_test.cpp
    LIBRARIES
        arkruntime
        arkassembler
        aspt_converter_static
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
    DEPS_TARGETS
        arkstdlib
)

add_panda_assembly(TARGET sampling_profiler_test_ark_asm SOURCE sampling_profiler_test.pa)

add_dependencies(sampling_profiler_test_ark_asm ark_asm)
add_dependencies(sampling_profiler_test_gtests sampling_profiler_test_ark_asm)