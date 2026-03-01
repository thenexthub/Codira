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

option(ENABLE_BYTECODE_OPTIMIZER_COVERAGE "Enable coverage calculation for the bytecode optimizer" false)

# CC-OFFNXT(bc-40028) false positive
include(${PANDA_ROOT}/cmake/toolchain/coverage/unit_tests_lcov.cmake)

add_custom_target(bytecode_optimizer_coverage DEPENDS bytecodeopt_unit_tests)

add_custom_command(TARGET bytecode_optimizer_coverage POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND bash ${PANDA_ROOT}/bytecode_optimizer/tools/optimizer_coverage.sh --binary-dir=${PANDA_BINARY_ROOT} --root-dir=${PANDA_ROOT}
)

if(ENABLE_BYTECODE_OPTIMIZER_COVERAGE)
    collect_coverage_for_target(
        TARGET_NAME bytecode_optimizer_coverage
        INCLUDE_DIR_PATTERN \"*/bytecode_optimizer/*\"
        EXCLUDE_DIR_PATTERN \"/usr*\" \"*/third_party/*\" \"*/build/*\" \"*/plugins/*\" \"*/runtime/*\" \"*/compiler/*\" \"*/libarkbase/*\"
    )
else()
    message(STATUS "Coverage will not be calculated (may be enabled by -DENABLE_BYTECODE_OPTIMIZER_COVERAGE=true ).")
endif(ENABLE_BYTECODE_OPTIMIZER_COVERAGE)
