# Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

add_custom_target(declgen_ets2ts_test)
add_custom_target(declgen_ets2ts_test_diff)
add_dependencies(ets_interop_js_tests declgen_ets2ts_test_diff declgen_ets2ts_test)

# Create create_declgen_ets2ts_test test
#
# Declgen test consists of files:
#   ${ROOT_DIR}/*.ets
#   ${ROOT_DIR}/*.ts
#   ${ROOT_DIR}/*.js
#   ${ROOT_DIR}/*.expected
# This function:
#   1. Compiles 'ets' files with es2panda into 'abc' and decl 'ts':
#      ${ROOT_DIR}/*.ets -> ${BINARY_ROOT_DIR}/*.abc ${BINARY_ROOT_DIR}/*.ts
#   2. Copies ${ROOT_DIR}/*.ts into ${BINARY_ROOT_DIR}/*.ts
#   3. Compiles 'ts' project with tsc:
#      ${BINARY_ROOT_DIR}/*.ts -> ${BINARY_ROOT_DIR}/*.js
#   4. Links all 'abc' files into single file:
#      ${BINARY_ROOT_DIR}/*.abc -> ${BINARY_ROOT_DIR}/lib_linked.abc
#   5. Creates panda_ets_interop_js_test from ${BINARY_ROOT_DIR}/*.js and ${BINARY_ROOT_DIR}/lib_linked.abc
#   6. Tests difference between expected and actual declaration files:
#      ${BINARY_ROOT_DIR}/*.ts <-> ${ROOT_DIR}/*.expected
#
# Example usage:
#   create_declgen_ets2ts_test(common
#       ROOT_DIR
#           ${CMAKE_CURRENT_SOURCE_DIR}
#       BINARY_ROOT_DIR
#           ${CMAKE_CURRENT_BINARY_DIR}
#       TS_MAIN
#           ${CMAKE_CURRENT_SOURCE_DIR}/main.ts
#       TS_SOURCES
#           ${CMAKE_CURRENT_SOURCE_DIR}/test.ts
#       JS_SOURCES
#           ${CMAKE_CURRENT_SOURCE_DIR}/test_dyn.js
#       ETS_SOURCES
#           ${CMAKE_CURRENT_SOURCE_DIR}/lib1.ets
#           ${CMAKE_CURRENT_SOURCE_DIR}/lib2.ets
#       DECLS_EXPECTED
#           ${CMAKE_CURRENT_SOURCE_DIR}/lib1.expected
#           ${CMAKE_CURRENT_SOURCE_DIR}/lib2.expected
#   )
function(create_declgen_ets2ts_test TARGET)
    cmake_parse_arguments(
        ARG
        ""
        "ROOT_DIR;BINARY_ROOT_DIR;TS_MAIN;ETS_VM_LAUNCHER;"
        "TS_SOURCES;ETS_SOURCES;JS_SOURCES;DECLS_EXPECTED"
        ${ARGN}
    )

    set(TARGET "declgen_ets2ts_test_${TARGET}")

    set(ALL_DYNAMIC_SOURCES)
    if (DEFINED ARG_TS_SOURCES)
        list(APPEND ALL_DYNAMIC_SOURCES ${ARG_TS_SOURCES})
    endif()
    if (DEFINED ARG_JS_SOURCES)
        list(APPEND ALL_DYNAMIC_SOURCES ${ARG_JS_SOURCES})
    endif()
    if (DEFINED ARG_ETS_VM_LAUNCHER)
        list(APPEND ALL_DYNAMIC_SOURCES ${ARG_ETS_VM_LAUNCHER})
    endif()

    # Replaces path prefix from ARG_ROOT_DIR to ARG_BINARY_ROOT_DIR and extension
    function(rebase_path SRC DST)
        if(ARGC GREATER 2 AND ARGV2 STREQUAL "ts")
            file(MAKE_DIRECTORY "${ARG_BINARY_ROOT_DIR}/glue_file")
            string(REGEX REPLACE "^${ARG_ROOT_DIR}" "${ARG_BINARY_ROOT_DIR}/glue_file" RES ${SRC})
        else()
            string(REGEX REPLACE "^${ARG_ROOT_DIR}" "${ARG_BINARY_ROOT_DIR}" RES ${SRC})
        endif()
        if(ARGC GREATER 2)
            set(NEW_EXT ${ARGV2})
            string(REGEX REPLACE "\.[a-zA-Z0-9]+$" ".${NEW_EXT}" RES ${RES})
        endif()
        set(${DST} ${RES} PARENT_SCOPE)
    endfunction()

    set(JS_TO_ABC_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/compiled_js)

    # Create arktsconfig.json with "dependencies" for js modules
    set(ARKTSCONFIG_CONTENT
        "    }\n"
        "  }\n"
        "}\n"
    )
    set(COMMA "")
    foreach(JS_SOURCE ${ARG_JS_SOURCES})
        get_filename_component(JS_SRC_WE ${JS_SOURCE} NAME_WE)
        set(ARKTSCONFIG_CONTENT
            "      \"${JS_SRC_WE}\": {\"language\": \"js\", \"ohmUrl\":\"${JS_TO_ABC_OUTPUT_DIR}/${JS_SRC_WE}\"}${COMMA}\n"
            "${ARKTSCONFIG_CONTENT}"
        )
        if(COMMA STREQUAL "")
            set(COMMA ",")
        endif()
    endforeach()
    set(ARKTSCONFIG_CONTENT
        "{\n"
        "  \"compilerOptions\": {\n"
        "    \"baseUrl\": \"${PANDA_ROOT}\",\n"
        "    \"paths\": {\n"
        "      \"std\": [\"${PANDA_ROOT}/plugins/ets/stdlib/std\"],\n"
        "      \"escompat\": [\"${PANDA_ROOT}/plugins/ets/stdlib/escompat\"]\n"
        "    },\n"
        "    \"dependencies\": {\n"
        "${ARKTSCONFIG_CONTENT}"
    )
    set(ARKTSCONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
    file(WRITE ${ARKTSCONFIG} ${ARKTSCONFIG_CONTENT})

    # NOTE (kkonsw): temporary disabling all other checks
    set(ETS_VERIFICATOR_ERRORS "NodeHasParent:EveryChildHasValidParent:VariableHasScope:NodeHasType:IdentifierHasVariable:ReferenceTypeAnnotationIsNull:ArithmeticOperationValid:SequenceExpressionHasLastType:ForLoopCorrectlyInitialized:VariableHasEnclosingScope:ModifierAccessValid")
    # NOTE(dkofanov): `ImportExportAccessValid` need to be fixed
    # set(ETS_VERIFICATOR_ERRORS "${ETS_VERIFICATOR_ERRORS}:ImportExportAccessValid")

    # Compile source .ets files into .abc and declaration .ts files
    set(ABC_FILES "")
    set(DECL_FILES "")
    set(GLUE_FILES "")
    set(ABC_TARGETS "")
    set(ETS_MODULE_KEY "")
    foreach(ETS_SOURCE ${ARG_ETS_SOURCES})
        rebase_path(${ETS_SOURCE} ABC_PATH "abc")
        list(APPEND ABC_FILES ${ABC_PATH})
        rebase_path(${ETS_SOURCE} DECL_PATH "d.ts")
        list(APPEND DECL_FILES ${DECL_PATH})
        rebase_path(${ETS_SOURCE} GLUE_PATH "ts")
        list(APPEND GLUE_FILES ${GLUE_PATH})

        add_custom_command(
            OUTPUT ${ABC_PATH} ${DECL_PATH} ${GLUE_PATH}
            COMMENT "Compiling: ${ETS_SOURCE}"
            COMMAND $<TARGET_FILE:es2panda> ${ETS_MODULE_KEY} --arktsconfig ${ARKTSCONFIG} --output ${ABC_PATH} --extension=ets --ast-verifier:errors=${ETS_VERIFICATOR_ERRORS} ${ETS_SOURCE}
            COMMAND $<TARGET_FILE:declgen_ets2ts> ${ETS_MODULE_KEY} --arktsconfig ${ARKTSCONFIG} --output-dets=${DECL_PATH} --output-ets=${GLUE_PATH} ${ETS_SOURCE}
            DEPENDS es2panda declgen_ets2ts ${ETS_SOURCE} ${ARKTSCONFIG}
        )

        get_filename_component(ABC_NAME ${ABC_PATH}  NAME)
        set(ABC_TARGET ${TARGET}_es2panda_${ABC_NAME})
        add_custom_target(${ABC_TARGET}
            DEPENDS ${ABC_PATH} ${DECL_PATH}
        )
        list(APPEND ABC_TARGETS ${ABC_TARGET})
        if(ETS_MODULE_KEY STREQUAL "")
            set(ETS_MODULE_KEY "--ets-module")
        endif()
    endforeach()

    set(COPIED_TS_SOURCES "")
    set(JS_FILES "")
    # Copy TS files into build folder and
    # Compute destination JS file paths for these TS files
    foreach(TS_SOURCE ${ARG_TS_SOURCES})
        rebase_path(${TS_SOURCE} COPIED_TS_SOURCE)
        configure_file(${TS_SOURCE} ${COPIED_TS_SOURCE} COPYONLY)
        list(APPEND COPIED_TS_SOURCES ${COPIED_TS_SOURCE})
        rebase_path(${TS_SOURCE} JS_SOURCE "js")
        list(APPEND JS_FILES ${JS_SOURCE})
    endforeach()

    # Compute destination JS file paths for generated TS declarations
    foreach(TS_SOURCE ${GLUE_FILES})
        rebase_path(${TS_SOURCE} JS_SOURCE "js")
        list(APPEND JS_FILES ${JS_SOURCE})
    endforeach()

    # Compile source .ts and declaratin .ts files into .js
    add_custom_command(
        OUTPUT ${JS_FILES}
        COMMENT "Compile ts files: ${COPIED_TS_SOURCES} ${DECL_FILES} ${GLUE_FILES}"
        COMMAND ${PANDA_THIRD_PARTY_SOURCES_DIR}/typescript/bin/tsc --target es5 --strict --lib es2020,dom --outDir ${CMAKE_CURRENT_BINARY_DIR} ${COPIED_TS_SOURCES} ${DECL_FILES}
        COMMAND ${PANDA_THIRD_PARTY_SOURCES_DIR}/typescript/bin/tsc --target es5 --strict --outDir "${CMAKE_CURRENT_BINARY_DIR}" ${GLUE_FILES}
        DEPENDS es2panda ark_link ${ABC_TARGETS} ${ARKTSCONFIG} ${COPIED_TS_SOURCES} ${DECL_FILES}
    )
    add_custom_target(${TARGET}_tsc
        DEPENDS ${JS_FILES}
    )

    # Link .abc files into single .abc file
    set(LIB_LINKED_ABC ${CMAKE_CURRENT_BINARY_DIR}/lib_linked.abc)
    add_custom_command(
        OUTPUT ${LIB_LINKED_ABC}
        COMMENT "Linking ABC files: ${ABC_FILES}"
        COMMAND $<TARGET_FILE:ark_link> --output ${LIB_LINKED_ABC} -- ${ABC_FILES}
        DEPENDS ark_link ${ABC_TARGETS} ${ABC_FILES}
    )
    add_custom_target(${TARGET}_link
        DEPENDS ${LIB_LINKED_ABC}
    )

    list(APPEND ALL_DYNAMIC_SOURCES ${DECL_FILES})
    list(APPEND ALL_DYNAMIC_SOURCES ${GLUE_FILES})

    # Execute compiled interop project
    # PACKAGE_NAME is not necessary for declgen_ets2ts_test,
    # but it is necessary for panda_ets_interop_js_test;
    panda_ets_interop_js_test(${TARGET}
        ABC_FILE ${LIB_LINKED_ABC}
        JS_LAUNCHER ${ARG_TS_MAIN}
        JS_SOURCES ${ALL_DYNAMIC_SOURCES}
        DYNAMIC_ABC_OUTPUT_DIR ${JS_TO_ABC_OUTPUT_DIR}
        PACKAGE_NAME "main"
    )
    add_dependencies(${TARGET} ${TARGET}_tsc ${TARGET}_link)
    add_dependencies(declgen_ets2ts_test ${TARGET})

    # Test difference between actual and expected declarations
    list(LENGTH DECL_FILES DECL_LEN)
    list(LENGTH ARG_DECLS_EXPECTED DECL_EXPECTED_LEN)
    if(NOT DECL_LEN EQUAL DECL_EXPECTED_LEN)
        message(FATAL_ERROR "create_declgen_ets2ts_test: number of source ets files is not equal to number of expected declarations")
    endif()
    math(EXPR DECL_LEN "${DECL_LEN} - 1")
    foreach(IDX RANGE ${DECL_LEN})
        list(GET ARG_DECLS_EXPECTED ${IDX} DECL_EXPECTED)
        list(GET DECL_FILES ${IDX} DECL_ACTUAL)
        string(SHA1 DECL_ACTUAL_HASH "${DECL_ACTUAL}")
        string(SHA1 DECL_EXPECTED_HASH "${DECL_EXPECTED}")
        set(ACTUAL_CLEANED "${CMAKE_BINARY_DIR}/declaration_actual_${DECL_ACTUAL_HASH}.ts")
        set(EXPECTED_CLEANED "${CMAKE_BINARY_DIR}/declaration_expected_${DECL_EXPECTED_HASH}.ts")

        get_filename_component(DECL_NAME ${DECL_EXPECTED} NAME)
        set(DIFF_TARGET ${TARGET}_diff_${DECL_NAME})

        # Remove multi-line comments
        add_custom_command(
            OUTPUT ${ACTUAL_CLEANED}
            COMMAND sed '/^\\/\\*/,/\\*\\//d' ${DECL_ACTUAL} > ${ACTUAL_CLEANED}
            DEPENDS ${DECL_ACTUAL}
        )

        add_custom_command(
            OUTPUT ${EXPECTED_CLEANED}
            COMMAND sed '/^\\/\\*/,/\\*\\//d' ${DECL_EXPECTED} > ${EXPECTED_CLEANED}
            DEPENDS ${DECL_EXPECTED}
        )

        # Compare the modified files and remove them
        add_custom_target(${DIFF_TARGET}
            COMMENT "Test declgen ets2ts diff between ${DECL_ACTUAL} and ${DECL_EXPECTED} (ignoring comments)"
            COMMAND ${CMAKE_COMMAND} -E compare_files ${ACTUAL_CLEANED} ${EXPECTED_CLEANED}
            COMMAND ${CMAKE_COMMAND} -E remove ${ACTUAL_CLEANED} ${EXPECTED_CLEANED}
            DEPENDS ${ACTUAL_CLEANED} ${EXPECTED_CLEANED}
        )
        add_dependencies(declgen_ets2ts_test_diff ${DIFF_TARGET})
    endforeach()
endfunction()


add_subdirectory(async)
add_subdirectory(basic_var)
add_subdirectory(classes)
add_subdirectory(enums)
add_subdirectory(fields)
add_subdirectory(functions)
add_subdirectory(generics)
add_subdirectory(getters_setters)
#add_subdirectory(imports)
add_subdirectory(interface)
add_subdirectory(methods)
add_subdirectory(unions)
add_subdirectory(depend_export)
add_subdirectory(re_export)
add_subdirectory(export_error)
#add_subdirectory(export_default)
add_subdirectory(jsdoc)

# interop not supported
# eg: E/ets_interop_js: InteropCtx::Fatal: ets_proxy requested for Lstd/containers/WeakMap; must add or forbid
# add_subdirectory(esvalue)
# add_subdirectory(standard_lib)
# add_subdirectory(properties)

# error: [TID 006c83] E/ets_interop_js: InteropCtx::Fatal: Class lib.A has no constructor
# a bug exists in interop. interop is being fixed
# please see issue: #23096
# add_subdirectory(namespace)

# NOTE(aakmaev): need proper comparing. Issue #17097
# add_subdirectory(global_vars)
