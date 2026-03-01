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

if(NOT PANDA_WITH_QUICKENER)
    add_custom_target(arkquick)
    return()
endif()

project(quickener)

set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${GENERATED_DIR})
set(QUICKENER_GEN ${GENERATED_DIR}/quickener_gen.cpp)
panda_isa_gen(
    TEMPLATES
            "quickener_gen.cpp.erb"
            "translation_table_gen.h.erb"
    SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/templates
    DESTINATION ${GENERATED_DIR}
)

set(QUICK_SOURCES
    quick.cpp
    ${QUICKENER_GEN}
)

panda_add_executable(arkquick ${QUICK_SOURCES})

panda_target_link_libraries(arkquick arkbase arkfile)

panda_target_include_directories(arkquick
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/templates
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/generated
)

add_dependencies(arkquick arkbase arkfile isa_gen_${PROJECT_NAME})

panda_add_sanitizers(TARGET arkquick SANITIZERS ${PANDA_SANITIZERS_LIST})
