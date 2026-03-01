# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

add_custom_target(ani_tests COMMENT "Common target to run ANI ETS tests")
add_custom_target(ani_tests_without_bridges COMMENT "Common target to run ANI ETS tests exclude bridges tests")

add_dependencies(ani_tests ani_tests_without_bridges)

add_dependencies(ets_tests ani_tests)

# Add gtest-based tests to ani_tests_without_bridges target.
#
# Example usage:
#   ani_add_gtest(test_name
#     ETS_CONFIG path/to/arktsconfig.json
#     CPP_SOURCES
#       tests/unit1_test.cpp
#       tests/unit2_test.cpp
#     ETS_SOURCES
#       tests/unit1_test.ets
#     LIBRARIES
#       lib_target1
#       lib_target2
#   )
function(ani_add_gtest TARGET)
    cmake_parse_arguments(
        ARG # give prefix `ARG` to each argument
        ""
        "ETS_CONFIG;TARGET_PREFIX"
        "CPP_SOURCES;ETS_SOURCES;LIBRARIES;TSAN_EXTRA_OPTIONS"
        ${ARGN}
    )

    # Check TARGET prefix
    set(TARGET_PREFIX "ani_test_")
    if(DEFINED ARG_TARGET_PREFIX)
        set(TARGET_PREFIX ${ARG_TARGET_PREFIX})
    endif()
    string(FIND "${TARGET}" "${TARGET_PREFIX}" PREFIX_INDEX)
    if(NOT ${PREFIX_INDEX} EQUAL 0)
        message(FATAL_ERROR "TARGET (${TARGET}) should have '${TARGET_PREFIX}' prefix")
    endif()

    set(VERIFY_SOURCES true)
    # NOTE(dslynko, #24335) Disable verifier on arm32 qemu due to flaky OOM
    if(PANDA_TARGET_ARM32 AND PANDA_QEMU_BUILD)
        set(VERIFY_SOURCES false)
    endif()

    # NOTE(@srokashevich, #29390): enable verifier with tsan after fix
    if (PANDA_ENABLE_THREAD_SANITIZER)
        set(VERIFY_SOURCES false)
    endif()

    ets_native_test_helper(${TARGET}
        ETS_CONFIG ${ARG_ETS_CONFIG}
        ETS_SOURCES ${ARG_ETS_SOURCES}
        CPP_SOURCES ${ARG_CPP_SOURCES}
        LIBRARIES ${ARG_LIBRARIES} ani_gtest
        TSAN_EXTRA_OPTIONS ${ARG_TSAN_EXTRA_OPTIONS}
        ETS_GTEST_ABC_PATH "ANI_GTEST_ABC_PATH"
        INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani
        VERIFY_SOURCES ${VERIFY_SOURCES}
        TEST_GROUP ani_tests_without_bridges
    )

endfunction(ani_add_gtest)
