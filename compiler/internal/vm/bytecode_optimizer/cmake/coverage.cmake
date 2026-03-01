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

add_custom_target(bytecode_optimizer_coverage DEPENDS bytecodeopt_unit_tests)

add_custom_command(TARGET bytecode_optimizer_coverage POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND bash ${PANDA_ROOT}/bytecode_optimizer/tools/optimizer_coverage.sh --binary-dir=${PANDA_BINARY_ROOT} --root-dir=${PANDA_ROOT}
)

option(ENABLE_BYTECODE_OPTIMIZER_COVERAGE "Enable coverage calculation for the bytecode optimizer" false)

find_program(
    LCOV
    NAMES "lcov"
    DOC "Path to lcov executable")
if(NOT LCOV)
    set(ENABLE_BYTECODE_OPTIMIZER_COVERAGE false)
endif()

find_program(
    GENHTML
    NAMES "genhtml"
    DOC "Path to genhtml executable")
if(NOT GENHTML)
    set(ENABLE_BYTECODE_OPTIMIZER_COVERAGE false)
endif()

if(ENABLE_BYTECODE_OPTIMIZER_COVERAGE)
    # Set coverage options
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    add_custom_target(bo_coverage DEPENDS bytecodeopt_unit_tests) # Execute tests
    set(ADD_COV_FLAGS --quiet --rc lcov_branch_coverage=1)
    add_custom_command(TARGET bo_coverage POST_BUILD
        WORKING_DIRECTORY ${PANDA_BINARY_ROOT}
        # Update current coverage info
        COMMAND lcov --no-external -b ${PANDA_ROOT}/bytecode_optimizer -d  ${CMAKE_CURRENT_BINARY_DIR} -c -o bo_coverage.info ${ADD_COV_FLAGS}
        # Generating an html report
        COMMAND genhtml -o bo_coverage_report bo_coverage.info --ignore-errors source ${ADD_COV_FLAGS}
        COMMAND echo "Coverage report: ${PANDA_BINARY_ROOT}/bo_coverage_report"
        # Delete temporary files to collect statistics
        COMMAND rm bo_coverage.info
        COMMAND find ${PANDA_BINARY_ROOT}/* -iname "*.gcda" -delete
    )
else()
    message(STATUS "Coverage will not be calculated (may be enabled by -DENABLE_BYTECODE_OPTIMIZER_COVERAGE=true ).")
endif(ENABLE_BYTECODE_OPTIMIZER_COVERAGE)
