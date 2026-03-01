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

project(ark_aot)

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(arkcompiler_OPTIONS_H ${GENERATED_DIR}/paoc_options.h)
panda_gen_file(
        DATA ${CMAKE_CURRENT_LIST_DIR}/paoc.yaml
        TEMPLATE ${PANDA_ROOT}/templates/options/options.h.erb
        OUTPUTFILE ${arkcompiler_OPTIONS_H}
        API ${PANDA_ROOT}/templates/common.rb
)
add_custom_target(arkcompiler_options DEPENDS ${arkcompiler_OPTIONS_H})
add_dependencies(panda_gen_files arkcompiler_options)

set(PAOC_SOURCES paoc.cpp)

if (PANDA_LLVM_AOT)
    list(APPEND PAOC_SOURCES paoc_llvm.cpp)
endif()

panda_add_executable(ark_aot ${PAOC_SOURCES})

if(PANDA_WITH_ETS)
    add_dependencies(ark_aot arkcompiler_options es2panda-public)
    panda_target_link_libraries(ark_aot arkbase arkruntime aot_builder es2panda-public arkassembler)
else()
    add_dependencies(ark_aot arkcompiler_options)
    panda_target_link_libraries(ark_aot arkbase arkruntime aot_builder arkassembler)
endif()

if (PANDA_LLVM_AOT)
    panda_target_include_directories(ark_aot SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
    panda_target_link_libraries(ark_aot llvmbackend)
    panda_target_include_directories(ark_aot PUBLIC ${CMAKE_SOURCE_DIR}/libllvmbackend)
endif()

panda_target_include_directories(ark_aot
    PUBLIC ${GENERATED_DIR}
    PUBLIC ${PANDA_ROOT}/compiler/aot
    PUBLIC ${PANDA_BINARY_ROOT}
)

panda_add_sanitizers(TARGET ark_aot SANITIZERS ${PANDA_SANITIZERS_LIST})

# TODO: remove after all components will use ark_aot instead of paoc
add_custom_target(paoc ALL
                  COMMAND cd $<TARGET_FILE_DIR:ark_aot> && ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_NAME:ark_aot> paoc)

add_dependencies(paoc ark_aot)
