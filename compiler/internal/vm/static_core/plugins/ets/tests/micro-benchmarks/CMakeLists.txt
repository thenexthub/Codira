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

if (CMAKE_CROSSCOMPILING)
    return()
endif()

if(PANDA_WITH_BENCHMARKS)
    add_custom_target(ets_microbenchmarks COMMENT "Running ets micro-benchmarks")
    add_dependencies(benchmarks ets_microbenchmarks)

    set(INTERPRETER_ARGUMENTS_LIST "cpp")
    set(MODES_LIST "int" "jit" "aot")


    if(PANDA_TARGET_ARM64 OR PANDA_TARGET_AMD64)
        list(APPEND INTERPRETER_ARGUMENTS_LIST "irtoc")
        if (PANDA_LLVM_INTERPRETER)
            list(APPEND INTERPRETER_ARGUMENTS_LIST "llvm")
        endif()
    endif()

    set(script_dir "${PANDA_ROOT}/plugins/ets/tests/scripts/micro-benchmarks")

    string(TOLOWER "${CMAKE_BUILD_TYPE}" cm_build_type)
    if ("${cm_build_type}" STREQUAL "")
        set(cm_build_type "debug")
    endif()
    if ("${cm_build_type}" STREQUAL "debug")
        set(benchmark_timeout "5s")
    endif()
    if ("${cm_build_type}" STREQUAL "fastverify")
        set(benchmark_timeout "5s")
    endif()

    if (NOT "${benchmark_timeout}" STREQUAL "")
        set(timeout_signal 10)
        set(timeout_prefix "timeout --preserve-status --signal=${timeout_signal} --kill-after=10s ${benchmark_timeout}")
        set(timeout_suffix "|| [ `expr \$? % 128` -eq ${timeout_signal} ]")
    else()
        set(timeout_prefix "")
        set(timeout_suffix "")
    endif()

    foreach(mode_arg ${MODES_LIST})
        foreach(int_arg ${INTERPRETER_ARGUMENTS_LIST})
            set(launch_file "${CMAKE_CURRENT_BINARY_DIR}/launch_${mode_arg}_${int_arg}.sh")
            set(error_file "${CMAKE_CURRENT_BINARY_DIR}/run_${mode_arg}_${int_arg}.err")
            set(launcher
                "${timeout_prefix}"
                "python3 run_micro_benchmarks.py --bindir ${CMAKE_BINARY_DIR}/bin --interpreter-type=${int_arg} --mode=${mode_arg} --log-level=debug"
                "${timeout_suffix}"
                "2> ${error_file}"
            )
            string(REPLACE ";" " " launcher "${launcher}")
            file(GENERATE OUTPUT ${launch_file} CONTENT "${launcher}")

            add_custom_target(ets_microbenchmarks_${mode_arg}_${int_arg}
                COMMENT "Running ${ARG_TARGET}"
                COMMAND . ${launch_file} || (cat ${error_file} && false)
                WORKING_DIRECTORY ${script_dir}
                DEPENDS ark etsstdlib ark_asm ark_aot
            )
            add_dependencies(ets_microbenchmarks ets_microbenchmarks_${mode_arg}_${int_arg})
        endforeach()
    endforeach()
endif()
