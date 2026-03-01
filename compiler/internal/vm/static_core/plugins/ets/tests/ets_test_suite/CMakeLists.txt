# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

function(add_ets_test test_suite)
    set(prefix ARG)
    set(noValues SKIP_ARM32_COMPILER)
    set(singleValues FILE TEST_NAME ENTRYPOINT TEST_PATH)
    set(multiValues OPTIONS MODE COMPILER_EXTRA_OPTIONS)
    cmake_parse_arguments(${prefix} "${noValues}" "${singleValues}" "${multiValues}" ${ARGN})

    # Create target to run one test in different modes(e.g. ets_test_suite_coroutines_launch_n_workers)
    get_filename_component(file_name "${ARG_FILE}" NAME_WE)
    set(all_modes_tests ${test_suite}_${file_name})
    if(NOT TARGET ${all_modes_tests})
        add_custom_target(${all_modes_tests})
        add_dependencies(${test_suite} ${all_modes_tests})
    endif()

    set(test_in "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_FILE}")
    if (DEFINED ARG_TEST_PATH)
        set(test_in "${ARG_TEST_PATH}")
    endif()

    set(test_out_dir "${CMAKE_CURRENT_BINARY_DIR}/${ARG_TEST_NAME}")

    if ("INT" IN_LIST ARG_MODE)
        set(target ${test_suite}_${ARG_TEST_NAME}-ets-int)
        run_int_ets_code(${target} ${test_out_dir}
                         SOURCES "${test_in}"
                         ENTRYPOINT ${ARG_ENTRYPOINT}
                         RUNTIME_EXTRA_OPTIONS ${ARG_OPTIONS}
                         COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
        )
        add_dependencies(${all_modes_tests} ${target})
    endif()

    # Compiler doesn't support some instruction in arm32 mode(launch)
    if (ARG_SKIP_ARM32_COMPILER AND PANDA_TARGET_ARM32)
        return()
    endif()

    set(targets)
    if ("AOT" IN_LIST ARG_MODE)
        set(target ${test_suite}_${ARG_TEST_NAME}-ets-aot)
        LIST(APPEND targets ${target})
        run_aot_ets_code(aot ${test_in} ${test_out_dir} ${target}
                         RUNTIME_EXTRA_OPTIONS ${ARG_OPTIONS}
                         COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
        )
    endif()

    if ("LLVMAOT" IN_LIST ARG_MODE AND PANDA_LLVM_AOT)
        set(target ${test_suite}_${ARG_TEST_NAME}-ets-llvmaot)
        LIST(APPEND targets ${target})
        run_aot_ets_code(llvm ${test_in} ${test_out_dir} ${target}
                         RUNTIME_EXTRA_OPTIONS ${ARG_OPTIONS}
                         COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
        )
    endif()

    if ("JIT" IN_LIST ARG_MODE)
        set(target ${test_suite}_${ARG_TEST_NAME}-ets-jit)
        LIST(APPEND targets ${target})
        run_jit_ets_code(${test_in} ${test_out_dir} ${target}
                         ENTRYPOINT ${ARG_ENTRYPOINT}
                         RUNTIME_EXTRA_OPTIONS ${ARG_OPTIONS}
                         COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
        )
    endif()

    if ("JITOSR" IN_LIST ARG_MODE AND PANDA_TARGET_ARM64)
        set(target ${test_suite}_${ARG_TEST_NAME}-ets-jit-osr)
        LIST(APPEND targets ${target})
        run_jit_osr_ets_code(${test_in} ${test_out_dir} ${target}
                             ENTRYPOINT ${ARG_ENTRYPOINT}
                             RUNTIME_EXTRA_OPTIONS ${ARG_OPTIONS}
                             COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
        )
    endif()

    foreach(target ${targets})
        if (TARGET ${target})
            add_dependencies(${all_modes_tests} ${target})
        endif()
    endforeach()
endfunction()

macro(get_ets_int_targets_list RESULT ETS_INT_TARGET)
    set(TARGETS_LIST ${ETS_INT_TARGET}-cpp ${ETS_INT_TARGET}-irtoc)
    if (PANDA_LLVM_INTERPRETER)
        set(TARGETS_LIST ${TARGETS_LIST} ${ETS_INT_TARGET}-llvmirtoc)
    endif()
    set(${RESULT} ${TARGETS_LIST})
endmacro()

add_subdirectory(ani)
add_subdirectory(any)
add_subdirectory(backtrace)
add_subdirectory(backtrace_native)
add_subdirectory(symbol_extractor)
add_subdirectory(coroutines)
add_subdirectory(functions)
add_subdirectory(gc)
add_subdirectory(linker)
add_subdirectory(modules)
add_subdirectory(object_literal)
add_subdirectory(profiler)
add_subdirectory(strings)
add_subdirectory(tooling)
add_subdirectory(unions)
