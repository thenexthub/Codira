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


# Helped method for creating xx package
#
# Example usage:
#   do_panda_ets_package(package_name
#     ABC_FILE
#       path/to/file0.abc
#     ETS_SOURCE
#       path/to/file0.ets
#       path/to/file1.ets
#     OUTPUT_DIRECTORY
#       path/to/output_director
#     ETS_CONFIG
#       path/to/arktsconfig.json
#     ETS_VERIFICATOR_ERRORS
#       ForLoopCorrectlyInitialized:VariableHasScope:IdentifierHasVariable
#   )
function(do_panda_ets_package TARGET)
    add_custom_target(${TARGET})

    # Parse arguments
    cmake_parse_arguments(
        ARG
        ""
        "OUTPUT_DIRECTORY;ETS_CONFIG;VERIFY_SOURCES"
        "ABC_FILE;ETS_SOURCES;ETS_VERIFICATOR_ERRORS"
        ${ARGN}
    )

    # Check arguments
    if(NOT DEFINED ARG_OUTPUT_DIRECTORY)
        message(FATAL_ERROR "OUTPUT_DIRECTORY is not set")
    endif()

    if (NOT DEFINED ARG_VERIFY_SOURCES)
        set(ARG_VERIFY_SOURCES true)
    endif()

    if(NOT DEFINED ARG_ETS_SOURCES AND NOT DEFINED ARG_ABC_FILE OR
       DEFINED ARG_ETS_SOURCES AND DEFINED ARG_ABC_FILE)
        message(FATAL_ERROR "One and only one of ETS_SOURCES or ABC_FILE should be set")
    endif()

    # Set variables
    set(ES2PANDA_ARGUMENTS
        --opt-level=2
        --thread=0
        --extension=ets
    )
    if(DEFINED ARG_ETS_VERIFICATOR_ERRORS)
        string(REPLACE "," ":" ARG_ETS_VERIFICATOR_ERRORS, ${ARG_ETS_VERIFICATOR_ERRORS})
        list(APPEND ES2PANDA_ARGUMENTS --ast-verifier:errors=${ARG_ETS_VERIFICATOR_ERRORS})
    endif()

    set(VERIFIER_ARGUMENTS
        --boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
        --load-runtimes=ets
    )

    if(DEFINED ARG_ETS_CONFIG)
        list(APPEND ES2PANDA_ARGUMENTS --arktsconfig=${ARG_ETS_CONFIG})
        list(APPEND ES2PANDA_ARGUMENTS --stdlib=${PANDA_ROOT}/plugins/ets/stdlib)
    endif()

    set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TARGET})

    # Convert *.ets -> classes.abc
    set(OUTPUT_ABC ${BUILD_DIR}/src/classes.abc)
    if(DEFINED ARG_ETS_SOURCES)
        list(LENGTH ARG_ETS_SOURCES list_length)

        if (list_length EQUAL 1)
            # Compile one .ets file to OUTPUT_ABC
            add_custom_command(
                OUTPUT ${OUTPUT_ABC}
                COMMENT "${TARGET}: Convert ets files to ${OUTPUT_ABC}"
                COMMAND mkdir -p ${BUILD_DIR}/src
                COMMAND ${es2panda_bin} ${ES2PANDA_ARGUMENTS} --output=${OUTPUT_ABC} ${ARG_ETS_SOURCES}
                DEPENDS ${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc ${es2panda_target} ${ARG_ETS_SOURCES}
            )
        else()
            # Compile several .ets files and link them to OUTPUT_ABC
            set(ABC_FILES)
            foreach(ETS_SOURCE ${ARG_ETS_SOURCES})
                get_filename_component(CLEAR_NAME ${ETS_SOURCE} NAME_WE)
                set(CUR_OUTPUT_ABC ${BUILD_DIR}/src/${CLEAR_NAME}.abc)
                list(APPEND ABC_FILES ${CUR_OUTPUT_ABC})

                add_custom_command(
                    OUTPUT ${CUR_OUTPUT_ABC}
                    COMMENT "${TARGET}: Convert ets files to ${CUR_OUTPUT_ABC}"
                    COMMAND mkdir -p ${BUILD_DIR}/src
                    COMMAND ${es2panda_bin} ${ES2PANDA_ARGUMENTS} --output=${CUR_OUTPUT_ABC} ${ETS_SOURCE}
                    DEPENDS ${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc ${es2panda_target} ${ETS_SOURCE}
                )
            endforeach()

            # Link .abc files into single .abc file
            add_custom_command(
                OUTPUT ${OUTPUT_ABC}
                COMMENT "Linking ABC files: ${ABC_FILES}"
                COMMAND $<TARGET_FILE:ark_link> --output ${OUTPUT_ABC} -- ${ABC_FILES}
                DEPENDS ark_link ${ABC_FILES}
            )
        endif()

        if (ARG_VERIFY_SOURCES)
            set(VERIFY_OUTPUT ${BUILD_DIR}/.verified)
            add_custom_command(
                OUTPUT ${VERIFY_OUTPUT}
                COMMENT "${TARGET}: Verify abc file ${OUTPUT_ABC}"
                COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:verifier> ${VERIFIER_ARGUMENTS} ${OUTPUT_ABC} && touch ${VERIFY_OUTPUT}
                DEPENDS verifier ${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc ${OUTPUT_ABC}
            )
            add_custom_target(${TARGET}_verify DEPENDS ${VERIFY_OUTPUT})
            add_dependencies(${TARGET} ${TARGET}_verify)
        else()
            message(STATUS "do_panda_ets_package: Skip verify: ${TARGET}")
        endif()
    else()
        add_custom_command(
                OUTPUT ${OUTPUT_ABC}
                COMMENT "${TARGET}: Copy abc file to ${OUTPUT_ABC}"
                COMMAND cp -rf ${ARG_ABC_FILE} ${OUTPUT_ABC}
                DEPENDS ${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc ${ARG_ABC_FILE}
            )
    endif()

    # Pack classes.abc to .zip archive
    set(OUTPUT_ZIP ${BUILD_DIR}/out/${TARGET}.zip)
    add_custom_command(
        OUTPUT ${OUTPUT_ZIP}
        COMMENT "Create ${OUTPUT_ZIP}"
        COMMAND rm -rf ${BUILD_DIR}/out
        COMMAND mkdir ${BUILD_DIR}/out
        COMMAND cd ${BUILD_DIR}/src && zip -r -0 ${OUTPUT_ZIP} *
        DEPENDS ${OUTPUT_ABC}
    )

    # Copy .zip to <PANDA_BINARY_ROOT>/<ARG_OUTPUT_DIRECTORY>
    set(RELEASE_ZIP ${PANDA_BINARY_ROOT}/${ARG_OUTPUT_DIRECTORY}/${TARGET}.zip)
    add_custom_command(
        OUTPUT ${RELEASE_ZIP}
        COMMENT "Copy ${OUTPUT_ZIP} to ${RELEASE_ZIP}"
        COMMAND mkdir -p ${PANDA_BINARY_ROOT}/${ARG_OUTPUT_DIRECTORY}
        COMMAND cp ${OUTPUT_ZIP} ${RELEASE_ZIP}
        DEPENDS ${OUTPUT_ZIP}
    )
    add_custom_target(${TARGET}-zip DEPENDS ${RELEASE_ZIP})
    add_dependencies(${TARGET} ${TARGET}-zip)
endfunction(do_panda_ets_package)


# Create ets package
#
# Example usage:
#   panda_ets_package(package_name
#     ABC_FILE
#       path/to/file0.abc
#     ETS_SOURCES
#       path/to/file0.ets
#       path/to/file1.ets
#     ETS_CONFIG
#       path/to/arktsconfig.json
#   )
function(panda_ets_package)
    do_panda_ets_package(
        ${ARGV}
        OUTPUT_DIRECTORY abc
    )
endfunction(panda_ets_package)


# Create ets package for tests
#
# Example usage:
#   panda_ets_package_gtest(package_name
#     ETS_SOURCES
#       path/to/file0.ets
#       path/to/file1.ets
#     ETS_CONFIG
#       path/to/arktsconfig.json
#   )
function(panda_ets_package_gtest)
    do_panda_ets_package(
        ${ARGV}
        OUTPUT_DIRECTORY abc-gtests
    )
endfunction(panda_ets_package_gtest)
