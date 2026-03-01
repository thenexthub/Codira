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

set(INTEROP_TESTS_GENERATED_DIR "${PANDA_BINARY_ROOT}/plugins/ets/tests/ets_interop_js/generated")
set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/tests/checked/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/checked/arktsconfig.in.json ${ETS_CONFIG})

add_custom_target(ets_interop_js_checked_tests COMMENT "Run ets_interop_js checked tests")
add_dependencies(ets_tests ets_interop_js_checked_tests)
add_dependencies(ets_interop_tests ets_interop_js_checked_tests)

function(panda_ets_interop_js_checked_test)
    cmake_parse_arguments(
        ARG
        "COMPILATION_JS_WITH_CODES_ON"
        "FILE;PACKAGE_NAME"
        "JS_SOURCES;TS_SOURCES;DYNAMIC_ABC_OUTPUT_DIR"
        ${ARGN}
    )

    if (NOT DEFINED ARG_FILE)
        message(FATAL_ERROR "Mandatory FILE argument is not defined.")
    endif()

    get_filename_component(TARGET "${ARG_FILE}" NAME_WE)
    set(TARGET "interop_${TARGET}.checked")

    set(TARGET_TEST_PACKAGE ${TARGET}_test_package)
    # NOTE (kkonsw): temporary disabling all other checks
    set(ETS_VERIFICATOR_ERRORS "NodeHasParent:EveryChildHasValidParent:VariableHasScope:NodeHasType:IdentifierHasVariable:ReferenceTypeAnnotationIsNull:ArithmeticOperationValid:SequenceExpressionHasLastType:ForLoopCorrectlyInitialized:VariableHasEnclosingScope:ModifierAccessValid")
    # NOTE(dkofanov): `ImportExportAccessValid` need to be fixed
    # set(ETS_VERIFICATOR_ERRORS "${ETS_VERIFICATOR_ERRORS}:ImportExportAccessValid")
    panda_ets_package(${TARGET_TEST_PACKAGE}
        ETS_SOURCES ${ARG_FILE}
        ETS_CONFIG ${ETS_CONFIG}
        ETS_VERIFICATOR_ERRORS ${ETS_VERIFICATOR_ERRORS}
    )

    set(JS_COMPILATION_OPTIONS --module --merge-abc --enable-ets-implements)
    if(ARG_COMPILATION_JS_WITH_CODES_ON)
        set(JS_COMPILATION_OPTIONS --commonjs)
    endif()

    set(ALL_DYNAMIC_SOURCES)

    if(DEFINED ARG_JS_SOURCES)
        list(APPEND ALL_DYNAMIC_SOURCES ${ARG_JS_SOURCES})
    endif()

    if(DEFINED ARG_TS_SOURCES)
        list(APPEND ALL_DYNAMIC_SOURCES ${ARG_TS_SOURCES})
    endif()

    if(ALL_DYNAMIC_SOURCES)
        set(COMPILE_OPTIONS SOURCES ${ALL_DYNAMIC_SOURCES} COMPILE_OPTION ${JS_COMPILATION_OPTIONS})
        if (DEFINED ARG_DYNAMIC_ABC_OUTPUT_DIR)
            list(APPEND COMPILE_OPTIONS ${COMPILE_OPTIONS} OUTPUT_DIR ${ARG_DYNAMIC_ABC_OUTPUT_DIR})
        endif()
        compile_dynamic_file(${TARGET}_dynamic_modules ${COMPILE_OPTIONS})
    endif()

    set(CHECKER "${PANDA_ROOT}/tests/checked/checker.rb")

    if (PANDA_TARGET_AMD64)
        set(ARCHITECTURE "x64")
    elseif (PANDA_TARGET_ARM64)
        set(ARCHITECTURE "arm64")
    else()
        set(ARCHITECTURE "arm32")
    endif()

    set(JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_checked_test.js)
    get_filename_component(LAUNCHER_CLEAR_NAME ${JS_LAUNCHER} NAME_WLE)

    set(COMPILED_LAUNCHER_NAME ${TARGET}_launcher_abc_name)
    compile_dynamic_file(${TARGET}_js_launcher
        SOURCES ${JS_LAUNCHER}
        OUTPUT_ABC_PATHS ${COMPILED_LAUNCHER_NAME}
        COMPILE_OPTION ${JS_COMPILATION_OPTIONS}
    )

    set(TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}")
    file(MAKE_DIRECTORY "${TEST_DIR}")

    # Make symbolic links to convinient work with requireNapiPreview
    set(SO_FILES_LINK_PATH "${TEST_DIR}/module/")
    set(INTEROP_LIB_SOURCE "${PANDA_BINARY_ROOT}/lib/module/ets_interop_js_napi.so")
    set(INTEROP_HELPER_LIB_SOURCE "${PANDA_BINARY_ROOT}/lib/interop_js/libinterop_test_helper.so")

    add_custom_target(${TARGET}_create_symlinks
        COMMAND mkdir -p ${SO_FILES_LINK_PATH}
                && ln -sf ${INTEROP_LIB_SOURCE} ${INTEROP_HELPER_LIB_SOURCE} -t ${SO_FILES_LINK_PATH}
        DEPENDS ets_interop_js_napi ${INTEROP_HELPER_LIB_SOURCE}
    )

    set(ARK_ETS_INTEROP_JS_PACKAGE_PATH ${PANDA_BINARY_ROOT}/abc/${TARGET_TEST_PACKAGE}.zip)
    set(CUSTOM_PRERUN_ENVIRONMENT
        "LD_LIBRARY_PATH=${PANDA_BINARY_ROOT}/lib/interop_js/:${PANDA_BINARY_ROOT}/lib/"
        "ARK_ETS_INTEROP_JS_TEST_ABC_PATH=${ARK_ETS_INTEROP_JS_PACKAGE_PATH}"
        "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
    )
    if(NOT DEFINED ARG_PACKAGE_NAME AND DEFINED ARG_FILE)
        get_filename_component(ARG_PACKAGE_NAME ${ARG_FILE} NAME_WE)
    elseif(NOT DEFINED ARG_PACKAGE_NAME)
        message(FATAL_ERROR "Please provide PACKAGE_NAME for ${TARGET}")
    endif()
    set(RUN_COMMAND
        "/usr/bin/env"
        "LD_LIBRARY_PATH=${PANDA_BINARY_ROOT}/lib/interop_js/:${PANDA_BINARY_ROOT}/lib/"
        "ARK_ETS_INTEROP_JS_TEST_ABC_PATH=${ARK_ETS_INTEROP_JS_PACKAGE_PATH}"
        "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
        "PACKAGE_NAME=${ARG_PACKAGE_NAME}"
        ${ARK_JS_NAPI_CLI}
        --stub-file=${ARK_JS_STUB_FILE}
        --enable-force-gc=false
        --entry-point=${LAUNCHER_CLEAR_NAME}
        ${${COMPILED_LAUNCHER_NAME}}
    )

    SET(OPTIONS "--run-gc-in-place")
    set(PAOC_OPTIONS ${OPTIONS} "--load-runtimes=ets" "--boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc")
    set(ES2PANDA_OPTIONS --thread=0 --extension=ets --arktsconfig=${ETS_CONFIG} --ast-verifier:errors=${ETS_VERIFICATOR_ERRORS})

    if (PANDA_LLVM_AOT)
        set(WITH_LLVM "--with-llvm")
    endif()

    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        set(RELEASE_OPT "--release")
    endif()

    add_custom_target(${TARGET}
        COMMENT "Running ${TARGET} checked test"
        WORKING_DIRECTORY ${TEST_DIR}
        COMMAND
            ${CHECKER}
            --source ${ARG_FILE}
            --panda \"${RUN_COMMAND}\"
            --paoc $<TARGET_FILE:ark_aot>
            --frontend ${es2panda_bin}
            --run-prefix \"${PANDA_RUN_PREFIX}\"
            --test-file ${ARK_ETS_INTEROP_JS_PACKAGE_PATH}
            --panda-options \"${OPTIONS}\"
            --paoc-options \"${PAOC_OPTIONS}\"
            --frontend-options \"${ES2PANDA_OPTIONS}\"
            --command-token \"//!\"
            --arch ${ARCHITECTURE}
            ${WITH_LLVM}
            ${RELEASE_OPT}
        DEPENDS
            ${TARGET}_js_launcher
            ${TARGET}_create_symlinks
            ${TARGET_TEST_PACKAGE}
            ets_interop_js_napi
            ${ETS_CONFIG}
    )

    if(DEFINED ARG_JS_SOURCES OR ARG_TS_SOURCES)
        add_dependencies(${TARGET} ${TARGET}_dynamic_modules)
    endif()

    add_dependencies(ets_interop_js_checked_tests ${TARGET})
    add_dependencies(ets_checked_tests ${TARGET})
endfunction(panda_ets_interop_js_checked_test)
