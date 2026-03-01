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

add_custom_target(ets_interop_js_gtests COMMENT "Run ets_interop_js_gtests Gtests on ArkJSVM")
add_dependencies(ets_gtests ets_interop_js_gtests)
add_dependencies(ets_interop_tests ets_interop_js_gtests)

add_custom_target(ets_interop_js_tests COMMENT "Run ets_interop_js_tests tests on ArkJSVM")
add_dependencies(ets_tests ets_interop_js_tests)
add_dependencies(ets_interop_tests ets_interop_js_tests)

function(compile_dynamic_file TARGET)
    cmake_parse_arguments(
        ARG
        ""
        ""
        "SOURCES;OUTPUT_DIR;OUTPUT_ABC_PATHS;COMPILE_OPTION"
        ${ARGN}
    )

    if(NOT DEFINED ARG_SOURCES)
        message(FATAL_ERROR "SOURCES should be passed in compile_dynamic_file")
    endif()

    set(BUILD_DIR)
    set(ABC_FILES)

    if(NOT DEFINED ARG_OUTPUT_DIR)
        set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
    else()
        set(BUILD_DIR ${ARG_OUTPUT_DIR})
    endif()

    foreach(DYNAMIC_SOURCE ${ARG_SOURCES})
        set(CUR_OUTPUT_ABC)
        get_filename_component(CLEAR_NAME ${DYNAMIC_SOURCE} NAME_WLE)

        # determine output path of abc file
        if (DEFINED ARG_OUTPUT_DIR)
            set(CUR_OUTPUT_ABC ${BUILD_DIR}/${CLEAR_NAME}.abc)
        else()
            get_filename_component(DIR_NAME "${DYNAMIC_SOURCE}" DIRECTORY)
            if (NOT ${DIR_NAME} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
                # in this case source file in in subdirectory
                string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" DIR_NAME "${DIR_NAME}")
                set(CUR_OUTPUT_ABC ${BUILD_DIR}/${DIR_NAME}/${CLEAR_NAME}.abc)
            else()
                set(CUR_OUTPUT_ABC ${BUILD_DIR}/${CLEAR_NAME}.abc)
            endif()
        endif()

        list(APPEND ABC_FILES ${CUR_OUTPUT_ABC})

        add_custom_command(
            OUTPUT ${CUR_OUTPUT_ABC}
            COMMENT "${TARGET}: Convert dynamic file to ${CUR_OUTPUT_ABC}"
            COMMAND mkdir -p ${BUILD_DIR}
            COMMAND ${ES2ABC} ${ARG_COMPILE_OPTION} ${DYNAMIC_SOURCE} --output=${CUR_OUTPUT_ABC}
            DEPENDS ${DYNAMIC_SOURCE}
        )
    endforeach()

    set(${ARG_OUTPUT_ABC_PATHS} ${ABC_FILES} PARENT_SCOPE)

    add_custom_target(${TARGET} DEPENDS ${ABC_FILES})
endfunction(compile_dynamic_file)

# Add Gtest-based tests to ets_interop_js_gtests target.
#
# Example usage:
#   panda_ets_interop_js_gtest(test_name
#     CPP_SOURCES
#       tests/unit1_test.cpp
#       tests/unit2_test.cpp
#     ETS_SOURCES
#       tests/unit1_test.ets
#       tests/unit2_test.ets
#     LIBRARIES
#       lib_target1
#       lib_target2
#     ETS_CONFIG
#       path/to/arktsconfig.json
#     PACKAGE_NAME
#       unit1_test
#   )
function(panda_ets_interop_js_gtest TARGET)
    # Parse arguments
    cmake_parse_arguments(
        ARG
        "COMPILATION_JS_WITH_CODES_ON"
        "ETS_CONFIG;PACKAGE_NAME;VERIFY_SOURCES"
        "CPP_SOURCES;ETS_SOURCES;JS_SOURCES;TS_SOURCES;JS_TEST_SOURCE;ASM_SOURCE;LIBRARIES"
        ${ARGN}
    )

    set(SO_FILES_OUTPUT "${INTEROP_TESTS_DIR}/module/lib/")

    panda_ets_interop_js_plugin(${TARGET}
        SOURCES ${ARG_CPP_SOURCES}
        LIBRARIES ets_interop_js_gtest ets_interop_js_napi ${ARG_LIBRARIES}
        LIBRARY_OUTPUT_DIRECTORY ${SO_FILES_OUTPUT}
        OUTPUT_SUFFIX ".so"
    )

    if(DEFINED ARG_ETS_SOURCES)
        set(TARGET_GTEST_PACKAGE ${TARGET}_gtest_package)
        panda_ets_package_gtest(${TARGET_GTEST_PACKAGE}
            VERIFY_SOURCES ${ARG_VERIFY_SOURCES}
            ETS_SOURCES ${ARG_ETS_SOURCES}
            ETS_CONFIG ${ARG_ETS_CONFIG}
        )
        add_dependencies(${TARGET} ${TARGET_GTEST_PACKAGE})
    endif()
    set(COMPILED_ABCPATH ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_gtest_package/src/classes.abc)

    set(JS_COMPILATION_OPTIONS --module --merge-abc --enable-ets-implements)
    if(ARG_COMPILATION_JS_WITH_CODES_ON)
        set(JS_COMPILATION_OPTIONS --commonjs)
    endif()

    set(ALL_SOURCES)

    if(DEFINED ARG_JS_SOURCES)
        list(APPEND ALL_SOURCES ${ARG_JS_SOURCES})
    endif()

    if(DEFINED ARG_TS_SOURCES)
        list(APPEND ALL_SOURCES ${ARG_TS_SOURCES})
    endif()

    if(ALL_SOURCES)
        compile_dynamic_file(${TARGET}_dynamic_modules
            SOURCES ${ALL_SOURCES}
            COMPILE_OPTION ${JS_COMPILATION_OPTIONS}
        )
    endif()

    if(DEFINED ARG_ASM_SOURCE)
        get_filename_component(ASM_DIR_PATH ${ARG_ASM_SOURCE} DIRECTORY)
        get_filename_component(ASM_FILE_NAME ${ARG_ASM_SOURCE} NAME)
        add_panda_assembly(TARGET ${TARGET}_asm_abc INDIR ${ASM_DIR_PATH} SOURCE ${ASM_FILE_NAME}
                           OUTDIR ${CMAKE_CURRENT_BINARY_DIR} TARGETNAME ${TARGET}_asm)
        set(ARK_ETS_INTEROP_JS_GTEST_ASM_ABC_PATH "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_asm.abc")
    endif()

    # if not set PACKAGE_NAME, using first ets file as its name;
    if(DEFINED ARG_ETS_SOURCES)
        set(ETS_SOURCES_NUM)
        list(LENGTH ARG_ETS_SOURCES ETS_SOURCES_NUM)
        if(NOT DEFINED ARG_PACKAGE_NAME AND ${ETS_SOURCES_NUM} EQUAL 1)
            list(GET ARG_ETS_SOURCES 0 PACKATE_FILE)
            get_filename_component(ARG_PACKAGE_NAME ${PACKATE_FILE} NAME_WE)
        elseif(NOT DEFINED ARG_PACKAGE_NAME)
            message(FATAL_ERROR "Please provide PACKAGE_NAME for ${TARGET}")
        endif()

        # Add launcher <${TARGET}_gtests> target
        set(ARK_ETS_INTEROP_JS_GTEST_ABC_PATH_PATH ${PANDA_BINARY_ROOT}/abc-gtests/${TARGET_GTEST_PACKAGE}.zip)
    endif()

    panda_ets_add_gtest(
        NAME ${TARGET}
        NO_EXECUTABLE
        NO_CORES
        CUSTOM_PRERUN_ENVIRONMENT
            "LD_LIBRARY_PATH=${PANDA_BINARY_ROOT}/lib/interop_js/:${PANDA_BINARY_ROOT}/lib/"
            "JS_ABC_OUTPUT_PATH=${CMAKE_CURRENT_BINARY_DIR}"
            "INTEROP_TEST_BUILD_DIR=${PANDA_BINARY_ROOT}/tests/ets_interop_js"
            "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
            "ARK_ETS_INTEROP_JS_GTEST_ABC_PATH=${ARK_ETS_INTEROP_JS_GTEST_ABC_PATH_PATH}"
            "ARK_ETS_INTEROP_JS_GTEST_ASM_ABC_PATH=${ARK_ETS_INTEROP_JS_GTEST_ASM_ABC_PATH}"
            "ARK_ETS_INTEROP_JS_GTEST_SOURCES=${CMAKE_CURRENT_SOURCE_DIR}"
            "ARK_ETS_INTEROP_JS_GTEST_DIR=${INTEROP_TESTS_DIR}"
            "FIXED_ISSUES=${FIXED_ISSUES}"
            "PACKAGE_NAME=${ARG_PACKAGE_NAME}"
            "ARK_ETS_INTEROP_JS_TARGET_GTEST_PACKAGE=${TARGET_GTEST_PACKAGE}"
            "ARK_ETS_INTEROP_JS_COMPILED_ABC_PATH=${COMPILED_ABCPATH}"
        LAUNCHER
            ${ARK_JS_NAPI_CLI}
            --stub-file=${ARK_JS_STUB_FILE}
            --enable-force-gc=false
            --entry-point=gtest_launcher
            ${INTEROP_TESTS_DIR}/gtest_launcher.abc
            ${TARGET}
        DEPS_TARGETS ${TARGET} ets_interop_js_gtest_launcher
        TEST_RUN_DIR ${INTEROP_TESTS_DIR}
        OUTPUT_DIRECTORY ${INTEROP_TESTS_DIR}
    )

    if(DEFINED ARG_JS_SOURCES OR ARG_TS_SOURCES)
        add_dependencies(${TARGET}_gtests ${TARGET}_dynamic_modules)
    endif()

    if(DEFINED ARG_ASM_SOURCE)
        add_dependencies(${TARGET}_gtests ${TARGET}_asm_abc)
    endif()

    add_dependencies(ets_interop_js_gtests ${TARGET}_gtests)
endfunction(panda_ets_interop_js_gtest)

function(panda_ets_interop_js_test TARGET)
    # Parse arguments
    cmake_parse_arguments(
        ARG
        "COMPILATION_JS_WITH_CODES_ON"
        "JS_LAUNCHER;ETS_CONFIG;DYNAMIC_ABC_OUTPUT_DIR;PACKAGE_NAME"
        "ETS_SOURCES;JS_SOURCES;ABC_FILE;LAUNCHER_ARGS;"
        ${ARGN}
    )

    if(NOT DEFINED ARG_JS_LAUNCHER)
        message("ARG_JS_LAUNCHER should be defined")
        return()
    endif()

    set(TARGET_TEST_PACKAGE ${TARGET}_test_package)
    panda_ets_package(${TARGET_TEST_PACKAGE}
        ABC_FILE ${ARG_ABC_FILE}
        ETS_SOURCES ${ARG_ETS_SOURCES}
        ETS_CONFIG ${ARG_ETS_CONFIG}
    )

    set(JS_COMPILATION_OPTIONS --module --merge-abc --enable-ets-implements)
    if(ARG_COMPILATION_JS_WITH_CODES_ON)
        set(JS_COMPILATION_OPTIONS --commonjs)
    endif()

    if(DEFINED ARG_JS_SOURCES)
        set(COMPILE_OPTIONS SOURCES ${ARG_JS_SOURCES} COMPILE_OPTION ${JS_COMPILATION_OPTIONS})
        if (DEFINED ARG_DYNAMIC_ABC_OUTPUT_DIR)
            list(APPEND COMPILE_OPTIONS ${COMPILE_OPTIONS} OUTPUT_DIR ${ARG_DYNAMIC_ABC_OUTPUT_DIR})
        endif()
        compile_dynamic_file(${TARGET}_js_modules ${COMPILE_OPTIONS})
    endif()
    # if not set PACKAGE_NAME, using first ets file as its name;
    set(ETS_SOURCES_NUM)
    list(LENGTH ARG_ETS_SOURCES ETS_SOURCES_NUM)
    if(NOT DEFINED ARG_PACKAGE_NAME AND ${ETS_SOURCES_NUM} EQUAL 1)
        list(GET ARG_ETS_SOURCES 0 PACKATE_FILE)
        get_filename_component(ARG_PACKAGE_NAME ${PACKATE_FILE} NAME_WE)
    elseif(NOT DEFINED ARG_PACKAGE_NAME)
        message(FATAL_ERROR "Please provide PACKAGE_NAME  for ${TARGET}")
    endif()
    set(COMPILED_LAUNCHER_NAME ${TARGET}_launcher_abc_name)
    set(COMPILE_OPTIONS SOURCES ${ARG_JS_LAUNCHER} OUTPUT_ABC_PATHS ${COMPILED_LAUNCHER_NAME} COMPILE_OPTION ${JS_COMPILATION_OPTIONS})
    if (DEFINED ARG_DYNAMIC_ABC_OUTPUT_DIR)
        list(APPEND COMPILE_OPTIONS ${COMPILE_OPTIONS} OUTPUT_DIR ${ARG_DYNAMIC_ABC_OUTPUT_DIR})
    endif()
    compile_dynamic_file(${TARGET}_js_launcher ${COMPILE_OPTIONS})

    # Make symbolic links to convinient work with requireNapiPreview
    set(SO_FILES_LINK_PATH "${CMAKE_CURRENT_BINARY_DIR}/module/")
    set(INTEROP_LIB_SOURCE "${PANDA_BINARY_ROOT}/lib/module/ets_interop_js_napi.so")
    set(INTEROP_HELPER_LIB_SOURCE "${PANDA_BINARY_ROOT}/lib/interop_js/libinterop_test_helper.so")

    add_custom_target(${TARGET}_create_symlinks
        COMMAND mkdir -p ${SO_FILES_LINK_PATH}
                && ln -sf ${INTEROP_LIB_SOURCE} ${INTEROP_HELPER_LIB_SOURCE} -t ${SO_FILES_LINK_PATH}
        DEPENDS ets_interop_js_napi ${INTEROP_HELPER_LIB_SOURCE}
    )

    set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_interop_js_output.txt")

    set(CUSTOM_PRERUN_ENVIRONMENT
        "LD_LIBRARY_PATH=${PANDA_BINARY_ROOT}/lib/interop_js/:${PANDA_BINARY_ROOT}/lib/"
        "ARK_ETS_INTEROP_JS_GTEST_ABC_PATH=${PANDA_BINARY_ROOT}/abc/${TARGET_TEST_PACKAGE}.zip"
        "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
        "PACKAGE_NAME=${ARG_PACKAGE_NAME}"
    )

    get_filename_component(LAUNCHER_CLEAR_NAME ${ARG_JS_LAUNCHER} NAME_WLE)

    add_custom_target(${TARGET}
        COMMAND
            ${CUSTOM_PRERUN_ENVIRONMENT}
            ${ARK_JS_NAPI_CLI}
            --stub-file=${ARK_JS_STUB_FILE}
            --enable-force-gc=false
            --entry-point=${LAUNCHER_CLEAR_NAME}
            ${${COMPILED_LAUNCHER_NAME}}
            ${ARG_LAUNCHER_ARGS}
            > ${OUTPUT_FILE} 2>&1 || (cat ${OUTPUT_FILE} && false)
        DEPENDS
            ${TARGET}_js_launcher
            ${TARGET}_create_symlinks
            ${TARGET_TEST_PACKAGE}
            ets_interop_js_napi
    )

    if(DEFINED ARG_JS_SOURCES)
        add_dependencies(${TARGET} ${TARGET}_js_modules)
    endif()
    add_dependencies(ets_interop_js_tests ${TARGET})
endfunction(panda_ets_interop_js_test)