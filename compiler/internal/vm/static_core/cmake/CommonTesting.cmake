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
# Convenience functions for testing Panda.

include(${CMAKE_CURRENT_LIST_DIR}/PandaCmakeFunctions.cmake)

function(common_add_gtest)
    if(NOT PANDA_WITH_TESTS)
        return()
    endif()

    set(prefix ARG)
    set(noValues CONTAINS_MAIN NO_CORES RAPIDCHECK_ON NO_EXECUTABLE NO_WARNING_FILTER)
    set(singleValues NAME OUTPUT_DIRECTORY TSAN_EXTRA_OPTIONS PANDA_STD_LIB ARK_BOOTCLASSPATH TEST_RUN_DIR TEST_GROUP STASH_LIST)
    set(multiValues SOURCES INCLUDE_DIRS LIBRARIES SANITIZERS DEPS_TARGETS CUSTOM_PRERUN_ENVIRONMENT LAUNCHER)
    set(TIMEOUT_SIGNAL USR1)

    cmake_parse_arguments(${prefix}
                          "${noValues}"
                          "${singleValues}"
                          "${multiValues}"
                          ${ARGN})

    if (ARG_RAPIDCHECK_ON AND DEFINED DONT_USE_RAPIDCHECK)
      return()
    endif()

    if (NOT DEFINED ARG_OUTPUT_DIRECTORY)
        message(FATAL_ERROR "OUTPUT_DIRECTORY is not defined")
    endif()

    if (NOT ARG_CONTAINS_MAIN)
        if (NOT CMAKE_CROSSCOMPILING)
            set(ARG_SOURCES ${ARG_SOURCES} ${PANDA_ROOT}/tests/gtest_launcher/main.cpp)
        endif()
    endif()

    if (ARG_RAPIDCHECK_ON)
        if(NOT ARG_NO_EXECUTABLE)
		    panda_add_executable(${ARG_NAME} RAPIDCHECK_ON OUTPUT_DIRECTORY ${ARG_OUTPUT_DIRECTORY} EXCLUDE_FROM_ALL ${ARG_SOURCES})
        endif()
        set_target_properties(${ARG_NAME} PROPERTIES LINK_FLAGS "-fno-rtti -fexceptions")
        panda_target_compile_definitions(${ARG_NAME} PRIVATE PANDA_RAPIDCHECK)
        panda_target_compile_options(${ARG_NAME} PRIVATE "-fno-rtti" "-fexceptions" "-fPIC")
        panda_target_compile_definitions(${ARG_NAME} PUBLIC PANDA_RAPIDCHECK)
    else()
        if(NOT ARG_NO_EXECUTABLE)
		    panda_add_executable(${ARG_NAME} OUTPUT_DIRECTORY ${ARG_OUTPUT_DIRECTORY} EXCLUDE_FROM_ALL ${ARG_SOURCES})
        endif()
    endif()

    if(NOT ARG_NO_EXECUTABLE)
        string(REPLACE "${PANDA_BINARY_ROOT}/" "" bin_path ${ARG_OUTPUT_DIRECTORY})
        if(NOT DEFINED ARG_STASH_LIST)
            set(ARG_STASH_LIST stash_list)
        endif()
        set_property(GLOBAL APPEND PROPERTY ${ARG_STASH_LIST} "${bin_path}/${ARG_NAME}")
    endif()

    set(ARG_USE_GTESTS ON)

    if (ARG_USE_GTESTS)
        panda_target_compile_definitions(${ARG_NAME} PUBLIC PANDA_GTEST)
        if (NOT ARG_CONTAINS_MAIN AND NOT CMAKE_CROSSCOMPILING)
            find_program(GDB_PATH gdb REQUIRED)
            panda_target_compile_definitions(${ARG_NAME} PUBLIC -DTIMEOUT_SIGNAL=SIG${TIMEOUT_SIGNAL} -DGDB_PATH=${GDB_PATH})
        endif()
    endif()

    if(PANDA_CI_TESTING_MODE STREQUAL "Nightly")
        panda_target_compile_definitions(${ARG_NAME} PUBLIC PANDA_NIGHTLY_TEST_ON)
    endif()
    # By default tests are just built, running is available either via an
    # umbrella target or via `ctest -R ...`. But one can always do something
    # like this if really needed:
    # add_custom_target(${ARG_NAME}_run
    #                  COMMENT "Run ${ARG_NAME}"
    #                  COMMAND ${CMAKE_CTEST_COMMAND}
    #                  DEPENDS ${ARG_NAME})
    if (ARG_USE_GTESTS)
        foreach(include_dir ${ARG_INCLUDE_DIRS})
            panda_target_include_directories(${ARG_NAME} PUBLIC ${include_dir})
        endforeach()
        panda_target_include_directories(${ARG_NAME} SYSTEM PUBLIC
            ${PANDA_THIRD_PARTY_SOURCES_DIR}/googletest/googlemock/include
        )
        panda_target_link_libraries(${ARG_NAME} gmock)
    endif()

    if (ARG_USE_GTESTS)
        if (CONTAINS_MAIN OR NOT CMAKE_CROSSCOMPILING)
            panda_target_link_libraries(${ARG_NAME} gtest)
        else()
            panda_target_link_libraries(${ARG_NAME} gtest_main)
        endif()
    endif()

    if (NOT (PANDA_TARGET_MOBILE OR PANDA_TARGET_OHOS))
        panda_target_link_libraries(${ARG_NAME} pthread)
    endif()
    panda_target_link_libraries(${ARG_NAME} ${ARG_LIBRARIES})

    add_dependencies(gtests_build ${ARG_NAME} ${ARG_DEPS_TARGETS})

    if (ARG_RAPIDCHECK_ON)
        panda_target_link_libraries(${ARG_NAME} rapidcheck)
        panda_target_link_libraries(${ARG_NAME} rapidcheck_gtest)
        panda_target_include_directories(${ARG_NAME} SYSTEM
            PRIVATE ${PANDA_THIRD_PARTY_SOURCES_DIR}/rapidcheck/include
        )
        add_dependencies(${ARG_NAME} rapidcheck_gtest)
        add_dependencies(${ARG_NAME} rapidcheck)
    endif()

    panda_add_sanitizers(TARGET ${ARG_NAME} SANITIZERS ${ARG_SANITIZERS})

    set(prlimit_prefix "")
    if (ARG_NO_CORES)
        set(prlimit_prefix prlimit --core=0)
    endif()

    set(tsan_options "")
    if ("thread" IN_LIST PANDA_SANITIZERS_LIST)
        if (DEFINED ENV{TSAN_OPTIONS})
            set(tsan_options "TSAN_OPTIONS=$ENV{TSAN_OPTIONS},${ARG_TSAN_EXTRA_OPTIONS}")
        endif()
    endif()

    # Yes, this is hardcoded. No, do not ask for an option. Go and fix your tests.
    if (PANDA_CI_TESTING_MODE STREQUAL "Nightly")
        set(timeout_prefix timeout --preserve-status --signal=${TIMEOUT_SIGNAL} --kill-after=30s 40m)
    else ()
        set(timeout_prefix timeout --preserve-status --signal=${TIMEOUT_SIGNAL} --kill-after=30s 20m)
    endif()

    if (ARG_RAPIDCHECK_ON)
        add_custom_target(${ARG_NAME}_rapidcheck_tests
                          COMMAND ${tsan_options} ${prlimit_prefix} ${timeout_prefix}
                                  ${PANDA_RUN_PREFIX} "${ARG_OUTPUT_DIRECTORY}/${ARG_NAME}"
                          DEPENDS ${ARG_NAME} ${ARG_DEPS_TARGETS}
        )
        add_dependencies(gtests ${ARG_NAME}_rapidcheck_tests)
    else()
        set(PANDA_STD_LIB "")
        if (DEFINED ARG_PANDA_STD_LIB)
            set(PANDA_STD_LIB "PANDA_STD_LIB=${ARG_PANDA_STD_LIB}")
        endif()

        set(ARK_BOOTCLASSPATH "")
        if (DEFINED ARG_ARK_BOOTCLASSPATH)
            set(ARK_BOOTCLASSPATH "ARK_BOOTCLASSPATH=${ARG_ARK_BOOTCLASSPATH}")
        endif()

        set(TEST_RUN_DIR ${CMAKE_CURRENT_BINARY_DIR})
        if(DEFINED ARG_TEST_RUN_DIR)
            set(TEST_RUN_DIR ${ARG_TEST_RUN_DIR})
        endif()

        if(NOT DEFINED ARG_LAUNCHER)
            set(ARG_LAUNCHER "${ARG_OUTPUT_DIRECTORY}/${ARG_NAME}")
        endif()

        set(output_file "${ARG_OUTPUT_DIRECTORY}/${ARG_NAME}_gtest_output.txt")
        
        # Some tests (examples: Logger.FileLogging) can work only with the "fast" mode.
        # For such tests, it is necessary to disable filter using the NO_WARNING_FILTER flag.
        # By default, the filter is enabled.
        if(NOT ARG_NO_WARNING_FILTER)
            set(FILTER ! grep "-q"
                "'\\[WARNING] .* Death tests use fork(), which is unsafe particularly in a threaded context.'"
                "${output_file}"
            )
        else()
            set(FILTER true)
        endif()
        set(PRINT_FILTER_TRIGGERING_MESSAGE 
            echo "Fail: using \\'fast\\' mode in multithreading test detected: please, check warnings below"
        )
        
        add_custom_target(${ARG_NAME}_gtests
                          COMMAND ${PANDA_STD_LIB} ${ARK_BOOTCLASSPATH} ${ARG_CUSTOM_PRERUN_ENVIRONMENT}
                                  ${tsan_options} ${prlimit_prefix} ${timeout_prefix}
                                  ${PANDA_RUN_PREFIX} ${ARG_LAUNCHER}
                                  --gtest_shuffle --gtest_death_test_style=threadsafe
                                  --gtest_brief=0
                                  >${output_file} 2>&1
                                  && (${FILTER} || (${PRINT_FILTER_TRIGGERING_MESSAGE} && false))
                                  || (cat ${output_file} && false)
                          DEPENDS ${ARG_NAME} ${ARG_DEPS_TARGETS}
                          WORKING_DIRECTORY ${TEST_RUN_DIR}
        )

        if(NOT DEFINED ARG_TEST_GROUP)
            set(ARG_TEST_GROUP ${DEFAULT_TEST_GROUP})
        endif()
        add_dependencies(${ARG_TEST_GROUP} ${ARG_NAME}_gtests)
    endif()
endfunction()
