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

cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

project(bytecode_optimizer)

include(cmake/coverage.cmake)

set(BYTECODE_OPT_SOURCES
    canonicalization.cpp
    codegen.cpp
    bytecodeopt_peepholes.cpp
    optimize_bytecode.cpp
    reg_acc_alloc.cpp
    reg_encoder.cpp
    check_resolver.cpp
    common.cpp
    const_array_resolver.cpp
)

set(SOURCES ${BYTECODE_OPT_SOURCES})
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY "${GENERATED_DIR}")

panda_isa_gen(
    TEMPLATES
        "insn_selection.h.erb"
        "insn_selection.cpp.erb"
        "check_width.cpp.erb"
        "check_width.h.erb"
    REQUIRES
        "${CMAKE_CURRENT_SOURCE_DIR}/bytecode_optimizer_isapi.rb"
        "${PANDA_ROOT}/assembler/asm_isapi.rb"
    DESTINATION
        "${GENERATED_DIR}"
)

add_library(arkbytecodeopt ${PANDA_DEFAULT_LIB_TYPE} ${SOURCES})
add_dependencies(arkbytecodeopt isa_gen_bytecode_optimizer)

panda_gen_options(TARGET arkbytecodeopt YAML_FILE options.yaml GENERATED_HEADER bytecodeopt_options_gen.h)

target_include_directories(arkbytecodeopt
    PUBLIC ${PANDA_ROOT}
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
    PUBLIC ${GENERATED_DIR}
    PUBLIC ${GENERATED_DIR}/../compiler/generated
)

target_link_libraries(arkbytecodeopt arkcompiler arkassembler arkfile)

set(PANDA_BYTECODE_OPT_TESTS_LIBRARIES arkbytecodeopt arkfile arkbase)
if (NOT (PANDA_TARGET_MOBILE OR PANDA_TARGET_OHOS OR PANDA_ENABLE_FUZZBENCH))
    list(APPEND PANDA_BYTECODE_OPT_TESTS_LIBRARIES stdc++fs)
endif()

set(BYTECODE_OPT_TEST_SOURCES
    tests/bitops_bitwise_and_test.cpp
    tests/bc_lowering_test.cpp
    tests/codegen_test.cpp
    tests/reg_acc_alloc_test.cpp
    tests/reg_encoder_test.cpp
    tests/runtime_adapter_test.cpp
    tests/const_array_resolver_test.cpp
    tests/bytecodeopt_peepholes_test.cpp
    tests/check_resolver_test.cpp
    tests/canonicalization_test.cpp
)

panda_add_gtest(
    NAME bytecodeopt_unit_tests
    SOURCES
        ${BYTECODE_OPT_TEST_SOURCES}
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
    LIBRARIES
        ${PANDA_BYTECODE_OPT_TESTS_LIBRARIES}
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
)

panda_add_gtest(
    NO_CORES
    NAME bytecodeopt_peepholes_runtime_tests
    SOURCES
        tests/bytecodeopt_peepholes_runtime_test.cpp
    LIBRARIES
        arkassembler
        arkbytecodeopt
        arkruntime
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
)

panda_add_to_clang_tidy(TARGET arkbytecodeopt CHECKS
    # TODO(mbolshov): elaborate
    "-hicpp-use-equals-default"
    "-modernize-use-equals-default"
    "-cppcoreguidelines-pro-type-member-init"
    "-misc-unused-parameters"
    # From assembler
    "-misc-non-private-member-variables-in-classes"
    "-cert-dcl21-cpp"
    "-cppcoreguidelines-macro-usage"
    "-google-runtime-references"
    "-readability-identifier-naming" # to use ins_create_api.h
)

panda_add_sanitizers(TARGET arkbytecodeopt SANITIZERS ${PANDA_SANITIZERS_LIST})

add_check_style(".")
