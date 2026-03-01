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

cmake_minimum_required(VERSION 3.10)

project(arkfile)

set(GEN_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
file(MAKE_DIRECTORY ${GEN_INCLUDE_DIR})
file(MAKE_DIRECTORY "${GEN_INCLUDE_DIR}/tests")

set(TEMPLATES
    bytecode_instruction_enum_gen.h.erb
    bytecode_instruction-inl_gen.h.erb
    bytecode_emitter_def_gen.h.erb
    bytecode_emitter_gen.h.erb
    file_format_version.h.erb
    tests/bytecode_emitter_tests_gen.h.erb
)

panda_isa_gen(TEMPLATES ${TEMPLATES}
    SOURCE ${CMAKE_CURRENT_LIST_DIR}/templates
    DESTINATION ${GEN_INCLUDE_DIR}
    REQUIRES "${CMAKE_CURRENT_LIST_DIR}/pandafile_isapi.rb"
    EXTRA_DEPENDENCIES ${output}
)

set(GENERATOR ${CMAKE_CURRENT_LIST_DIR}/types.rb)
set(TEMPLATE ${CMAKE_CURRENT_LIST_DIR}/templates/type.h.erb)
set(DATAFILE ${CMAKE_CURRENT_LIST_DIR}/types.yaml)
set(DEPENDENCIES ${GENERATOR} ${TEMPLATE} ${DATAFILE})
set(OUTFILE "${CMAKE_CURRENT_BINARY_DIR}/include/type.h")
add_custom_command(OUTPUT "${OUTFILE}"
                   COMMENT "Generate type.h"
                   COMMAND ${PANDA_ROOT}/isa/gen.rb -d ${DATAFILE} -t ${TEMPLATE} -o "${OUTFILE}" -r ${GENERATOR}
                   DEPENDS ${DEPENDENCIES})
               add_custom_target(type_gen_${PROJECT_NAME} ALL DEPENDS "${OUTFILE}")

set(SOURCES
    annotation_data_accessor.cpp
    bytecode_emitter.cpp
    debug_data_accessor.cpp
    field_data_accessor.cpp
    file.cpp
    file_reader.cpp
    file_writer.cpp
    file_items.cpp
    file_item_container.cpp
    pgo.cpp
    class_data_accessor.cpp
    code_data_accessor.cpp
    method_data_accessor.cpp
    method_handle_data_accessor.cpp
    debug_info_extractor.cpp
    literal_data_accessor.cpp
    file_format_version.cpp
)

add_library(arkfile ${PANDA_DEFAULT_LIB_TYPE} ${SOURCES})

panda_add_to_clang_tidy(TARGET arkfile)

target_include_directories(arkfile
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include
)

# Disable warning about offsetof usage for non-standard layout types.
# In C++17 offsetof is conditionally implemented for such types, so
# compiler should issue error if it doesn't implemented offsetof for
# them. Also we use static asserts to ensure that offsetof works correctly
# for non-standard layout types.
target_compile_options(arkfile PUBLIC -Wno-invalid-offsetof)

set(LINK_LIBRARIES arkbase arkziparchive c_secshared)
if (NOT (PANDA_TARGET_MACOS OR PANDA_TARGET_OHOS))
    list(APPEND LINK_LIBRARIES atomic)
endif()

add_dependencies(arkfile isa_gen_${PROJECT_NAME} type_gen_${PROJECT_NAME})

target_link_libraries(arkfile ${LINK_LIBRARIES})

panda_add_gtest(
    NAME arkfile_tests
    SOURCES
        tests/bytecode_instruction_tests.cpp
        tests/file_test.cpp
        tests/file_item_container_test.cpp
        tests/file_items_test.cpp
        tests/bytecode_emitter_tests.cpp
        tests/debug_info_extractor_test.cpp
        tests/file_format_version_test.cpp
    LIBRARIES
        arkbase
        arkfile
        arkassembler
        arkziparchive
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
)
if(TARGET arkfile_tests)
    add_dependencies(arkfile_tests isa_gen_${PROJECT_NAME})
    target_include_directories(arkfile_tests
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include/tests)
endif()

panda_add_sanitizers(TARGET arkfile SANITIZERS ${PANDA_SANITIZERS_LIST})

add_check_style(".")

if (TARGET host_tools_depends)
    add_dependencies(host_tools_depends arkfile)
endif()

if (DEFINED PANDA_ROOT_BINARY_DIR)
    # Special case for host tool build.
    target_include_directories(arkfile PUBLIC ${PANDA_ROOT_BINARY_DIR}/libpandafile/include)
endif()
