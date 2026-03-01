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

project(arkziparchive)

panda_add_library(arkziparchive ${PANDA_DEFAULT_LIB_TYPE}
    zip_archive.cpp
    extractortool/extractor.cpp
    extractortool/file_mapper.cpp
    extractortool/zip_file_reader_io.cpp
    extractortool/zip_file_reader.cpp
    extractortool/zip_file.cpp)
panda_target_include_directories(arkziparchive
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${ZLIB_ROOT}
)

panda_target_include_directories(arkziparchive SYSTEM
    PUBLIC ${ZLIB_ROOT}/contrib/minizip
)

panda_target_link_libraries(arkziparchive arkbase)

if(NOT PANDA_USE_PREBUILT_TARGETS)
    # Without this if branch zlib will get built in 2nd stage (issue #1389)
    panda_target_link_libraries(arkziparchive zlib)
endif()

panda_add_gtest(
    NAME arkziparchive_tests
    SOURCES
        tests/libziparchive_tests.cpp
    LIBRARIES
        arkbase
        arkfile
        arkziparchive
        arkassembler
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
)

panda_add_sanitizers(TARGET arkziparchive SANITIZERS ${PANDA_SANITIZERS_LIST})
