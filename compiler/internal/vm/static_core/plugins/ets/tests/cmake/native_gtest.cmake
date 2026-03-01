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

add_custom_target(ets_native_gtests COMMENT "Run ets_native tests")
add_dependencies(ets_gtests ets_native_gtests)

# Helper to add gtests.
#
# Example usage:
#   ets_native_test_helper(test_name
#     ETS_CONFIG
#       arktsconfig.json
#     CPP_SOURCES
#       tests/unit1_test.cpp
#       tests/unit2_test.cpp
#     ETS_SOURCES
#       tests/unit1_test.ets
#     LIBRARIES
#       lib_target1
#       lib_target2
#     TSAN_EXTRA_OPTIONS
#       option1
#       option2
#     ETS_GTEST_ABC_PATH
#       path/package.zip
#     VERIFY_SOURCES
#       true/false
#   )
function (ets_native_test_helper TARGET)
    cmake_parse_arguments(
        ARG # give prefix `ARG` to each argument
        ""
        "ETS_CONFIG;ETS_GTEST_ABC_PATH;VERIFY_SOURCES;TEST_GROUP"
        "CPP_SOURCES;ETS_SOURCES;LIBRARIES;TSAN_EXTRA_OPTIONS;INCLUDE_DIRS"
        ${ARGN}
    )
    if(NOT DEFINED ARG_CPP_SOURCES)
        message(FATAL_ERROR "CPP_SOURCES is not defined")
    endif()

    if(DEFINED ARG_ETS_SOURCES)
        if(NOT DEFINED ARG_VERIFY_SOURCES)
            message(FATAL_ERROR "ARG_VERIFY_SOURCES is not defined")
        endif()
        set(TARGET_GTEST_PACKAGE ${TARGET}_gtest_package)
        panda_ets_package_gtest(${TARGET_GTEST_PACKAGE}
            ETS_SOURCES ${ARG_ETS_SOURCES}
            ETS_CONFIG ${ARG_ETS_CONFIG}
            VERIFY_SOURCES ${ARG_VERIFY_SOURCES}
        )
        set(MANAGED_GTEST_ABC_PATH "${ARG_ETS_GTEST_ABC_PATH}=${PANDA_BINARY_ROOT}/abc-gtests/${TARGET_GTEST_PACKAGE}.zip")
    endif()

    set(TEST_GROUP "")
    if (DEFINED ARG_TEST_GROUP)
        set(TEST_GROUP "TEST_GROUP;${ARG_TEST_GROUP}")
    endif()

    # Add launcher <${TARGET}_gtests> target
    set(NATIVE_TESTS_DIR "${PANDA_BINARY_ROOT}/tests/native")
    panda_ets_add_gtest(
        NAME ${TARGET}
        NO_CORES
        CUSTOM_PRERUN_ENVIRONMENT
            "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
            "${MANAGED_GTEST_ABC_PATH}"

        SOURCES ${ARG_CPP_SOURCES}
        INCLUDE_DIRS ${ARG_INCLUDE_DIRS}
        LIBRARIES ${ARG_LIBRARIES} arkruntime
        SANITIZERS ${PANDA_SANITIZERS_LIST}
        TSAN_EXTRA_OPTIONS ${ARG_TSAN_EXTRA_OPTIONS}
        DEPS_TARGETS etsstdlib ${TARGET_GTEST_PACKAGE}
        TEST_RUN_DIR ${NATIVE_TESTS_DIR}
        OUTPUT_DIRECTORY ${NATIVE_TESTS_DIR}
        ${TEST_GROUP}
    )

    if (DEFINED ARG_TEST_GROUP AND NOT ${ARG_TEST_GROUP} STREQUAL "")
        add_dependencies(${ARG_TEST_GROUP} ${TARGET}_gtests)
    endif()
endfunction(ets_native_test_helper)

# Add gtest-based tests to ets_native_gtests target.
#
# Example usage:
#   panda_add_gtest_with_ani(test_name
#     CPP_SOURCES
#       tests/unit1_test.cpp
#       tests/unit2_test.cpp
#     ETS_SOURCES
#       tests/unit1_test.ets
#     LIBRARIES
#       lib_target1
#       lib_target2
#   )
function(panda_add_gtest_with_ani TARGET)
    # Parse arguments
    cmake_parse_arguments(
        ARG
        ""
        "TEST_GROUP"
        "CPP_SOURCES;ETS_SOURCES;LIBRARIES;INCLUDE_DIRS"
        ${ARGN}
    )

    set(TEST_GROUP "")
    if (DEFINED ARG_TEST_GROUP)
        set(TEST_GROUP ${ARG_TEST_GROUP})
    endif()

    ets_native_test_helper(${TARGET}
        ETS_CONFIG ${ARG_ETS_CONFIG}
        ETS_SOURCES ${ARG_ETS_SOURCES}
        CPP_SOURCES ${ARG_CPP_SOURCES}
        LIBRARIES ${ARG_LIBRARIES} ani_gtest
        TSAN_EXTRA_OPTIONS ${ARG_TSAN_EXTRA_OPTIONS}
        ETS_GTEST_ABC_PATH "ANI_GTEST_ABC_PATH"
        VERIFY_SOURCES true
        TEST_GROUP ${TEST_GROUP}
        INCLUDE_DIRS
            ${ARG_INCLUDE_DIRS}
            ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani
    )
endfunction(panda_add_gtest_with_ani)
