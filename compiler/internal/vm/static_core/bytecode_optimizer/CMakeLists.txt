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

project(bytecode_optimizer)

include(cmake/coverage.cmake)

set(BYTECODE_OPT_SOURCES
    canonicalization.cpp
    codegen.cpp
    bytecodeopt_peepholes.cpp
    call_devirtualization.cpp
    optimize_bytecode.cpp
    reg_acc_alloc.cpp
    reg_encoder.cpp
    check_resolver.cpp
    common.cpp
    const_array_resolver.cpp
    runtime_adapter.cpp
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

panda_add_library(arkbytecodeopt_obj OBJECT ${SOURCES})
add_dependencies(arkbytecodeopt_obj isa_gen_bytecode_optimizer)
panda_gen_options(TARGET arkbytecodeopt_obj YAML_FILE options.yaml GENERATED_HEADER bytecodeopt_options_gen.h)

set_target_properties(arkbytecodeopt_obj PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

panda_target_include_directories(arkbytecodeopt_obj
    PUBLIC ${PANDA_ROOT}
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
    PUBLIC ${GENERATED_DIR}
)

panda_target_link_libraries(arkbytecodeopt_obj arkcompiler arkassembler arkfile arkbase)

panda_add_library(arkbytecodeopt_static STATIC $<TARGET_OBJECTS:arkbytecodeopt_obj>)
panda_target_link_libraries(arkbytecodeopt_static arkbytecodeopt_obj)

panda_add_library(arkbytecodeopt ${PANDA_DEFAULT_LIB_TYPE} $<TARGET_OBJECTS:arkbytecodeopt_obj>)
panda_target_link_libraries(arkbytecodeopt arkbytecodeopt_obj)

set(PANDA_BYTECODE_OPT_TESTS_LIBRARIES arkbytecodeopt_static)
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
    tests/irbuilder_test.cpp
    tests/bytecodeopt_peepholes_initobj.cpp
)

panda_add_gtest(
    NAME bytecodeopt_unit_tests
    SOURCES
        ${BYTECODE_OPT_TEST_SOURCES}
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GENERATED_DIR}/..
        ${GENERATED_DIR}/../panda_gen_options
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

panda_add_sanitizers(TARGET arkbytecodeopt SANITIZERS ${PANDA_SANITIZERS_LIST})
