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

# NOTE: bug with CI QEMU. Issue: #25115
if (PANDA_QEMU_BUILD)
    return()
endif()

# NOTE: Add tests only if sanitizers are disabled to avoid conflicts with sanitizers' signal stack
if (NOT PANDA_USE_CUSTOM_SIGNAL_STACK)
    return()
endif()

set(SAMPLER_GEN_ANI_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib")
file(MAKE_DIRECTORY ${SAMPLER_GEN_ANI_LIB_DIR})

panda_add_library(sampler_ani_lib SHARED
    SamplerAniTest.cpp
)
set_target_properties(sampler_ani_lib PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${SAMPLER_GEN_ANI_LIB_DIR})
panda_target_link_libraries(sampler_ani_lib arkruntime)
panda_target_include_directories(sampler_ani_lib PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani)

add_custom_target(sampler_ani_test_suite COMMENT "Running sampler ani tests")

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
add_custom_target(sampler_ani_es2panda
    COMMAND ${es2panda_bin} ${ETS_COMPILE_ARGUMENTS} --output ${testing_dir}/SamplerAniTest.abc ${CMAKE_CURRENT_SOURCE_DIR}/SamplerAniTest.ets
    WORKING_DIRECTORY "${testing_dir}"
    DEPENDS ${es2panda_target}
)
add_dependencies(sampler_ani_es2panda sampler_ani_lib)

set(ETS_ENTRY_POINT "releaseMain")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(ETS_ENTRY_POINT "debugMain")
endif()

# NOTE(kurnevichstanislav): fix SIGSEGV in sampler tests with TaskManager (#14278)
set(RUNTIME_ARGUMENTS ${RUNTIME_ARGUMENTS} --workers-type=threadpool)

foreach(c_arg ${COMPILER_ARGUMENTS_LIST})
    foreach(i_arg ${INTERPRETER_ARGUMENTS_LIST})
        # Run tests
        add_custom_target(sampler_ani_run_${c_arg}_${i_arg}
            COMMAND LD_LIBRARY_PATH=${SAMPLER_GEN_ANI_LIB_DIR} ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS} --compiler-enable-jit=${c_arg} --interpreter-type=${i_arg} --sampling-profiler-output-file=${testing_dir}/stack_output_${c_arg}_${i_arg}.aspt ${testing_dir}/SamplerAniTest.abc SamplerAniTest.ETSGLOBAL::${ETS_ENTRY_POINT}
            WORKING_DIRECTORY "${SAMPLER_GEN_ANI_LIB_DIR}"
            DEPENDS ark etsstdlib sampler_ani_es2panda
        )

        add_custom_target(sampler_ani_run_aspt_convertor_${c_arg}_${i_arg}
            COMMAND ${aspt_converter_bin} --input=${testing_dir}/stack_output_${c_arg}_${i_arg}.aspt --output=${testing_dir}/traceout_${c_arg}_${i_arg}.csv --cold-graph-enable=true
            WORKING_DIRECTORY "${SAMPLER_GEN_ANI_LIB_DIR}"
            DEPENDS sampler_ani_run_${c_arg}_${i_arg} ${aspt_converter_target}
        )

        add_custom_target(sampler_ani_trace_check_${c_arg}_${i_arg}
            COMMAND python3 ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/tooling/sampler/native/sampler_ani_traces_check.py --file=${testing_dir}/traceout_${c_arg}_${i_arg}.csv
            WORKING_DIRECTORY "${SAMPLER_GEN_ANI_LIB_DIR}"
            DEPENDS sampler_ani_run_aspt_convertor_${c_arg}_${i_arg}
        )

        add_dependencies(sampler_ani_test_suite sampler_ani_trace_check_${c_arg}_${i_arg})
    endforeach()
endforeach()

add_dependencies(ets_tests sampler_ani_test_suite)
