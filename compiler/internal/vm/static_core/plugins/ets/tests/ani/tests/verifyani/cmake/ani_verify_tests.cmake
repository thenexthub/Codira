# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

add_custom_target(ani_verify_tests COMMENT "Common target to run VerifyANI tests")

# Add gtest-based tests to ani_verify_tests target.
#
# Example usage:
#   ani_verify_add_gtest(test_name
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
function(ani_verify_add_gtest TARGET)
    # Parse arguments
    cmake_parse_arguments(
        ARG
        ""
        "ETS_CONFIG"
        "CPP_SOURCES;ETS_SOURCES;LIBRARIES;TSAN_EXTRA_OPTIONS"
        ${ARGN}
    )

    # Check TARGET prefix
    set(TARGET_PREFIX "ani_verify_test_")
    string(FIND "${TARGET}" "${TARGET_PREFIX}" PREFIX_INDEX)
    if(NOT ${PREFIX_INDEX} EQUAL 0)
        message(FATAL_ERROR "TARGET (${TARGET}) should have '${TARGET_PREFIX}' prefix")
    endif()

    ani_add_gtest(${TARGET}
        TARGET_PREFIX ${TARGET_PREFIX}
        ETS_CONFIG ${ARG_ETS_CONFIG}
        ETS_SOURCES ${ARG_ETS_SOURCES}
        CPP_SOURCES ${ARG_CPP_SOURCES}
        LIBRARIES ${ARG_LIBRARIES} ani_gtest
        TSAN_EXTRA_OPTIONS ${ARG_TSAN_EXTRA_OPTIONS}
        INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani
    )
    add_dependencies(ani_verify_tests ${TARGET}_gtests)
endfunction(ani_verify_add_gtest)

# Add gtest-based generated tests to ani_verify_tests target.
#
# Example usage:
#   ani_verify_add_generated_tests(test_name_prefix
#     ETS_CONFIG path/to/arktsconfig.json
#     J2_CPP_SOURCE
#       tests/unit1_test.cpp.j2
#     J2_ETS_SOURCE
#       tests/unit1_test.ets.j2
#     PARAMS
#       "test_name_suffix1: arg1=1,arg2=utf8"
#       "test_name_suffix2: arg1=2,arg2=utf16"
#   )
function(ani_verify_add_generated_tests TARGET_PREF)
    cmake_parse_arguments(
        ARG
        ""
        "ETS_CONFIG;J2_CPP_SOURCE;J2_ETS_SOURCE"
        "PARAMS"
        ${ARGN}
    )

    set(CREATE_VERIFY_ANI_TEST_SCRIPT 
        "${PANDA_ROOT}/plugins/ets/tests/ani/tests/verifyani/cmake/test_gen_script/test_gen_script.sh")

    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

    set(${OUTPUT_ETS_CONFIG} "")
    if(NOT ${ARG_ETS_CONFIG} STREQUAL "")
        cmake_path(GET ARG_ETS_CONFIG FILENAME CONFIG_NAME)
        set(OUTPUT_ETS_CONFIG "${OUTPUT_DIR}/${CONFIG_NAME}")

        add_custom_command(
            OUTPUT "${OUTPUT_DIR}/${CONFIG_NAME}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ARG_ETS_CONFIG}"
                "${OUTPUT_ETS_CONFIG}"
            DEPENDS "${ARG_ETS_CONFIG}"
        )

        add_custom_target(copy_config DEPENDS "${OUTPUT_ETS_CONFIG}")
    endif()

    foreach(params_item ${ARG_PARAMS})
        string(REGEX MATCH "^[^:]*" TARGET_SUFF ${params_item})

        get_filename_component(OUTPUT_CPP ${ARG_J2_CPP_SOURCE} NAME_WE)

        set(OUTPUT_CPP_FILE "${OUTPUT_DIR}/${TARGET_PREF}_${TARGET_SUFF}.cpp")
        set(GEN_FILES "${OUTPUT_CPP_FILE}")

        set(PYTHON_PARAMS "--info"            "${params_item}" 
                          "--j2_cpp_source"   ${ARG_J2_CPP_SOURCE}
                          "--output_cpp"      ${OUTPUT_CPP_FILE}
                          "--static_core_dir" ${PANDA_ROOT}
        )

        if(NOT ${ARG_J2_ETS_SOURCE} STREQUAL "")
            get_filename_component(OUTPUT_ETS ${ARG_J2_ETS_SOURCE} NAME_WE)
            set(OUTPUT_ETS_FILE "${OUTPUT_DIR}/${TARGET_PREF}_${TARGET_SUFF}.ets")
            list(APPEND GEN_FILES "${OUTPUT_ETS_FILE}")
            list(APPEND PYTHON_PARAMS "--j2_ets_source" ${ARG_J2_ETS_SOURCE})
            list(APPEND PYTHON_PARAMS "--output_ets" ${OUTPUT_ETS_FILE})
        endif()

        add_custom_command(
            OUTPUT ${GEN_FILES}
            COMMAND ${CREATE_VERIFY_ANI_TEST_SCRIPT}
                    ${PANDA_ROOT}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${PYTHON_PARAMS}
            DEPENDS ${ARG_J2_CPP_SOURCE} ${ARG_J2_ETS_SOURCE}
            VERBATIM
        )
        set(TARGET ${TARGET_PREF}_${TARGET_SUFF})
        add_custom_target(generate_files_for_${TARGET} DEPENDS ${GEN_FILES})

        ani_verify_add_gtest(${TARGET}
            CPP_SOURCES "${OUTPUT_CPP_FILE}"
            ETS_SOURCES "${OUTPUT_ETS_FILE}"
            ETS_CONFIG  "${OUTPUT_ETS_CONFIG}"
        )

        add_dependencies(${TARGET} generate_files_for_${TARGET})
        if(TARGET copy_config)
            add_dependencies(${TARGET} copy_config)
        endif()
    endforeach()
endfunction(ani_verify_add_generated_tests)
