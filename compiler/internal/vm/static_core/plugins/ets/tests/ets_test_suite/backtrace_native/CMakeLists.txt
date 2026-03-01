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

# NOTE: Disable arm32 because dfx not support arm32 platform.
if (PANDA_TARGET_ARM32)
    return()
endif()

# NOTE: Disable llvm because mix-stack not support llvm backend.
if (PANDA_LLVM_BACKEND)
    return()
endif()

# NOTE: Disable compiler because mix-stack not support JIT/AOT.
if (PANDA_COMPILER_ENABLE)
    return()
endif()

set(backtrace_test_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(backtrace_test_out_dir "${CMAKE_CURRENT_BINARY_DIR}")
add_custom_target(test_suite_backtrace_native)

function(add_backtrace_native_test)
    set(prefix ARG)
    set(singleValues FILE CPP_SOURCE)
    set(multiValues OPTIONS)
    cmake_parse_arguments(${prefix} "" "${singleValues}" "${multiValues}" ${ARGN})

    get_filename_component(TEST_NAME "${ARG_FILE}" NAME_WE)
    set(test_in "${backtrace_test_in_dir}/${ARG_FILE}")
    set(test_out_abc "${backtrace_test_out_dir}/${TEST_NAME}.abc")
    set(target test_suite_backtrace_native_${TEST_NAME})

    # Build rules for call_backtrace_native native shared library
    panda_add_library(call_backtrace_native SHARED
        call_backtrace_native.cpp
        ${PANDA_ROOT}/runtime/tooling/backtrace/backtrace.cpp
    )

    panda_target_link_libraries(call_backtrace_native arkruntime)

    set_target_properties(call_backtrace_native PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${backtrace_test_out_dir})

    panda_target_include_directories(call_backtrace_native
        PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani
    )

    compile_ets_code(${test_in} ${test_out_abc} ${target})

    panda_ets_add_gtest(
        NO_CORES
        NAME ${target}_cpp_test
        CUSTOM_PRERUN_ENVIRONMENT
        "LD_LIBRARY_PATH=${backtrace_test_out_dir}"
        "ARK_GTEST_ABC_PATH=${test_out_abc}"
        SOURCES
            ${ARG_CPP_SOURCE}
        LIBRARIES
            arkruntime arkbase arkfile arkassembler call_backtrace_native
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
        PANDA_STD_LIB
            ${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
        DEPS_TARGETS
            etsstdlib
            ${target}
        INCLUDE_DIRS
            ${CMAKE_SOURCE_DIR}/runtime/tooling
    )
    add_dependencies(test_suite_backtrace_native  ${target}_cpp_test_gtests)
endfunction()

add_backtrace_native_test(FILE call_backtrace_native.ets CPP_SOURCE call_backtrace_native_test.cpp OPTIONS "")
