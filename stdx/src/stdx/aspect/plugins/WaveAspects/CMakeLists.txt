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

if ("$ENV{CODIRA_HOME}" STREQUAL "")
    message(FATAL_ERROR "\n##### Please confirm codec is well configured in current environment. #####")
endif()

FILE(GLOB PLUGIN_SRC ./WaveAspects.cpp)
set(LIB_NAME wave-aspects)

add_library(${LIB_NAME} SHARED ${PLUGIN_SRC})
set_target_properties(${LIB_NAME} PROPERTIES COMPILE_FLAGS "-fPIC")

if ("$ENV{CODIRA_LSP_PATH}" STREQUAL "")
    target_link_directories(${LIB_NAME} PRIVATE $ENV{CODIRA_HOME}/tools/lib)
else()
    target_link_directories(${LIB_NAME} PRIVATE $ENV{CODIRA_LSP_PATH})
endif()

if(DARWIN)
    target_link_options(${LIB_NAME} PRIVATE "-Wl,-flat_namespace")
endif()

target_link_libraries(${LIB_NAME} PRIVATE cangjie-lsp)

install_cangjie_library_ffi(${LIB_NAME})
install_cangjie_library_ffi_s(${LIB_NAME})
