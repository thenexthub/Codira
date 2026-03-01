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

project(encoder)

include(${PANDA_ROOT}/compiler/cmake/target.cmake)

include_directories(
    .
    ${PANDA_ROOT}
    ${PANDA_ROOT}/../common_runtime
)

set(GENERATED_DIR ${PANDA_BINARY_ROOT}/compiler/generated)

function(append_sources dir)
    list(APPEND ENCODE_SOURCES target/${dir}/encode.cpp)
    list(APPEND ENCODE_SOURCES target/${dir}/callconv.cpp)
    list(APPEND ENCODE_SOURCES target/${dir}/regfile.cpp)
    set(ENCODE_SOURCES ${ENCODE_SOURCES} PARENT_SCOPE)
endfunction()

set(ENCODE_SOURCES
    target/target.cpp
    target/asm_printer.cpp
    )

if (PANDA_COMPILER_TARGET_X86_64)
    message(STATUS "Encoder: Build amd64 target")
    append_sources("amd64")
endif()
if (PANDA_COMPILER_TARGET_X86)
    message(STATUS "Encoder: Build x86 target")
    append_sources("x86")
endif()
if (PANDA_COMPILER_TARGET_AARCH64)
    message(STATUS "Encoder: Build aarch64 target")
    append_sources("aarch64")
endif()
if (PANDA_COMPILER_TARGET_AARCH32)
    if (NOT(PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP OR PANDA_TARGET_ARM32_ABI_HARD))
        message(ERROR "Undefined ABI for aarch32 architecture - please set PANDA_TARGET_ARM32_ABI_SOFT or PANDA_TARGET_ARM32_ABI_SOFTFP or PANDA_TARGET_ARM32_ABI_HARD")
    endif()
    message(STATUS "Encoder: Build aarch32 target")
    append_sources("aarch32")
endif()

panda_add_library(arkencoder STATIC ${ENCODE_SOURCES})
set_property(TARGET arkencoder PROPERTY POSITION_INDEPENDENT_CODE ON)

panda_target_compile_options(arkencoder PRIVATE -Wno-shadow -Wno-deprecated-declarations)

# Due to encoder includes headers from asmjit
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.0)
    panda_target_compile_options(arkencoder PRIVATE -Wno-bitwise-instead-of-logical)
endif()

panda_target_include_directories(arkencoder INTERFACE .)
panda_target_include_directories(arkencoder
  PRIVATE ${PANDA_ROOT}/compiler
)

panda_target_link_libraries(arkencoder arkbase)

if (PANDA_COMPILER_TARGET_AARCH64)
    panda_target_link_libraries(arkencoder vixl)
    add_dependencies(arkencoder vixl)
    panda_target_compile_options(arkencoder PUBLIC "-DUSE_VIXL_ARM64" "-DVIXL_CODE_BUFFER_MALLOC")
endif()
if (PANDA_COMPILER_TARGET_AARCH32)
    panda_target_link_libraries(arkencoder vixl)
    add_dependencies(arkencoder vixl)
    panda_target_compile_options(arkencoder PUBLIC  "-DUSE_VIXL_ARM32" "-DVIXL_CODE_BUFFER_MALLOC")
endif()
if (PANDA_COMPILER_TARGET_X86)
    panda_target_link_libraries(arkencoder asmjit Zydis)
    add_dependencies(arkencoder asmjit Zydis)
    panda_target_include_directories(arkencoder SYSTEM
        PRIVATE ${PANDA_THIRD_PARTY_SOURCES_DIR}/zydis/include
        PRIVATE ${PANDA_THIRD_PARTY_SOURCES_DIR}/zydis/zycore/include
    )
endif()
if (PANDA_COMPILER_TARGET_X86_64)
    panda_target_link_libraries(arkencoder asmjit Zydis)
    add_dependencies(arkencoder asmjit Zydis)
    panda_target_include_directories(arkencoder SYSTEM
        PRIVATE ${PANDA_THIRD_PARTY_SOURCES_DIR}/zydis/include
        PRIVATE ${PANDA_THIRD_PARTY_SOURCES_DIR}/zydis/zycore/include
    )
endif()

panda_add_sanitizers(TARGET arkencoder SANITIZERS ${PANDA_SANITIZERS_LIST})

add_dependencies(arkencoder intrinsics_gen_compiler)
add_dependencies(arkencoder isa_gen_compiler)
add_dependencies(arkencoder instructions_gen_compiler)
add_dependencies(arkencoder cpu_features_gen)
panda_target_include_directories(arkencoder PUBLIC ${GENERATED_DIR})
