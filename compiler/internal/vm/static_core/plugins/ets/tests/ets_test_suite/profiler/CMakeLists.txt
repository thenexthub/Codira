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

if (PANDA_TARGET_ARM32)
    return()
endif()

set(profiler_test_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(profiler_test_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(ets_test_suite_profiler)

function(add_ets_napi_profiler_test)
    set(prefix ARG)
    set(singleValues FILE CPP_SOURCE)
    set(multiValues OPTIONS)
    cmake_parse_arguments(${prefix} "" "${singleValues}" "${multiValues}" ${ARGN})

    get_filename_component(TEST_NAME "${ARG_FILE}" NAME_WE)
    set(test_in "${profiler_test_in_dir}/${ARG_FILE}")
    set(test_out_abc "${profiler_test_out_dir}/${TEST_NAME}.abc")
    set(target ets_test_suite_profiler_${TEST_NAME})

    compile_ets_code(${test_in} ${test_out_abc} ${target})

    panda_ets_add_gtest(
        NO_CORES
        NAME ${target}_cpp_test
        SOURCES
            ${ARG_CPP_SOURCE}
        LIBRARIES
            arkbase arkfile arkruntime arkassembler
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
        PANDA_STD_LIB
            ${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
        DEPS_TARGETS
            etsstdlib
            ${target}
    )
    add_dependencies(ets_test_suite_profiler ${target}_cpp_test_gtests)
endfunction()


# Running tests via API calls

add_ets_napi_profiler_test(FILE profile_saver_test.ets CPP_SOURCE profile_saver_test.cpp OPTIONS "")
