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

panda_add_library(asm_defines defines.cpp)

add_dependencies(asm_defines
    type_gen_arkfile
    isa_gen_arkfile
    arkfile_gen
    events_gen
    logger_gen
    intrinsics_gen_arkruntime
    entrypoints_gen
    runtime_language_context
    shorty_values_gen_arkruntime
    plugins_defines_h
    plugins_asm_defines_def
    coherency_line_size_h
    source_language_gen
    plugins_regmasks_gen
)

set(GEN_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/../include")

set(ASM_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(ASM_FILE libasm_defines.S)
set(OUTPUT_FILE asm_defines.h)
set(OUTPUT_DIR ${GEN_INCLUDE_DIR})
file(MAKE_DIRECTORY ${OUTPUT_DIR})

set_target_properties(asm_defines
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${ASM_DIR}
    SUFFIX .S
)

panda_target_include_directories(asm_defines
    PUBLIC ${PANDA_ROOT}/runtime
    PUBLIC ${PANDA_ROOT}
    PUBLIC ${CMAKE_BINARY_DIR}/runtime/include
    PUBLIC ${PANDA_BINARY_ROOT}/panda_gen_options
    PUBLIC ${PANDA_BINARY_ROOT}/verification/gen/include
    PUBLIC ${PANDA_ROOT}/../common_runtime
)

panda_target_include_directories(asm_defines SYSTEM PUBLIC
    "$<TARGET_PROPERTY:c_secshared,INCLUDE_DIRECTORIES>"
    "$<TARGET_PROPERTY:c_secshared,INTERFACE_INCLUDE_DIRECTORIES>"
)

# We have to disable LTO, because it generates llvm IR instead of assembly for '-S' option.
# Without this restriction, we can just use generator expressions and inherit compile flags from arkruntime (that looks
# best way to do that):
# panda_target_compile_options(asm_defines
#     PUBLIC "$<TARGET_PROPERTY:arkruntime,COMPILE_OPTIONS>"
# )
# But it seems that we can't remove options from the generator expressions.
# (-fno-lto doesn't work because compilation fails with inappropriate regalloc)
string(REPLACE "-mllvm -regalloc=${PANDA_LLVM_REGALLOC}" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
string(REPLACE "-flto=thin" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
panda_target_compile_options(asm_defines PUBLIC "-Wno-attributes")
panda_target_compile_options(asm_defines PUBLIC "-Wno-invalid-offsetof")
panda_target_compile_options(asm_defines PUBLIC "-S")

if (PANDA_TARGET_WINDOWS)
    panda_target_compile_options(asm_defines PUBLIC "-Wno-inconsistent-dllimport")
endif()

add_custom_command(OUTPUT ${OUTPUT_DIR}/${OUTPUT_FILE}
    COMMAND ruby ${CMAKE_CURRENT_SOURCE_DIR}/defines_generator.rb "${ASM_DIR}/${ASM_FILE}" ${OUTPUT_DIR}/${OUTPUT_FILE}
    DEPENDS asm_defines ${ASM_DIR}/${ASM_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/defines_generator.rb
)

add_custom_target(asm_defines_generator
    DEPENDS ${OUTPUT_DIR}/${OUTPUT_FILE}
)
