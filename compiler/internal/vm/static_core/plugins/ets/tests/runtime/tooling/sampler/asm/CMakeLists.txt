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

if (NOT PANDA_TARGET_ARM64 AND NOT PANDA_TARGET_AMD64)
    return()
endif()

if (PANDA_QEMU_BUILD)
    return()
endif()

enable_language(ASM)

# Build ETSNAPI library
set(SAMPLER_GEN_ASM_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}")
file(MAKE_DIRECTORY ${SAMPLER_GEN_ASM_LIB_DIR})

set(SAMPLER_ASM_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/SamplerAsmTest.cpp
)
if (PANDA_TARGET_ARM64)
    list (APPEND SAMPLER_ASM_TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/arm64/SamplerSigStackArm64.S)
elseif (PANDA_TARGET_AMD64)
    list (APPEND SAMPLER_ASM_TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/amd64/SamplerSigStackAmd64.S)
endif()

panda_add_library(sampler_asm_test SHARED ${SAMPLER_ASM_TEST_SOURCES})
set_target_properties(sampler_asm_test PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${SAMPLER_GEN_ASM_LIB_DIR})
panda_target_link_libraries(sampler_asm_test arkruntime)
panda_target_include_directories(sampler_asm_test PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani/)
add_custom_target(sampler_asm_test_suite COMMENT "Running signal alternate stack sampler test")

set(ETS_COMPILE_ARGUMENTS --gen-stdlib=false --extension=ets --opt-level=0)

set(RUNTIME_ARGUMENTS
    --boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
    --load-runtimes=ets
    --sampling-profiler-create
    --sampling-profiler-startup-run
    --sampling-profiler-interval=100
)

set(testing_dir "${CMAKE_CURRENT_BINARY_DIR}")

# Compile Ets tests
add_custom_target(sampler_asm_es2panda
    COMMAND ${es2panda_bin} ${ETS_COMPILE_ARGUMENTS} --output ${testing_dir}/SamplerAsmTest.abc ${CMAKE_CURRENT_SOURCE_DIR}/SamplerAsmTest.ets
    WORKING_DIRECTORY "${testing_dir}"
    DEPENDS ${es2panda_target}
)
add_dependencies(sampler_asm_es2panda sampler_asm_test)

# Run tests
add_custom_target(sampler_asm_run
    COMMAND LD_LIBRARY_PATH=${SAMPLER_GEN_ASM_LIB_DIR} ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS} --sampling-profiler-output-file=${testing_dir}/stack_output.aspt ${testing_dir}/SamplerAsmTest.abc SamplerAsmTest.ETSGLOBAL::main
    WORKING_DIRECTORY "${SAMPLER_GEN_ASM_LIB_DIR}"
    DEPENDS ark etsstdlib sampler_asm_es2panda
)
add_dependencies(sampler_asm_test_suite sampler_asm_run)

# NOTE: Add tests only if sanitizers are disabled to avoid conflicts with sanitizers' signal stack
if (PANDA_USE_CUSTOM_SIGNAL_STACK)
    add_dependencies(ets_tests sampler_asm_test_suite)
endif()
