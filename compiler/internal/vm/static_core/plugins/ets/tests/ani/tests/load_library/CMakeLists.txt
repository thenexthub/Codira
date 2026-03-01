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

set(PANDA_STD_LIB "${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc")

set(GEN_ETSNAPI_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib")
file(MAKE_DIRECTORY ${GEN_ETSNAPI_LIB_DIR})

panda_add_library(nativelib SHARED
    lib.cpp
)

set_target_properties(nativelib PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${GEN_ETSNAPI_LIB_DIR})
panda_add_sanitizers(TARGET nativelib SANITIZERS ${PANDA_SANITIZERS_LIST})
panda_target_include_directories(nativelib PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani)

if (PANDA_TARGET_ARM32)
    return()
endif()

panda_ets_add_gtest(
    NO_CORES
    NAME etsnative_library_path_test
    SOURCES etsnative_library_path_test.cpp
    LIBRARIES arkruntime
    SANITIZERS ${PANDA_SANITIZERS_LIST}
    CUSTOM_PRERUN_ENVIRONMENT
            "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
            "ARK_ETS_LIBRARY_PATH=${GEN_ETSNAPI_LIB_DIR}"
    DEPS_TARGETS etsstdlib nativelib
)
