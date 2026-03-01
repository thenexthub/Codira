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

# There seems to be a bug in either clang-tidy or CMake:
# When clang/gcc is used for cross-compilation, it is ran on host and use defines and options for host
# For example for arm32 cross-compilation Clang-Tidy:
#   - don't know about -march=armv7-a
#   - believes that size of pointer is 64 instead of 32 for aarch32
# TODO: Retry once we upgrade the checker.

# Split huge static_core and ets2panda folders on several subsets
set(CTIDY_CORE_PART1 "tools|tests|compiler|runtime")
set(CTIDY_CORE_PART2 "plugins/ets/tests/ani")
set(CTIDY_CORE_PART3 "?!(${CTIDY_CORE_PART1}|${CTIDY_CORE_PART2})")
set(CTIDY_ETS_PART1  "test")
set(CTIDY_ETS_PART2  "?!${CTIDY_ETS_PART1}")

add_custom_target(clang-tidy-changed-check
  COMMAND ${PANDA_ROOT}/scripts/clang-tidy/clang_tidy_check.py --changed-only ${PANDA_ROOT} ${PANDA_BINARY_ROOT}
  USES_TERMINAL
  DEPENDS panda_gen_files
)

add_custom_target(clang-tidy-static-core-tools-check
  COMMAND ${PANDA_ROOT}/scripts/clang-tidy/clang_tidy_check.py --filename-filter "'/runtime_core/static_core/(${CTIDY_CORE_PART1})'" ${PANDA_ROOT} ${PANDA_BINARY_ROOT}
  USES_TERMINAL
  DEPENDS panda_gen_files
)

add_custom_target(clang-tidy-static-core-libs-check
  COMMAND ${PANDA_ROOT}/scripts/clang-tidy/clang_tidy_check.py --filename-filter "'/runtime_core/static_core/(${CTIDY_CORE_PART2})'" ${PANDA_ROOT} ${PANDA_BINARY_ROOT}
  USES_TERMINAL
  DEPENDS panda_gen_files
)

add_custom_target(clang-tidy-static-core-main-check
  COMMAND ${PANDA_ROOT}/scripts/clang-tidy/clang_tidy_check.py --filename-filter "'/runtime_core/static_core/(${CTIDY_CORE_PART3})'" ${PANDA_ROOT} ${PANDA_BINARY_ROOT}
  USES_TERMINAL
  DEPENDS panda_gen_files
)

add_custom_target(clang-tidy-ets2panda-main-check
  COMMAND ${PANDA_ROOT}/scripts/clang-tidy/clang_tidy_check.py --filename-filter "'/ets2panda/(${CTIDY_ETS_PART2})'" ${PANDA_ROOT} ${PANDA_BINARY_ROOT}
  USES_TERMINAL
  DEPENDS panda_gen_files
)

add_custom_target(clang-tidy-ets2panda-test-check
  COMMAND ${PANDA_ROOT}/scripts/clang-tidy/clang_tidy_check.py --filename-filter /ets2panda/${CTIDY_ETS_PART1} ${PANDA_ROOT} ${PANDA_BINARY_ROOT}
  USES_TERMINAL
  DEPENDS panda_gen_files
)

add_custom_target(clang-tidy-check
  COMMAND ${PANDA_ROOT}/scripts/clang-tidy/clang_tidy_check.py ${PANDA_ROOT} ${PANDA_BINARY_ROOT}
  USES_TERMINAL
  DEPENDS panda_gen_files
)

add_custom_target(clang-tidy-check-full
  COMMAND ${PANDA_ROOT}/scripts/clang-tidy/clang_tidy_check.py --full ${PANDA_ROOT} ${PANDA_BINARY_ROOT}
  USES_TERMINAL
  DEPENDS panda_gen_files
)

add_custom_target(cmake-checker
  COMMAND ${PANDA_ROOT}/scripts/cmake-checker/cmake_checker.py ${PANDA_ROOT}
  USES_TERMINAL
)

add_custom_target(test-cmake-checker
  COMMAND ${PANDA_ROOT}/scripts/cmake-checker/cmake_checker.py ${PANDA_ROOT} TEST
  USES_TERMINAL
)
