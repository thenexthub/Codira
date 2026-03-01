# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(abc2program CXX)

set(SOURCES
    abc_annotation_processor.cpp
    abc_type_convert.cpp
    abc_opcode_convert.cpp
    abc_inst_convert.cpp
    abc2program_compiler.cpp
    abc2program_driver.cpp
    abc2program_key_data.cpp
    abc2program_log.cpp
    abc2program_options.cpp
    abc_class_processor.cpp
    abc_code_processor.cpp
    abc_field_processor.cpp
    abc_file_entity_processor.cpp
    abc_file_processor.cpp
    abc_file_utils.cpp
    abc_literal_array_processor.cpp
    abc_method_processor.cpp
    abc_string_table.cpp
    common/abc_code_converter.cpp
    program_dump.cpp
)

panda_isa_gen(
    TEMPLATES
        "abc_opcode_convert.cpp.erb"
        "abc_inst_convert.cpp.erb"
    REQUIRES
        "${PANDA_ROOT}/assembler/asm_isapi.rb"
        "${ARKFILE_ROOT}/pandafile_isapi.rb"
)

set(ABC_TYPE_CONVERT_CPP ${CMAKE_CURRENT_BINARY_DIR}/abc_type_convert.cpp)
panda_gen_file(
    DATA ${ARKFILE_ROOT}/types.yaml
    TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/templates/abc_type_convert.cpp.erb
    OUTPUTFILE ${ABC_TYPE_CONVERT_CPP}
    API ${ARKFILE_ROOT}/types.rb
)
add_custom_target(abc_type_convert_cpp DEPENDS ${ABC_TYPE_CONVERT_CPP})
add_dependencies(panda_gen_files abc_type_convert_cpp)

panda_add_library(abc2program ${PANDA_DEFAULT_LIB_TYPE} ${SOURCES})

add_dependencies(abc2program
    abc_type_convert_cpp
    isa_gen_abc2program
)
panda_target_link_libraries(abc2program
    arkbase
    arkfile
    arkassembler
)
add_dependencies(abc2program merge_plugins)

panda_add_executable(abc2prog abc2prog_main.cpp)
panda_target_link_libraries(abc2prog abc2program)

panda_target_include_directories(abc2program PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

panda_add_sanitizers(TARGET abc2program SANITIZERS ${PANDA_SANITIZERS_LIST})
panda_add_sanitizers(TARGET abc2prog SANITIZERS ${PANDA_SANITIZERS_LIST})
