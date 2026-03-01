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

add_custom_target(compiler_tests COMMENT "Common target to run compiler ETS tests")

add_dependencies(ets_tests compiler_tests)

function(compiler_add_gtest TARGET)
    cmake_parse_arguments(
        ARG # give prefix `ARG` to each argument
        ""
        "ETS_CONFIG"
        "CPP_SOURCES;ETS_SOURCES;LIBRARIES;TSAN_EXTRA_OPTIONS"
        ${ARGN}
    )

    set(VERIFY_SOURCES false) # it should fail and there is test for it
    ets_native_test_helper(${TARGET}
        ETS_CONFIG ${ARG_ETS_CONFIG}
        ETS_SOURCES ${ARG_ETS_SOURCES}
        CPP_SOURCES ${ARG_CPP_SOURCES}
        LIBRARIES ${ARG_LIBRARIES} compiler_gtest
        TSAN_EXTRA_OPTIONS ${ARG_TSAN_EXTRA_OPTIONS}
        ETS_GTEST_ABC_PATH "COMPILER_GTEST_ABC_PATH"
        INCLUDE_DIRS panda_target_include_directories ${GENERATED_DIR} ${PANDA_ROOT}/compiler/aot ${PANDA_BINARY_ROOT}
        VERIFY_SOURCES ${VERIFY_SOURCES}
        TEST_GROUP compiler_tests
    )
endfunction(compiler_add_gtest)
