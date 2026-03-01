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
    SOURCE ${ARKFILE_ROOT}/templates
    DESTINATION ${GEN_INCLUDE_DIR}
    REQUIRES "${ARKFILE_ROOT}/pandafile_isapi.rb"
    EXTRA_DEPENDENCIES ${output}
)

set(GENERATOR ${ARKFILE_ROOT}/types.rb)
set(TEMPLATE ${ARKFILE_ROOT}/templates/type.h.erb)
set(DATAFILE ${ARKFILE_ROOT}/types.yaml)
set(DEPENDENCIES ${GENERATOR} ${TEMPLATE} ${DATAFILE})
set(OUTFILE "${CMAKE_CURRENT_BINARY_DIR}/include/type.h")
add_custom_command(OUTPUT "${OUTFILE}"
                   COMMENT "Generate type.h"
                   COMMAND ${PANDA_ROOT}/isa/gen.rb -d ${DATAFILE} -t ${TEMPLATE} -o "${OUTFILE}" -q ${GENERATOR}
                   DEPENDS ${DEPENDENCIES})
add_custom_target(type_gen_${PROJECT_NAME} ALL DEPENDS "${OUTFILE}")
add_dependencies(panda_gen_files type_gen_${PROJECT_NAME})

set(SOURCES
    ${ARKFILE_ROOT}/annotation_data_accessor.cpp
    ${ARKFILE_ROOT}/bytecode_emitter.cpp
    ${ARKFILE_ROOT}/debug_data_accessor.cpp
    ${ARKFILE_ROOT}/debug_helpers.cpp
    ${ARKFILE_ROOT}/field_data_accessor.cpp
    ${ARKFILE_ROOT}/file.cpp
    ${ARKFILE_ROOT}/file_reader.cpp
    ${ARKFILE_ROOT}/file_writer.cpp
    ${ARKFILE_ROOT}/file_items.cpp
    ${ARKFILE_ROOT}/file_item_container.cpp
    ${ARKFILE_ROOT}/pgo.cpp
    ${ARKFILE_ROOT}/class_data_accessor.cpp
    ${ARKFILE_ROOT}/code_data_accessor.cpp
    ${ARKFILE_ROOT}/method_data_accessor.cpp
    ${ARKFILE_ROOT}/method_handle_data_accessor.cpp
    ${ARKFILE_ROOT}/debug_info_extractor.cpp
    ${ARKFILE_ROOT}/literal_data_accessor.cpp
    ${ARKFILE_ROOT}/file_format_version.cpp
)

panda_add_library(arkfile ${PANDA_DEFAULT_LIB_TYPE} ${SOURCES})

panda_target_include_directories(arkfile
    PUBLIC ${PANDA_BINARY_ROOT}
)

# Disable warning about offsetof usage for non-standard layout types.
# In C++17 offsetof is conditionally implemented for such types, so
# compiler should issue error if it doesn't implemented offsetof for
# them. Also we use static asserts to ensure that offsetof works correctly
# for non-standard layout types.
panda_target_compile_options(arkfile PUBLIC -Wno-invalid-offsetof)

set(LINK_LIBRARIES arkbase arkziparchive c_secshared)
if (NOT (PANDA_TARGET_MACOS OR PANDA_TARGET_OHOS))
    list(APPEND LINK_LIBRARIES atomic)
endif()

add_dependencies(arkfile isa_gen_${PROJECT_NAME} type_gen_${PROJECT_NAME})

panda_target_link_libraries(arkfile ${LINK_LIBRARIES})

# NOTE(aakmaev): Enable when #14728 internal issue will be fixed
if (NOT PANDA_TARGET_ARM64)
    panda_add_gtest(
        NAME arkfile_tests
        SOURCES
            ${ARKFILE_ROOT}/tests/bytecode_instruction_tests.cpp
            ${ARKFILE_ROOT}/tests/file_test.cpp
            ${ARKFILE_ROOT}/tests/file_item_container_test.cpp
            ${ARKFILE_ROOT}/tests/file_items_test.cpp
            ${ARKFILE_ROOT}/tests/bytecode_emitter_tests.cpp
            ${ARKFILE_ROOT}/tests/debug_info_extractor_test.cpp
            ${ARKFILE_ROOT}/tests/panda_cache_test.cpp
            ${ARKFILE_ROOT}/tests/file_format_version_test.cpp
            ${ARKFILE_ROOT}/tests/file_writer_test.cpp
            LIBRARIES
            arkbase
            arkfile
            arkassembler
            arkziparchive
            arkfileExt
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
    )
    if(TARGET arkfile_tests)
        add_dependencies(arkfile_tests isa_gen_${PROJECT_NAME})
        panda_target_include_directories(arkfile_tests
            PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include/tests)
    endif()
endif()

panda_add_sanitizers(TARGET arkfile SANITIZERS ${PANDA_SANITIZERS_LIST})

if (TARGET host_tools_depends)
    add_dependencies(host_tools_depends arkfile)
endif()
