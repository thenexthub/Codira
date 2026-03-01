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

project(aotdump)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVIXL_INCLUDE_SIMULATOR_AARCH64")

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(AOTDUMP_OPTIONS_H ${GENERATED_DIR}/aotdump_options.h)
panda_gen_file(
    DATA ${CMAKE_CURRENT_LIST_DIR}/aotdump.yaml
    TEMPLATE ${PANDA_ROOT}/templates/options/options.h.erb
    OUTPUTFILE ${AOTDUMP_OPTIONS_H}
    API ${PANDA_ROOT}/templates/common.rb
)
add_custom_target(aotdump_options DEPENDS ${AOTDUMP_OPTIONS_H})
add_dependencies(panda_gen_files aotdump_options)

panda_add_executable(ark_aotdump
    aotdump.cpp
)

panda_target_compile_options(ark_aotdump PUBLIC "-Wno-attributes")

add_dependencies(ark_aotdump aotdump_options)

panda_target_link_libraries(ark_aotdump arkbase arkruntime arkaotmanager)

if (PANDA_TARGET_AMD64)
    panda_target_link_libraries(ark_aotdump Zydis)
    panda_target_include_directories(ark_aotdump SYSTEM
        PRIVATE ${PANDA_THIRD_PARTY_SOURCES_DIR}/zydis/include
        PRIVATE ${PANDA_THIRD_PARTY_SOURCES_DIR}/zydis/zycore/include
    )
endif()

panda_target_include_directories(ark_aotdump
    PUBLIC ${GENERATED_DIR}
    PUBLIC ${PANDA_ROOT}/compiler/aot
    PUBLIC ${PANDA_ROOT}/runtime
)

panda_add_sanitizers(TARGET ark_aotdump SANITIZERS ${PANDA_SANITIZERS_LIST})
