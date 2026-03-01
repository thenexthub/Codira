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

set(RUNTIME_SOURCE_DIR "${PANDA_ROOT}/runtime")

set(SCHEDULER_TESTS_SOURCES
    "${RUNTIME_SOURCE_DIR}/scheduler/tests/worker_thread_test.cpp"
)

panda_add_gtest(
    NAME scheduler_gtest
    SOURCES
        ${SCHEDULER_TESTS_SOURCES}
    LIBRARIES
        arkruntime
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
    PANDA_STD_LIB "${panda_std_libs}"
    ARK_BOOTCLASSPATH "${panda_std_libs}"
    DEPS_TARGETS
        arkstdlib
)
