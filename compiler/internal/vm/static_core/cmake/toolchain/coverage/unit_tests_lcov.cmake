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

# =============================================================================
# Coverage Tools Configuration
# =============================================================================

macro(check_required_program name alias install_help)
    find_program(${alias}
        NAMES ${name}
        PATHS /usr/bin /usr/local/bin
    )
    if(NOT ${alias})
        message(WARNING "ERROR: The ${name} utility is required!\nInstall ways:\n${install_help}")
    endif()
endmacro()

set(INSTALL_HELP_MESSAGE "• script: static_core/scripts/install-deps-ubuntu -i=coverage-tools\n")

# Find coverage tools
check_required_program(lcov "LCOV" ${INSTALL_HELP_MESSAGE})
check_required_program(genhtml "GENHTML" ${INSTALL_HELP_MESSAGE})

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    check_required_program(llvm-cov-gcov "LLVM_COV_GCOV" ${INSTALL_HELP_MESSAGE})
    set(LLVM_COV_GCOV_OPTION --gcov-tool ${LLVM_COV_GCOV})
endif()

# =============================================================================
# Coverage Flags Setup
# =============================================================================

set(COVERAGE_FLAGS "-fprofile-arcs -ftest-coverage")
set(LCOV_COMMON_FLAGS --quiet --branch-coverage --ignore-errors empty,utility,range,inconsistent,source,format,mismatch)
set(LCOV_ZEROCOUNTERS_FLAGS --zerocounters --directory ${PANDA_BINARY_ROOT})

if(ENABLE_UNIT_TESTS_FULL_COVERAGE)
    set(ENABLE_BYTECODE_OPTIMIZER_COVERAGE TRUE)
    set(ENABLE_COMPILER_COVERAGE TRUE)
endif()

if((ENABLE_UNIT_TESTS_FULL_COVERAGE OR
    ENABLE_BYTECODE_OPTIMIZER_COVERAGE OR
    ENABLE_COMPILER_COVERAGE OR
    ENABLE_LIBPANDABASE_COVERAGE OR
    ENABLE_ES2PANDA_COVERAGE) AND
    NOT IS_USED_COMMON_COVERAGE_TOOLCHAIN)
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_FLAGS}")
endif()

# =============================================================================
# Coverage Collection Function
# =============================================================================

function(collect_coverage_for_target)
    set(options)
    set(oneValueArgs TARGET_NAME)
    set(multiValueArgs INCLUDE_DIR_PATTERN EXCLUDE_DIR_PATTERN)
    
    cmake_parse_arguments(ARG
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    set(HTML_REPORT_DIRECTORY "${PANDA_BINARY_ROOT}/coverage_reports/${ARG_TARGET_NAME}")
    set(COVERAGE_INTERMEDIATE_REPORT "${HTML_REPORT_DIRECTORY}/${ARG_TARGET_NAME}.info")
    
    # Prepare include/exclude patterns
    set(TRACEFILE_FILTER_COMMANDS)
    
    if(ARG_INCLUDE_DIR_PATTERN)
        message(STATUS "Including coverage patterns: ${ARG_INCLUDE_DIR_PATTERN}")
        list(APPEND TRACEFILE_FILTER_COMMANDS
            COMMAND ${LCOV} --extract ${COVERAGE_INTERMEDIATE_REPORT}
                ${ARG_INCLUDE_DIR_PATTERN}
                -o ${COVERAGE_INTERMEDIATE_REPORT}
                ${LCOV_COMMON_FLAGS}
                --ignore-errors unused
                2>/dev/null
            )
    endif()
    
    if(ARG_EXCLUDE_DIR_PATTERN)
        message(STATUS "Excluding coverage patterns: ${ARG_EXCLUDE_DIR_PATTERN}")
        list(APPEND TRACEFILE_FILTER_COMMANDS
            COMMAND ${LCOV} --remove ${COVERAGE_INTERMEDIATE_REPORT}
                ${ARG_EXCLUDE_DIR_PATTERN}
                -o ${COVERAGE_INTERMEDIATE_REPORT}
                ${LCOV_COMMON_FLAGS}
                --ignore-errors unused
                2>/dev/null
        )
    endif()

    add_custom_command(TARGET ${ARG_TARGET_NAME} POST_BUILD
        WORKING_DIRECTORY ${PANDA_BINARY_ROOT}
        COMMAND mkdir -p ${HTML_REPORT_DIRECTORY}
        COMMAND ${LCOV} ${LLVM_COV_GCOV_OPTION}
            ${LCOV_COMMON_FLAGS}
            --capture
            --base-directory ${PANDA_BINARY_ROOT}
            --all
            --directory ${CMAKE_CURRENT_BINARY_DIR}
            --output-file ${COVERAGE_INTERMEDIATE_REPORT}
            2>/dev/null
        ${TRACEFILE_FILTER_COMMANDS}
        COMMAND ${GENHTML}
            ${LCOV_COMMON_FLAGS}
            --output-directory ${HTML_REPORT_DIRECTORY}
            ${COVERAGE_INTERMEDIATE_REPORT}
            --ignore-errors category
            --filter missing
            2>/dev/null
        COMMAND ${LCOV} ${LLVM_COV_GCOV_OPTION} ${LCOV_ZEROCOUNTERS_FLAGS}
        COMMAND echo "Coverage report: ${HTML_REPORT_DIRECTORY}/index.html"
    )
endfunction()
