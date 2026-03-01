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
project(arkdisassembler CXX)

panda_add_executable(ark_disasm disasm.cpp)

set(GENERATOR ${ARKFILE_ROOT}/types.rb)
set(TEMPLATE ${CMAKE_CURRENT_LIST_DIR}/templates/type_to_pandasm_type.cpp.erb)
set(DATAFILE ${ARKFILE_ROOT}/types.yaml)
set(DEPENDENCIES ${GENERATOR} ${TEMPLATE} ${DATAFILE})
set(OUTFILE "${CMAKE_CURRENT_BINARY_DIR}/type_to_pandasm_type.cpp")
add_custom_command(OUTPUT "${OUTFILE}"
                   COMMENT "Generating type_to_pandasm_type.cpp"
                   COMMAND ${PANDA_ROOT}/isa/gen.rb -d ${DATAFILE} -t ${TEMPLATE} -o "${OUTFILE}" -q ${GENERATOR}
                   DEPENDS ${DEPENDENCIES})
add_custom_target(type_to_pandasm_type_gen_${PROJECT_NAME} ALL DEPENDS "${OUTFILE}")
add_dependencies(panda_gen_files type_to_pandasm_type_gen_${PROJECT_NAME})

panda_isa_gen(
    TEMPLATES
        "instructions_test_isa.cpp.erb"
        "opcode_translator.cpp.erb"
        "bc_ins_to_pandasm_ins.cpp.erb"
    REQUIRES
        "${PANDA_ROOT}/assembler/asm_isapi.rb"
        "${ARKFILE_ROOT}/pandafile_isapi.rb"
)

panda_add_library(arkdisassembler_obj OBJECT
    disassembler.cpp
    opcode_translator.cpp
    bc_ins_to_pandasm_ins.cpp
    type_to_pandasm_type.cpp
    disasm_backed_debug_info_extractor.cpp
)

panda_target_include_directories(arkdisassembler_obj
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${CMAKE_SOURCE_DIR}/runtime
    ${CMAKE_BINARY_DIR}
)

set_target_properties(arkdisassembler_obj PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

add_dependencies(arkdisassembler_obj source_languages_h)
add_dependencies(arkdisassembler_obj merge_plugins)

panda_target_link_libraries(arkdisassembler_obj arkfile arkbase arkassembler)

panda_add_library(arkdisassembler ${PANDA_DEFAULT_LIB_TYPE} $<TARGET_OBJECTS:arkdisassembler_obj>)
panda_target_link_libraries(arkdisassembler arkdisassembler_obj)
panda_target_link_libraries(ark_disasm arkdisassembler arkbase arkfile arkassembler)

panda_add_sanitizers(TARGET arkdisassembler_obj SANITIZERS ${PANDA_SANITIZERS_LIST})
panda_add_sanitizers(TARGET ark_disasm SANITIZERS ${PANDA_SANITIZERS_LIST})

panda_add_library(arkdisassembler_static STATIC $<TARGET_OBJECTS:arkdisassembler_obj>)
panda_target_link_libraries(arkdisassembler_static arkdisassembler_obj)

if(NOT PANDA_WITH_TESTS)
    return()
endif()

# disasm_tests directory
set(DISASM_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)

set(EXTRACTOR_TEST_SRC ${DISASM_TESTS_DIR}/extractor_test.cpp)
set(FUNCTIONS_TEST_SRC ${DISASM_TESTS_DIR}/functions_test.cpp)
set(INSTRUCTIONS_TEST_SRC ${DISASM_TESTS_DIR}/instructions_test.cpp)
set(INSTRUCTIONS_TEST_ISA_SRC ${CMAKE_CURRENT_BINARY_DIR}/instructions_test_isa.cpp)
set(LABELS_TEST_SRC ${DISASM_TESTS_DIR}/labels_test.cpp)
set(LITERALS_TEST_SRC ${DISASM_TESTS_DIR}/literals_test.cpp)
set(METADATA_TEST_SRC ${DISASM_TESTS_DIR}/metadata_test.cpp)
set(RECORDS_TEST_SRC ${DISASM_TESTS_DIR}/records_test.cpp)
set(DEBUG_INFO_TEST_SRC ${DISASM_TESTS_DIR}/test_debug_info.cpp)

panda_add_gtest(
   NAME disasm_tests
   SOURCES
        ${INSTRUCTIONS_TEST_ISA_SRC}
        ${EXTRACTOR_TEST_SRC}
        ${FUNCTIONS_TEST_SRC}
        ${INSTRUCTIONS_TEST_SRC}
        ${LABELS_TEST_SRC}
        ${LITERALS_TEST_SRC}
        ${METADATA_TEST_SRC}
        ${RECORDS_TEST_SRC}
        ${DEBUG_INFO_TEST_SRC}
   LIBRARIES
       arkbase arkassembler arkdisassembler_static arkfile
   SANITIZERS
       ${PANDA_SANITIZERS_LIST}
)
