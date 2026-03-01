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

include(${CMAKE_CURRENT_LIST_DIR}/common.cmake)

set(ENV{AFL_CC_COMPILER} "LLVM")
if (DEFINED AFL_LLVM_LAF_ALL AND NOT AFL_LLVM_LAF_ALL)
    unset(ENV{AFL_LLVM_LAF_ALL})
else()
    set(ENV{AFL_LLVM_LAF_ALL} "1")
endif()

set_c_compiler(afl-clang-fast)
set_cxx_compiler(afl-clang-fast++)

# This option is specific to AFL compilers.
# --afl_noopt means the AFL compiler will just pass everything to the normal compiler
set(AFL_NOOPT --afl_noopt)

add_definitions(-DFUZZING_EXIT_ON_FAILED_ASSERT=1)
add_definitions(-DFUZZING_EXIT_ON_FAILED_ASSERT_FOR="/libarkfile/,/libarkbase/")

set(FUZZER_LIB "/usr/lib/afl/libAFLDriver.a")
set(PANDA_WITH_TESTS false CACHE STRING "" FORCE)
set(PANDA_WITH_BENCHMARKS false)
set(PANDA_ENABLE_AFL true)
set(PANDA_BUILD_FUZZ_TARGETS true)
set(PANDA_FUZZING_SANITIZERS false)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-stack-address -Wno-unused-variable")
