# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

add_subdirectory(checker)
add_subdirectory(common)
add_subdirectory(rules)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(lint SHARED Codelint.cpp)
if(COVERAGE_FLAG)
    target_compile_options(lint PUBLIC "-fno-exceptions")
endif()
target_link_libraries(lint
        checker
        structuralrule
        dataflowrule
        commonlib
        cangjie-lsp
        ${labList}
        )
if (CMAKE_BUILD_TYPE MATCHES Release AND MINGW)
    target_link_options(lint PRIVATE -static-libgcc -static-libstdc++ LINKER:-Bstatic,--whole-archive -lwinpthread LINKER:-Bdynamic,--no-whole-archive)
endif ()
target_compile_options(lint PRIVATE -fPIC)
SET_TARGET_PROPERTIES(lint PROPERTIES OUTPUT_NAME "codelint")
install(TARGETS lint LIBRARY DESTINATION lib RUNTIME DESTINATION lib)

add_executable(codelint main.cpp)
target_link_libraries(codelint lint)
install(TARGETS codelint RUNTIME DESTINATION bin)