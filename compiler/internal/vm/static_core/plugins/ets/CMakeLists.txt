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

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(plugin_ets)

message(STATUS "eTS plugin is found")

include(cmake/import.cmake)

add_custom_target(ets_tests COMMENT "Running ets test suites")
set(DEFAULT_TEST_GROUP ets_tests)
set(STDLIB ${CMAKE_CURRENT_SOURCE_DIR}/stdlib)

function(compile_ets_code ETS_SRC OUTPUT_ABC TARGET)
    set(oneValueArgs ARKTS_CONFIG ETSPATH OPT_LEVEL ETS_STRINGS_CONCAT)
    set(multiValueArgs COMPILER_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT DEFINED ARG_OPT_LEVEL)
        set(ARG_OPT_LEVEL 2)
    endif()

    set(ES2PANDA_ARGUMENTS
        --opt-level=${ARG_OPT_LEVEL}
        --thread=0
        --extension=ets
        --output=${OUTPUT_ABC}
        ${ARG_COMPILER_EXTRA_OPTIONS}
    )
    if (DEFINED ARG_ARKTS_CONFIG)
        list(APPEND ES2PANDA_ARGUMENTS "--arktsconfig=${ARG_ARKTS_CONFIG}")
        list(APPEND ES2PANDA_ARGUMENTS "--stdlib=${PANDA_ROOT}/plugins/ets/stdlib")
    endif()

    if (DEFINED ARG_ETSPATH)
        list(APPEND ES2PANDA_ARGUMENTS "--ets-path=${ARG_ETSPATH}")
    endif()

    if (DEFINED ARG_ETS_STRINGS_CONCAT)
        list(APPEND ES2PANDA_ARGUMENTS "--ets-strings-concat=${ARG_ETS_STRINGS_CONCAT}")
    endif()

    set(COMPILE_DEPENDENCIES)
    set(E2P)

    get_filename_component(OUTPUT_DIR ${OUTPUT_ABC} DIRECTORY)
    file(MAKE_DIRECTORY ${OUTPUT_DIR})

    if ("${ETS_SRC}" STREQUAL "GEN_STD_LIB")
        set(E2P ${es2panda_bin_stdlib})
        list(APPEND ES2PANDA_ARGUMENTS "--gen-stdlib=true")
        list(APPEND ES2PANDA_ARGUMENTS "--arktsconfig=${STDLIB}/stdconfig.json")

        set(STAMP "${OUTPUT_ABC}.stamp")
        file(WRITE ${STAMP} "")
        add_custom_target(${TARGET}.stamp
            BYPRODUCTS ${STAMP}
            COMMAND find ${STDLIB} -newer ${STAMP} -exec touch ${STAMP} ";" || touch ${STAMP}
            VERBATIM
        )
        list(APPEND COMPILE_DEPENDENCIES ${es2panda_target_stdlib} ${TARGET}.stamp ${STAMP})
    else()
        set(E2P ${es2panda_bin})
        list(APPEND ES2PANDA_ARGUMENTS ${ETS_SRC})
        list(APPEND COMPILE_DEPENDENCIES ${es2panda_target} etsstdlib ${ETS_SRC})
    endif()


    add_custom_command(
        OUTPUT ${OUTPUT_ABC}
        COMMENT "Running es2panda for ets file: ${ETS_SRC}"
        COMMAND ${E2P} ${ES2PANDA_ARGUMENTS}
        DEPENDS ${COMPILE_DEPENDENCIES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_custom_target(${TARGET} DEPENDS ${OUTPUT_ABC})

    if(TARGET etsstdlib)
        add_dependencies(es2panda etsstdlib)
        if (NOT TARGET regenerate_and_check_stdlib AND ${CMAKE_GENERATOR} MATCHES "Ninja")
            # Note(ivan-tyulyandin): -P replaces CMAKE_CURRENT_SOURCE_DIR so using ETS_PLUGIN_SOURCE_DIR
            add_custom_target(regenerate_and_check_stdlib
                COMMAND ${CMAKE_COMMAND} -DETS_PLUGIN_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/GenerateStdLib.cmake)
            add_dependencies(etsstdlib regenerate_and_check_stdlib)
        endif()
    endif()

endfunction()

function(compile_ets_sources WORK_DIR ABC_TARGET ABC_FILES ETS_SOURCES)
    set(oneValueArgs ARKTS_CONFIG ETSPATH OPT_LEVEL)
    set(multiValueArgs ARKTS_CONFIGS COMPILER_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_custom_target(${ABC_TARGET})

    # Required for clearing `RESULT` when calling the function multiple times from a single scope
    set(RESULT "")

    list(LENGTH ETS_SOURCES SOURCES_LIST_LENGTH)

    if(DEFINED ARG_ARKTS_CONFIGS)
        list(LENGTH ARG_ARKTS_CONFIGS CONFIG_LIST_LENGTH)
        if(NOT SOURCES_LIST_LENGTH EQUAL CONFIG_LIST_LENGTH)
            error("If set, the number of elements in ARKTS_CONFIGS should be equal to the number of ETS_SOURCES")
        endif()
    endif()

    math(EXPR LAST_INDEX "${SOURCES_LIST_LENGTH} - 1")

    foreach(i RANGE ${LAST_INDEX})
        list(GET ETS_SOURCES ${i} SOURCE_ETS)
        if(DEFINED ARG_ARKTS_CONFIGS)
            list(GET ARG_ARKTS_CONFIGS ${i} ARKTS_CONFIG)
        else()
            set(ARKTS_CONFIG "${ARG_ARKTS_CONFIG}")
        endif()
        get_filename_component(SOURCE_FNAME ${SOURCE_ETS} NAME_WE)
        set(SOURCE_ABC ${WORK_DIR}/${ABC_TARGET}-${SOURCE_FNAME}.abc)
        set(SOURCE_ABC_TARGET ${ABC_TARGET}-${SOURCE_FNAME}-ets-es2panda)
        compile_ets_code(${SOURCE_ETS} ${SOURCE_ABC} ${SOURCE_ABC_TARGET}
                         ARKTS_CONFIG ${ARKTS_CONFIG}
                         ETSPATH ${ARG_ETSPATH}
                         OPT_LEVEL ${ARG_OPT_LEVEL}
                         COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
        )
        add_dependencies(${ABC_TARGET} ${SOURCE_ABC_TARGET})
        set(RESULT ${RESULT} ${SOURCE_ABC})
    endforeach()

    set(${ABC_FILES} ${RESULT} PARENT_SCOPE)
endfunction()

# NB! ADDING THIS PROPERTY IS ALLOWED ONLY IN SPECIAL CASES. DO NOT COPY-PASTE IT.
set_target_properties(ets_tests PROPERTIES first-level-tests-dependency TRUE)
add_dependencies(tests ets_tests)

if(NOT PANDA_ENABLE_THREAD_SANITIZER OR PANDA_CI_TESTING_MODE STREQUAL "Nightly")
    if(TARGET es2panda_tests)
        add_dependencies(ets_tests es2panda_tests)
    endif()
    if(TARGET es2panda_gtests)
        add_dependencies(ets_tests es2panda_gtests)
    endif()
endif()

if(PANDA_WITH_RUNTIME)
    add_subdirectory(runtime)
endif()

if(PANDA_WITH_TOOLCHAIN)
    add_subdirectory(assembler)
    add_subdirectory(abc2program)
    add_subdirectory(disassembler)
    add_subdirectory(isa)
endif()

add_subdirectory(bytecode_optimizer)
add_subdirectory(irtoc_scripts)
add_subdirectory(verification)
add_subdirectory(tools)

# NOTE(nsizov): change this logic when ets compiler moved into ets plugin
if(TARGET es2panda)
    compile_ets_code(GEN_STD_LIB "${CMAKE_CURRENT_BINARY_DIR}/etsstdlib.abc" etsstdlib)

    set_target_properties(etsstdlib PROPERTIES FILE "${CMAKE_CURRENT_BINARY_DIR}/etsstdlib.abc")
    add_dependencies(ark etsstdlib)
    add_subdirectory(sdk)

    # host-tools cannot have libarkruntime as for and verifier as well.
    if(NOT CMAKE_CROSSCOMPILING AND NOT HOST_TOOLS)
        set(verifier_target verifier)
        set(verifier_bin    $<TARGET_FILE:${verifier_target}>)

        add_dependencies(etsstdlib ${verifier_target})

        set (ETS_STD_LIB "${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc")

        set (RUN_VERIFIER_DST_DIR "${PANDA_BINARY_ROOT}/verification/verifier/")
        set (RUN_VERIFIER_SRC_DIR "${CMAKE_SOURCE_DIR}/verification/verifier/")

        configure_file(
            ${RUN_VERIFIER_SRC_DIR}/run_verifier.sh
            ${RUN_VERIFIER_DST_DIR}/run_verifier.sh
            COPYONLY
        )
        add_custom_command(TARGET etsstdlib
            POST_BUILD
            COMMAND bash -c "${RUN_VERIFIER_DST_DIR}/run_verifier.sh '${verifier_bin}' '${ETS_STD_LIB}'" -P
        )

        # NOTE(ermolaevavarvara): Re-enable this checker within spec#818
        # if (PANDA_TARGET_X86 OR PANDA_TARGET_AMD64)
        #     add_custom_target(run-spec-checker
        #         COMMENT "Running checker for specification code snippets"
        #         COMMAND PANDA_ROOT=${PANDA_BINARY_ROOT} python3 ${CMAKE_SOURCE_DIR}/plugins/ets/tools/specification_checker.py --spec-folder ${CMAKE_SOURCE_DIR}/plugins/ets/doc/concurrency
        #         DEPENDS ark)
        #     add_dependencies(ets_tests run-spec-checker)
        # endif()

    endif()
else()
    message(WARNING "es2panda was not found for etsstdlib and etssdk build")
endif()

if(PANDA_WITH_COMPILER)
    add_subdirectory(compiler)
    set (COMPILER_OPTIONS "--compiler-check-final=true")
    add_custom_target(ets-compile-stdlib-default
        COMMENT "Running ark_aot compilation for etsstdlib with default options"
        COMMAND ${CMAKE_SOURCE_DIR}/plugins/ets/compiler/tools/paoc_compile_stdlib.sh --prefix="${PANDA_RUN_PREFIX}" --binary-dir=${CMAKE_BINARY_DIR} -compiler-options="${COMPILER_OPTIONS}" -paoc-output=${CMAKE_CURRENT_BINARY_DIR}/etsstdlib.an
        DEPENDS ark_aot etsstdlib
    )
endif()

if(PANDA_WITH_TESTS)
    add_subdirectory(tests)
endif()

add_plugin_options(${CMAKE_CURRENT_SOURCE_DIR}/ets_plugin_options.yaml)

add_runtime_options(${CMAKE_CURRENT_SOURCE_DIR}/runtime_options.yaml)

add_entrypoints_yaml(${CMAKE_CURRENT_SOURCE_DIR}/runtime/ets_entrypoints.yaml)

# ----- Collecting ets gtests paths for CI stash ---------------------------------
# NB! This must be the last section!

# Write to a file the list of all binaries
get_property(stash_files GLOBAL PROPERTY ets_stash_list)
list(APPEND stash_files "abc-gtests")
list(JOIN stash_files "\n" file_content)
file(WRITE ${PANDA_BINARY_ROOT}/ets_stash_files.txt "${file_content}\n")
