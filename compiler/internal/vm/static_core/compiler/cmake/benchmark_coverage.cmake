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

add_custom_target(benchmark_coverage DEPENDS benchmark_coverage_arm64 benchmark_coverage_x86_64 benchmark_coverage_arm)

# arm64 targets:
add_custom_target(benchmark_coverage_arm64 DEPENDS benchmark_coverage_arm64_aot benchmark_coverage_arm64_jit benchmark_coverage_arm64_osr)
add_custom_target(benchmark_coverage_arm64_aot DEPENDS benchmark_coverage_arm64_aot pandasm paoc)
add_custom_target(benchmark_coverage_arm64_jit DEPENDS benchmark_coverage_arm64_jit pandasm paoc)
add_custom_target(benchmark_coverage_arm64_osr DEPENDS benchmark_coverage_arm64_osr pandasm paoc)

# x86_64 targets:
add_custom_target(benchmark_coverage_x86_64 DEPENDS benchmark_coverage_x86_64_aot benchmark_coverage_x86_64_jit)
add_custom_target(benchmark_coverage_x86_64_aot DEPENDS benchmark_coverage_x86_64_aot pandasm paoc)
add_custom_target(benchmark_coverage_x86_64_jit DEPENDS benchmark_coverage_x86_64_jit pandasm paoc)

# arm32 targets:
add_custom_target(benchmark_coverage_arm DEPENDS benchmark_coverage_arm_jit)
add_custom_target(benchmark_coverage_arm_jit DEPENDS benchmark_coverage_arm_jit pandasm paoc)

# Build rules:
add_custom_command(TARGET benchmark_coverage_arm64_aot POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND bash ${PANDA_ROOT}/compiler/tools/benchmark_coverage.sh --binary-dir=${PANDA_BINARY_ROOT} --root-dir=${PANDA_ROOT} --target-arch=arm64 --paoc-mode=aot 2>/dev/null
)

add_custom_command(TARGET benchmark_coverage_arm64_jit POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND bash ${PANDA_ROOT}/compiler/tools/benchmark_coverage.sh --binary-dir=${PANDA_BINARY_ROOT} --root-dir=${PANDA_ROOT} --target-arch=arm64 --paoc-mode=jit 2>/dev/null
)

add_custom_command(TARGET benchmark_coverage_arm64_osr POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND bash ${PANDA_ROOT}/compiler/tools/benchmark_coverage.sh --binary-dir=${PANDA_BINARY_ROOT} --root-dir=${PANDA_ROOT} --target-arch=arm64 --paoc-mode=osr 2>/dev/null
)

add_custom_command(TARGET benchmark_coverage_x86_64_jit POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND bash ${PANDA_ROOT}/compiler/tools/benchmark_coverage.sh --binary-dir=${PANDA_BINARY_ROOT} --root-dir=${PANDA_ROOT} --target-arch=x86_64 --paoc-mode=jit 2>/dev/null
)

add_custom_command(TARGET benchmark_coverage_x86_64_aot POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND bash ${PANDA_ROOT}/compiler/tools/benchmark_coverage.sh --binary-dir=${PANDA_BINARY_ROOT} --root-dir=${PANDA_ROOT} --target-arch=x86_64 --paoc-mode=aot 2>/dev/null
)

# TODO (asidorov): enable after AOT support on arm32
#add_custom_command(TARGET benchmark_coverage_arm_jit POST_BUILD
#    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#    COMMAND bash ${PANDA_ROOT}/compiler/tools/benchmark_coverage.sh --binary-dir=${PANDA_BINARY_ROOT} --root-dir=${PANDA_ROOT} --target-arch=arm --paoc-mode=jit
#)
