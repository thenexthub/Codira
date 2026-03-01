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

set(SOURCES
    init.cpp
    test_util.cpp
    test_extractor.cpp)

# Test library can't be built with --no-undefined option
set(UNSUPPORTED_LINKER_FLAGS "-Wl,--no-undefined")
if (CMAKE_SHARED_LINKER_FLAGS)
    string(REPLACE ${UNSUPPORTED_LINKER_FLAGS} "" CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS})
endif()

# Common testing framework for tooling tests
panda_add_library(debugtest SHARED ${SOURCES})
panda_target_link_libraries(debugtest arkfile arkruntime)

panda_target_include_directories(debugtest PUBLIC
    ${PANDA_ROOT}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

panda_add_sanitizers(TARGET debugtest SANITIZERS ${PANDA_SANITIZERS_LIST})

# Sampling profiler tests
# Currently profiler is not supported for arm64
# TODO(m.strizhak): support for device
if (NOT PANDA_TARGET_ARM64)
    add_subdirectory(sampler)
endif()