# Copyright (c)Huawei Device Co., Ltd.
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

if (PANDA_TARGET_ARM32)
    return()
endif()

set(gc_test_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(gc_test_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(ets_test_suite_gc)

function(add_ets_gc_test)
    set(prefix ARG)
    set(singleValues FILE MODE TIMEOUT_CMD TIMEOUT_TIME)
    set(multiValues OPTIONS)
    cmake_parse_arguments(${prefix} "" "${singleValues}" "${multiValues}" ${ARGN})

    get_filename_component(TEST_NAME "${ARG_FILE}" NAME_WE)

    set(PANDA_RUN_PREFIX ${ARG_TIMEOUT_CMD} ${ARG_TIMEOUT_TIME} ${PANDA_RUN_PREFIX} )
    set(test_in "${gc_test_in_dir}/${ARG_FILE}")
    set(test_out_dir "${gc_test_out_dir}/${TEST_NAME}")
    string(TOLOWER "${ARG_MODE}" run_mode)
    set(target ets_test_suite_gc_${TEST_NAME}-ets-${run_mode})

    # NOTE(26403) Profilesaver is turned off beause of integer overflow
    list(APPEND ARG_OPTIONS "--profilesaver-enabled=false" "--incremental-profilesaver-enabled=false")

    if ("${ARG_MODE}" STREQUAL "INT")
        run_int_ets_code(${target} ${test_out_dir} SOURCES "${test_in}" RUNTIME_EXTRA_OPTIONS ${ARG_OPTIONS})
    elseif ("${ARG_MODE}" STREQUAL "AOT")
        run_aot_ets_code(aot ${test_in} ${test_out_dir} ${target} RUNTIME_EXTRA_OPTIONS ${ARG_OPTIONS})
    elseif ("${ARG_MODE}" STREQUAL "JIT")
        run_jit_ets_code(${test_in} ${test_out_dir} ${target} RUNTIME_EXTRA_OPTIONS ${ARG_OPTIONS})
    else()
        message(FATAL_ERROR "Mode ${ARG_MODE} is not defined. Available modes: INT/AOT/JIT")
    endif()
    add_dependencies(ets_test_suite_gc ${target})
endfunction()

function(add_ets_napi_gc_test)
    set(prefix ARG)
    set(singleValues FILE CPP_SOURCE)
    set(multiValues OPTIONS)
    cmake_parse_arguments(${prefix} "" "${singleValues}" "${multiValues}" ${ARGN})

    get_filename_component(TEST_NAME "${ARG_FILE}" NAME_WE)
    set(test_in "${gc_test_in_dir}/${ARG_FILE}")
    set(test_out_abc "${gc_test_out_dir}/${TEST_NAME}.abc")
    set(target ets_test_suite_gc_${TEST_NAME})

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
    add_dependencies(ets_test_suite_gc ${target}_cpp_test_gtests)
endfunction()

# mm_tests target will launch tests that use GC the most
add_dependencies(mm_tests
                ets_test_suite_coroutines
                ets_test_suite_gc
                ets_test_suite_strings
                ets_test_suite_unions
                bouncing_pandas_ets-ets-int
                bouncing_pandas_ets-ets-int-cpp
                bouncing_pandas_ets-ets-jit
                bouncing_peas_ets-ets-int
                bouncing_peas_ets-ets-int-cpp
                bouncing_peas_ets-ets-jit)
if(NOT CMAKE_CROSSCOMPILING)
    add_dependencies(mm_tests
                    bouncing_pandas_ets-ets-aot
                    bouncing_peas_ets-ets-aot)
endif()

add_ets_gc_test(FILE pin_object.ets OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(FILE get_object_address_test.ets OPTIONS "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(FILE space_type_test.ets OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(FILE weak_ref_test.ets OPTIONS "--gc-type=g1-gc" "--run-gc-in-place" "--max-free=1" "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(FILE concurrent_start_gc.ets OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(FILE register_native_allocation_test.ets OPTIONS "--gc-type=g1-gc" "--run-gc-in-place" "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(TIMEOUT_CMD timeout TIMEOUT_TIME 1m FILE finalization_registry_test.ets OPTIONS "--gc-type=g1-gc" "--run-gc-in-place" "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(TIMEOUT_CMD timeout TIMEOUT_TIME 1m FILE test_finreg_coros.ets OPTIONS "--gc-type=g1-gc" "--run-gc-in-place" "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(FILE finalization_registry_coro_test.ets OPTIONS  "--coroutine-workers-count=10" "--gc-type=g1-gc" "--run-gc-in-place"  "--gc-trigger-type=debug-never" MODE "INT")
add_ets_gc_test(FILE weakref_test.ets OPTIONS  "--gc-type=g1-gc" "--run-gc-in-place"  "--gc-trigger-type=debug-never" MODE "INT")

add_ets_gc_test(TIMEOUT_CMD timeout TIMEOUT_TIME 10m FILE finalization_registry_deadlock_test.ets OPTIONS "--gc-type=g1-gc" "--run-gc-in-place" "--gc-trigger-type=debug-never" MODE "INT")
# NOTE(@srokashevich, #29644): enable with tsan after fix
if (NOT PANDA_ENABLE_THREAD_SANITIZER)
add_ets_gc_test(FILE scheduled_gc.ets OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug-never" "--gc-use-nth-alloc-trigger=true" MODE "INT")
endif()
add_ets_gc_test(FILE scheduled_gc_trigger_exception.ets OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug-never" MODE "INT")

add_ets_gc_test(FILE postpone_gc_test.ets OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug" MODE "INT")

add_ets_gc_test(FILE run_gc_every_safepoint.ets OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug" "--run-gc-in-place" "--run-gc-every-safepoint" MODE "INT")
add_ets_gc_test(FILE run_gc_every_safepoint.ets OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug" "--run-gc-in-place" "--run-gc-every-safepoint" MODE "JIT")
if (NOT CMAKE_CROSSCOMPILING)
    add_ets_gc_test(FILE run_gc_every_safepoint.ets OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug" "--run-gc-in-place" "--run-gc-every-safepoint" MODE "AOT")
endif()
# NOTE(@srokashevich, #29137): enable with tsan after fix
if (NOT PANDA_ENABLE_THREAD_SANITIZER)
    add_ets_gc_test(FILE heap_info_test.ets OPTIONS MODE "INT")
endif()
add_ets_gc_test(FILE free_heap_size_test.ets OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug" "--run-gc-in-place" "--gc-workers-count=1" MODE "INT")
add_ets_gc_test(FILE alloc_pinned.ets OPTIONS "--gc-type=g1-gc" "--heap-verifier=fail_on_verification:pre:into:before_g1_concurrent:post" MODE "INT")
# Running tests via API calls
if (NOT ENABLE_UNIT_TESTS_FULL_COVERAGE)
    add_ets_napi_gc_test(FILE pending_exception_gc_test.ets CPP_SOURCE pending_exception_test.cpp OPTIONS "--gc-type=g1-gc" "--gc-trigger-type=debug-never")
endif()
