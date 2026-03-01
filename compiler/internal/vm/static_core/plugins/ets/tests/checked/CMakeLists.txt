#Copyright(c) 2022 - 2025 Huawei Device Co., Ltd.
#Licensed under the Apache License, Version 2.0(the "License");
#you may not use this file except in compliance with the License.
#You may obtain a copy of the License at
#
#http:  // www.apache.org/licenses/LICENSE-2.0
#
#Unless required by applicable law or agreed to in writing, software
#distributed under the License is distributed on an "AS IS" BASIS,
#WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#See the License for the specific language governing permissions and
#limitations under the License.

add_custom_target(ets_checked_tests COMMENT "Running ets core tests with checkers")
add_dependencies(ets_checked_tests ark)
add_subdirectory(jitinterface)

function(panda_add_checked_test_ets)
    set(prefix ARG)
    set(options SUPPORT_RELEASE JITINTERFACE NATIVE BOOT_SELF)
    set(singleValues NAME FILE LIBCORE OPT_LEVEL ETS_STRINGS_CONCAT)
    set(multiValues AUX_FILES EXT_FILES)
    cmake_parse_arguments(${prefix} "${options}" "${singleValues}" "${multiValues}" ${ARGN})

#Events does not work in Release mode
    if (NOT ARG_SUPPORT_RELEASE AND "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        return()
    endif()

    if(NOT PANDA_COMPILER_ENABLE)
        return()
    endif()

    if ("${ARG_NAME}" STREQUAL "")
        get_filename_component(TEST_NAME "${ARG_FILE}" NAME_WE)
        set(TEST_NAME "${TEST_NAME}.checked")
    else()
        set(TEST_NAME "${ARG_NAME}.checked")
    endif()
    set(CHECKER "${PANDA_ROOT}/tests/checked/checker.rb")
    if (CMAKE_VERBOSE_MAKEFILE)
        set(CHECKER_VERBOSE "-v")
    else()
        set(CHECKER_VERBOSE "")
    endif()
    set(TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}")
    set(BINARY_FILE "${TEST_DIR}/test.abc")
    set(BUILD_LOG   "${TEST_DIR}/build.log")

    if (NOT DEFINED ARG_FILE)
        message(FATAL_ERROR "Mandatory FILE argument is not defined.")
    endif()

    file(MAKE_DIRECTORY "${TEST_DIR}")
    set(ABC_TARGET ${TEST_NAME}-ets-es2panda)
    get_filename_component(TEST_TYPE "${ARG_FILE}" EXT)
    if (NOT DEFINED ARG_OPT_LEVEL)
        set(ARG_OPT_LEVEL 2)
    endif()

    if (NOT DEFINED ARG_ETS_STRINGS_CONCAT)
        set(ARG_ETS_STRINGS_CONCAT "true")
    endif()

    if (${TEST_TYPE} MATCHES "ets")
        compile_ets_code(${ARG_FILE} ${BINARY_FILE} ${ABC_TARGET} OPT_LEVEL ${ARG_OPT_LEVEL} ETS_STRINGS_CONCAT ${ARG_ETS_STRINGS_CONCAT})
    else()
#Compile assembly file
        add_custom_command(OUTPUT "${BINARY_FILE}"
                COMMENT "Building ${TEST_NAME}"
                COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark_asm> --log-file ${BUILD_LOG} ${ARG_FILE} ${BINARY_FILE}
                DEPENDS ${assembler} "${ARG_FILE}"
                WORKING_DIRECTORY "${ARG_WORKING_DIR}")
    endif()

    set(EXT_FILES "")
    foreach(APP_FILE ${ARG_EXT_FILES})
        get_filename_component(APP_FILE_BASE "${APP_FILE}" NAME_WE)
        set(APP_FILE_OUTPUT "${TEST_DIR}/${APP_FILE_BASE}.abc")
        set(ABC_APP_TARGET ${TEST_NAME}-${APP_FILE_BASE}-ets-es2panda)
        compile_ets_code(${APP_FILE} ${APP_FILE_OUTPUT} ${ABC_APP_TARGET})
#TODO(pishin) : pass external files via-- app - panda - files(now it doesn't work)
        list(APPEND stdlibs "${APP_FILE_OUTPUT}")
        list(APPEND EXT_FILES "${APP_FILE_OUTPUT}")
    endforeach()

    if (${TEST_TYPE} MATCHES "ets")
        set(COMMAND_TOKEN "//!")
    else()
        set(COMMAND_TOKEN "#!")
    endif()

    set(BOOT_PANDA_FILES ${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc)
    if (ARG_BOOT_SELF)
        set(BOOT_PANDA_FILES ${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc:../test.abc)
    endif()

    set(OPTIONS "--load-runtimes=ets" "--run-gc-in-place" "--coroutine-workers-count=1" "--boot-panda-files=${BOOT_PANDA_FILES}")
    set(ES2PANDA_OPTIONS --thread=0 --extension=ets)
    if (NOT PANDA_CI_TESTING_MODE STREQUAL "Nightly")
        set (OPTIONS ${OPTIONS} "--compiler-check-final=true")
    endif()

    if (PANDA_LLVM_AOT)
        set(WITH_LLVM "--with-llvm")
    endif()

    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        set(RELEASE_OPT "--release")
    endif()

    if (PANDA_TARGET_AMD64)
        set(ARCHITECTURE "x64")
    elseif (PANDA_TARGET_ARM64)
        set(ARCHITECTURE "arm64")
    else()
        set(ARCHITECTURE "arm32")
    endif()

    if (ARK_HYBRID OR NOT PANDA_32_BIT_MANAGED_POINTER)
        set (CHECKER_BUILD_CONFIG "Hybrid")
    else()
        set (CHECKER_BUILD_CONFIG "Static")
    endif()

    set(PANDA_OPTIONS "${OPTIONS}")
    if (ARG_NATIVE)
        set(PANDA_OPTIONS ${PANDA_OPTIONS} "--ets.native-library-path=${TEST_DIR}")
    endif()

#Run checker
    add_custom_target(${TEST_NAME}
                      COMMAND ${CHECKER} --source ${ARG_FILE}
                                         --panda $<TARGET_FILE:ark>
                                         --paoc $<TARGET_FILE:ark_aot>
                                         --frontend ${es2panda_bin}
                                         --run-prefix \"${PANDA_RUN_PREFIX}\"
                                         --test-file ${BINARY_FILE}
                                         --panda-options \"${PANDA_OPTIONS}\"
                                         --paoc-options \"${OPTIONS}\"
                                         --frontend-options \"${ES2PANDA_OPTIONS}\"
                                         --command-token \"${COMMAND_TOKEN}\"
                                         --arch ${ARCHITECTURE}
                                         --build-config ${CHECKER_BUILD_CONFIG}
                                         ${RELEASE_OPT}
                                         ${WITH_LLVM}
                                         ${CHECKER_VERBOSE}
                      WORKING_DIRECTORY ${TEST_DIR}
                      COMMENT "Running ${TEST_NAME} checked test"
                      DEPENDS ${BINARY_FILE} ${EXT_FILES})

    if (ARG_JITINTERFACE)
        add_dependencies(${TEST_NAME} jitinterface)
        add_dependencies(${TEST_NAME} string_tlab_repeat)
    endif()

    if (ARG_NATIVE)
        cmake_path(REMOVE_EXTENSION ARG_FILE LAST_ONLY OUTPUT_VARIABLE NATIVE_FILE)
        cmake_path(GET NATIVE_FILE FILENAME NATIVE_LIB)
        set(NATIVE_FILE "${NATIVE_FILE}.cpp")
        set(NATIVE_LIB "checked_test_${NATIVE_LIB}")

        panda_add_library("${NATIVE_LIB}" SHARED "${NATIVE_FILE}")
        set_target_properties("${NATIVE_LIB}" PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${TEST_DIR}")
        panda_target_include_directories("${NATIVE_LIB}" PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani)
        add_dependencies(${TEST_NAME} "${NATIVE_LIB}")
    endif()

    add_dependencies(ets_checked_tests ${TEST_NAME})

    foreach(AUX_FILE ${ARG_AUX_FILES})
        get_filename_component(AUX_NAME "${AUX_FILE}" NAME_WE)
        set(AUX_NAME "${AUX_NAME}.checked")
        add_dependencies(${TEST_NAME} ${AUX_NAME})
    endforeach()

endfunction()

#Disable checked tests with sanitizers for arm64, since it takes too long time running.
if (PANDA_TARGET_AMD64 OR NOT PANDA_ARM64_TESTS_WITH_SANITIZER)
    if (PANDA_TARGET_AMD64 OR PANDA_TARGET_ARM64)
#inlining of the following intrinsics is only enabled for these two targets
#AOT mode for arm32 is not supported
        if (NOT PANDA_PRODUCT_BUILD)
#Disabled as stdlib API was required to change.issue 25232
#panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR} / string_tlab_allocations.ets JITINTERFACE)
            panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/string_tlab_repeat.ets JITINTERFACE)
        endif()
#LoadAndInitClass in AOT mode for arm32 is not supported
    endif()
endif()

panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ani/direct_native.ets NATIVE SUPPORT_RELEASE)
panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ani/quick_native.ets NATIVE SUPPORT_RELEASE)
panda_add_checked_test_ets(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ani/unsafe_memory.ets NATIVE BOOT_SELF SUPPORT_RELEASE)

if (NOT ENABLE_UNIT_TESTS_FULL_COVERAGE)
endif()
