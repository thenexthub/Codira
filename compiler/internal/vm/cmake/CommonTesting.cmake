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
    set(noValues CONTAINS_MAIN NO_CORES RAPIDCHECK_ON USE_CATCH2)
    set(singleValues NAME OUTPUT_DIRECTORY TSAN_EXTRA_OPTIONS PANDA_STD_LIB ARK_BOOTCLASSPATH CUSTOM_PRERUN_ENVIRONMENT TEST_RUN_DIR)
    set(multiValues SOURCES INCLUDE_DIRS LIBRARIES SANITIZERS DEPS_TARGETS)
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
        if(ARG_USE_CATCH2)
            set(ARG_SOURCES ${ARG_SOURCES} ${PANDA_THIRD_PARTY_SOURCES_DIR}/rapidcheck/test/main.cpp)
        elseif (NOT CMAKE_CROSSCOMPILING)
            set(ARG_SOURCES ${ARG_SOURCES} ${PANDA_ROOT}/tests/gtest_launcher/main.cpp)
        endif()
    endif()

    if (ARG_RAPIDCHECK_ON)
        panda_add_executable(${ARG_NAME} RAPIDCHECK_ON ${ARG_SOURCES})
        set_target_properties(${ARG_NAME} PROPERTIES LINK_FLAGS "-frtti -fexceptions")
        target_compile_definitions(${ARG_NAME} PRIVATE PANDA_RAPIDCHECK)
        target_compile_options(${ARG_NAME} PRIVATE "-frtti" "-fexceptions" "-fPIC")
        target_compile_definitions(${ARG_NAME} PUBLIC PANDA_RAPIDCHECK)
    else()
        panda_add_executable(${ARG_NAME} ${ARG_SOURCES})
    endif()

    if (ARG_USE_CATCH2)
        target_compile_definitions(${ARG_NAME} PUBLIC PANDA_CATCH2)
    else()
        target_compile_definitions(${ARG_NAME} PUBLIC PANDA_GTEST)
        if (NOT ARG_CONTAINS_MAIN AND NOT CMAKE_CROSSCOMPILING)
            find_program(GDB_PATH gdb REQUIRED)
            target_compile_definitions(${ARG_NAME} PUBLIC -DTIMEOUT_SIGNAL=SIG${TIMEOUT_SIGNAL} -DGDB_PATH=${GDB_PATH})
        endif()
    endif()

    if(PANDA_CI_TESTING_MODE STREQUAL "Nightly")
        target_compile_definitions(${ARG_NAME} PUBLIC PANDA_NIGHTLY_TEST_ON)
    endif()
    # By default tests are just built, running is available either via an
    # umbrella target or via `ctest -R ...`. But one can always do something
    # like this if really needed:
    # add_custom_target(${ARG_NAME}_run
    #                  COMMENT "Run ${ARG_NAME}"
    #                  COMMAND ${CMAKE_CTEST_COMMAND}
    #                  DEPENDS ${ARG_NAME})
    if (ARG_USE_CATCH2)
        foreach(include_dir ${ARG_INCLUDE_DIRS} ${PANDA_THIRD_PARTY_SOURCES_DIR}/rapidcheck/ext/catch/single_include)
            target_include_directories(${ARG_NAME} PUBLIC ${include_dir})
        endforeach()
    else()
        foreach(include_dir ${ARG_INCLUDE_DIRS} ${PANDA_THIRD_PARTY_SOURCES_DIR}/googletest/googlemock/include)
            target_include_directories(${ARG_NAME} PUBLIC ${include_dir})
        endforeach()
    endif()

    if (NOT ARG_USE_CATCH2)
        if (CONTAINS_MAIN OR NOT CMAKE_CROSSCOMPILING)
            target_link_libraries(${ARG_NAME} gtest)
        else()
            target_link_libraries(${ARG_NAME} gtest_main)
        endif()
    endif()

    if (NOT (PANDA_TARGET_MOBILE OR PANDA_TARGET_OHOS))
       target_link_libraries(${ARG_NAME} pthread)
    endif()
    target_link_libraries(${ARG_NAME} ${ARG_LIBRARIES})

    add_dependencies(gtests_build ${ARG_NAME} ${ARG_DEPS_TARGETS})

    if (ARG_RAPIDCHECK_ON)
        target_link_libraries(${ARG_NAME} rapidcheck)
        target_link_libraries(${ARG_NAME} rapidcheck_catch)
        add_dependencies(${ARG_NAME} rapidcheck)
    endif()

    panda_add_sanitizers(TARGET ${ARG_NAME} SANITIZERS ${ARG_SANITIZERS})

    set(prlimit_prefix "")
    if (ARG_NO_CORES)
        set(prlimit_prefix prlimit --core=0)
    endif()
    set_target_properties(${ARG_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${ARG_OUTPUT_DIRECTORY}")

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

        set(CUSTOM_PRERUN_ENVIRONMENT "")
        if(DEFINED ARG_CUSTOM_PRERUN_ENVIRONMENT)
            set(CUSTOM_PRERUN_ENVIRONMENT ${ARG_CUSTOM_PRERUN_ENVIRONMENT})
        endif()

        set(TEST_RUN_DIR ${CMAKE_CURRENT_BINARY_DIR})
        if(DEFINED ARG_TEST_RUN_DIR)
            set(TEST_RUN_DIR ${ARG_TEST_RUN_DIR})
        endif()

        set(output_file "${ARG_OUTPUT_DIRECTORY}/${ARG_NAME}_gtest_output.txt")
        add_custom_target(${ARG_NAME}_gtests
                          COMMAND ${PANDA_STD_LIB} ${ARK_BOOTCLASSPATH} ${CUSTOM_PRERUN_ENVIRONMENT}
                                  ${tsan_options} ${prlimit_prefix} ${timeout_prefix}
                                  ${PANDA_RUN_PREFIX} "${ARG_OUTPUT_DIRECTORY}/${ARG_NAME}"
                                  --gtest_shuffle --gtest_death_test_style=threadsafe
                                  --gtest_brief=0
                                  >${output_file} 2>&1
                                  || (cat ${output_file} && false)
                          DEPENDS ${ARG_NAME} ${ARG_DEPS_TARGETS}
                          WORKING_DIRECTORY ${TEST_RUN_DIR}
       )
       add_dependencies(gtests ${ARG_NAME}_gtests)
    endif()
endfunction()
