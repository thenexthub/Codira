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

# option(ENABLE_COMPILER_COVERAGE "Enable coverage-calculation for compiler" false)

# CC-OFFNXT(bc-40028) false positive
include(${PANDA_ROOT}/cmake/toolchain/coverage/unit_tests_lcov.cmake)

if(ENABLE_COMPILER_COVERAGE)
    add_custom_target(compiler_coverage DEPENDS cts-assembly tests benchmarks)
    collect_coverage_for_target(
        TARGET_NAME compiler_coverage
        INCLUDE_DIR_PATTERN \"*/compiler/*\"
        EXCLUDE_DIR_PATTERN \"/usr*\" \"*/third_party/*\" \"*/build/*\" \"*/plugins/*\" \"*/runtime/*\" \"*/bytecode_optimizer/*\" \"*/libarkbase/*\"
    )
else()
    message(STATUS "Coverage will not be calculated (may be enabled by -DENABLE_COMPILER_COVERAGE=true ).")
endif(ENABLE_COMPILER_COVERAGE)
