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

if (PANDA_TARGET_ARM32 OR PANDA_QEMU_BUILD)
    return()
endif()

add_custom_target(sampler_test_suite COMMENT "Running sampler etsnapi tests")

if(CMAKE_CROSSCOMPILING)
    ExternalProject_Get_Property(panda_host_tools binary_dir)

    set(aspt_converter_target panda_host_tools-build)
    set(aspt_converter_bin "${binary_dir}/tools/sampler/aspt_converter")
else()
    set(aspt_converter_target aspt_converter)
    set(aspt_converter_bin  $<TARGET_FILE:${aspt_converter_target}>)
endif()

set(ETS_COMPILE_ARGUMENTS --gen-stdlib=false --extension=ets --opt-level=0)

set(RUNTIME_ARGUMENTS
    --boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
    --load-runtimes=ets
    --sampling-profiler-create
    --sampling-profiler-startup-run
    --sampling-profiler-interval=200
)

set(COMPILER_ARGUMENTS_LIST
    "false"
    "true"
)

set(INTERPRETER_ARGUMENTS_LIST
    "cpp"
    "irtoc"
)

set(testing_dir "${CMAKE_CURRENT_BINARY_DIR}")

# Compile Ets tests
add_custom_target(sampler_es2panda
    COMMAND ${es2panda_bin} ${ETS_COMPILE_ARGUMENTS} --output ${testing_dir}/SamplerTest.abc ${CMAKE_CURRENT_SOURCE_DIR}/SamplerTest.ets
    WORKING_DIRECTORY "${testing_dir}"
    DEPENDS ${es2panda_target}
)

# NOTE(kurnevichstanislav): fix SIGSEGV in sampler tests with TaskManager (#14278)
set(RUNTIME_ARGUMENTS ${RUNTIME_ARGUMENTS} --workers-type=threadpool)

foreach(c_arg ${COMPILER_ARGUMENTS_LIST})
    foreach(i_arg ${INTERPRETER_ARGUMENTS_LIST})
        # Run tests
        add_custom_target(sampler_run_${c_arg}_${i_arg}
            COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS} --compiler-enable-jit=${c_arg} --interpreter-type=${i_arg} --sampling-profiler-output-file=${testing_dir}/stack_output_${c_arg}_${i_arg}.aspt ${testing_dir}/SamplerTest.abc SamplerTest.ns::main
            WORKING_DIRECTORY "${testing_dir}"
            DEPENDS ark etsstdlib sampler_es2panda
        )

        add_custom_target(sampler_run_aspt_converter_${c_arg}_${i_arg}
            COMMAND ${aspt_converter_bin} --input=${testing_dir}/stack_output_${c_arg}_${i_arg}.aspt --output=${testing_dir}/traceout_${c_arg}_${i_arg}.csv --cold-graph-enable=true
            WORKING_DIRECTORY "${testing_dir}"
            DEPENDS sampler_run_${c_arg}_${i_arg} ${aspt_converter_target}
        )

        add_custom_target(sampler_trace_check_${c_arg}_${i_arg}
            COMMAND python3 ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/tooling/sampler/managed/sampler_traces_check.py --file=${testing_dir}/traceout_${c_arg}_${i_arg}.csv
            WORKING_DIRECTORY "${testing_dir}"
            DEPENDS sampler_run_aspt_converter_${c_arg}_${i_arg}
        )

        add_dependencies(sampler_test_suite sampler_trace_check_${c_arg}_${i_arg})
    endforeach()
endforeach()

add_custom_target(sampler_run_aspt_converter_module_dump_mode
    COMMAND ${aspt_converter_bin} --input=${testing_dir}/stack_output_false_cpp.aspt --dump-modules --output=${testing_dir}/modules.txt
    WORKING_DIRECTORY "${testing_dir}"
    DEPENDS sampler_run_false_cpp ${aspt_converter_target}
)

add_custom_target(sampler_aspt_converter_module_file_check
    COMMAND python3 ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/tooling/sampler/managed/sampler_module_file_check.py --file=${testing_dir}/modules.txt --bindir=${PANDA_BINARY_ROOT}
    WORKING_DIRECTORY "${testing_dir}"
    DEPENDS sampler_run_aspt_converter_module_dump_mode
)

add_dependencies(sampler_test_suite sampler_aspt_converter_module_file_check)

# NOTE: Add tests only if sanitizers are disabled to avoid conflicts with sanitizers' signal stack
if (PANDA_USE_CUSTOM_SIGNAL_STACK)
    add_dependencies(ets_tests sampler_test_suite)
endif()
