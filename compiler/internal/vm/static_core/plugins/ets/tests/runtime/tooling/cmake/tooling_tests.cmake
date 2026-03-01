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

add_custom_target(tooling_ets_tests COMMENT "Common target to run tooling ETS tests")

function(tooling_ets_native_test TARGET)
    cmake_parse_arguments(
        ARG # give prefix `ARG` to each argument
        ""
        "ETS_CONFIG"
        "CPP_SOURCES;ETS_SOURCES;LIBRARIES;TSAN_EXTRA_OPTIONS"
        ${ARGN}
    )

    set(VERIFY_SOURCES true)
    # NOTE(dslynko, #24335) Disable verifier on arm32 qemu due to flaky OOM
    if(PANDA_TARGET_ARM32 AND PANDA_QEMU_BUILD)
        set(VERIFY_SOURCES false)
    endif()

    ets_native_test_helper(${TARGET}
        ETS_CONFIG ${ARG_ETS_CONFIG}
        ETS_SOURCES ${ARG_ETS_SOURCES}
        CPP_SOURCES ${ARG_CPP_SOURCES}
        LIBRARIES ${ARG_LIBRARIES}
        TSAN_EXTRA_OPTIONS ${ARG_TSAN_EXTRA_OPTIONS}
        ETS_GTEST_ABC_PATH "MANAGED_GTEST_ABC_PATH"
        INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani
        VERIFY_SOURCES ${VERIFY_SOURCES}
        TEST_GROUP tooling_ets_tests
    )
endfunction(tooling_ets_native_test)
