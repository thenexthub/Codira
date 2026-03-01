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

# This target is to collect coverage after fuzzing on special fuzzing targets

include(${CMAKE_CURRENT_LIST_DIR}/common.cmake)

set_c_compiler(clang-12)
set_cxx_compiler(clang++-12)

add_definitions(-DFUZZING_EXIT_ON_FAILED_ASSERT=1)
add_definitions(-DFUZZING_EXIT_ON_FAILED_ASSERT_FOR="/libpandafile/,/libpandabase/")
add_definitions(-DFUZZING_FUZZILLI_BUILTIN=1)

set(PANDA_ENABLE_CLANG_TIDY false CACHE BOOL "Enable clang-tidy checks during compilation" FORCE)
set(PANDA_BUILD_FUZZ_TARGETS false)
set(PANDA_FUZZING_SANITIZERS false)
set(PANDA_WITH_TESTS false)
set(PANDA_WITH_BENCHMARKS false)

set(PANDA_SANCOV_BLACKLIST_PATH ${CMAKE_CURRENT_LIST_DIR}/../../fuzzing/fuzzilli/blacklist.txt)
set(PANDA_FUZZILLI true)

set(PANDA_CLANG_COVERAGE_FLAGS
    "-fsanitize-coverage=trace-pc-guard -fsanitize-coverage-blacklist=${PANDA_SANCOV_BLACKLIST_PATH}"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PANDA_CLANG_COVERAGE_FLAGS} -Wno-return-stack-address -Wno-unused-variable")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PANDA_CLANG_COVERAGE_FLAGS}")
