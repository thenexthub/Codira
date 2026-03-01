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

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(arkfileExt)

set(SOURCES
    ${ARKFILE_ROOT}/external/file_ext.cpp)

panda_add_library(arkfileExt SHARED ${SOURCES})

panda_target_link_libraries(arkfileExt arkfile arkbase)
panda_target_include_directories(arkfileExt
                          PUBLIC ${PANDA_ROOT})

panda_add_sanitizers(TARGET arkfileExt SANITIZERS ${PANDA_SANITIZERS_LIST})

set(PANDA_SUPPORT_SOURCE
    ${ARKFILE_ROOT}/external/panda_file_support.cpp)

panda_add_library(arksupport STATIC SHARED ${PANDA_SUPPORT_SOURCE})
set_property(TARGET arksupport PROPERTY POSITION_INDEPENDENT_CODE ON)

panda_target_link_libraries(arksupport arkfile arkbase)

panda_target_include_directories(arksupport
                          PUBLIC ${PANDA_ROOT})

panda_add_sanitizers(TARGET arksupport SANITIZERS ${PANDA_SANITIZERS_LIST})
