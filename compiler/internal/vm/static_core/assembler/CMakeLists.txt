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
project(assembler CXX)

panda_add_executable(ark_asm pandasm.cpp)

set(PANDASM_BIN_TESTS ${CMAKE_CURRENT_BINARY_DIR}/tests)
file(MAKE_DIRECTORY "${PANDASM_BIN_TESTS}")

panda_isa_gen(
    TEMPLATES
        "isa.h.erb"
        "ins_emit.h.erb"
        "ins_to_string.cpp.erb"
        "ins_create_api.h.erb"
        "opcode_parsing.h.erb"
        "operand_types_print.h.erb"
    REQUIRES
        "${CMAKE_CURRENT_SOURCE_DIR}/asm_isapi.rb"
        "${ARKFILE_ROOT}/pandafile_isapi.rb"
)

set(SOURCES
    lexer.cpp
    annotation.cpp
    assembly-emitter.cpp
    assembly-parser.cpp
    assembly-program.cpp
    assembly-type.cpp
    assembly-ins.cpp
    context.cpp
    meta.cpp
    ins_to_string.cpp
    extensions/extensions.cpp
    utils/number-utils.cpp
)

set(META_GEN_H ${CMAKE_CURRENT_BINARY_DIR}/meta_gen.h)
panda_gen_file(
    DATA ${CMAKE_CURRENT_SOURCE_DIR}/metadata.yaml
    TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/templates/meta_gen.cpp.erb
    OUTPUTFILE ${META_GEN_H}
    API ${CMAKE_CURRENT_SOURCE_DIR}/asm_metadata.rb
)
add_custom_target(meta_gen_h DEPENDS ${META_GEN_H})
add_dependencies(panda_gen_files meta_gen_h)

panda_add_library(arkassembler_obj OBJECT ${SOURCES})
add_dependencies(arkassembler_obj
    isa_gen_assembler
    arkfile
    meta_gen_h
    profiling_gen
)

set_target_properties(arkassembler_obj PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

panda_target_include_directories(arkassembler_obj PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PANDA_ROOT}
    ${CMAKE_BINARY_DIR}
)

panda_target_link_libraries(arkassembler_obj arkfile)
panda_target_link_libraries(ark_asm arkassembler arkbase)
if(PANDA_WITH_BYTECODE_OPTIMIZER)
    panda_target_link_libraries(ark_asm arkbytecodeopt)
endif()

panda_add_library(arkassembler_static STATIC $<TARGET_OBJECTS:arkassembler_obj>)
panda_target_link_libraries(arkassembler_static arkassembler_obj)

panda_add_gtest(
    NAME assembler_tests
    SOURCES
        tests/lexer_test.cpp
        tests/parser_test.cpp
        tests/emitter_test.cpp
        tests/mangling_tests.cpp
        tests/type_test.cpp
    LIBRARIES
        arkbase arkassembler_static
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
)
if(TARGET assembler_tests)
    panda_target_compile_options(assembler_tests PUBLIC "-Wno-ignored-attributes")
endif()

panda_add_sanitizers(TARGET arkassembler_obj SANITIZERS ${PANDA_SANITIZERS_LIST})
panda_add_sanitizers(TARGET ark_asm SANITIZERS ${PANDA_SANITIZERS_LIST})

# NOTE: remove after all components will use ark_asm instead of pandasm
add_custom_target(pandasm ALL
                  COMMAND cd $<TARGET_FILE_DIR:ark_asm> && ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_NAME:ark_asm> pandasm)

add_dependencies(pandasm ark_asm)

if (TARGET host_tools_depends)
    add_dependencies(host_tools_depends arkassembler_obj)
endif()

if (DEFINED PANDA_ROOT_BINARY_DIR)
    # Special case for host tool build.
    panda_target_include_directories(arkassembler_obj PUBLIC ${PANDA_ROOT_BINARY_DIR}/assembler)
endif()

panda_add_library(arkassembler ${PANDA_DEFAULT_LIB_TYPE} $<TARGET_OBJECTS:arkassembler_obj>)
panda_target_link_libraries(arkassembler arkassembler_obj)

if (PANDA_ENABLE_AFL)
    include("${PANDA_ROOT}/extras/fuzzing/Fuzzing.cmake")
    panda_substitute_libs(TARGET arkassembler_obj LIBS arkbase c_secshared arkfile)

    panda_add_library(arkassembler_fuzz ${PANDA_DEFAULT_LIB_TYPE} $<TARGET_OBJECTS:arkassembler_obj_fuzz>)
    panda_target_link_libraries(arkassembler_fuzz arkassembler_obj_fuzz)
endif()
