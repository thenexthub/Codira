# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

add_subdirectory(dynamicLoader)
add_subdirectory(compress)
add_subdirectory(serialization)
add_subdirectory(encoding)
add_subdirectory(fuzz)
add_subdirectory(log)
add_subdirectory(logger)
add_subdirectory(crypto)
add_subdirectory(net)
add_subdirectory(unittest)
add_subdirectory(actors)
add_subdirectory(string_intern)
if(CODIRA_CODEGEN_CODENATIVE_BACKEND AND "$ENV{NO_ASPECTCODE}" STREQUAL "")
    if(CMAKE_CROSSCOMPILING)
        if ("$ENV{CODIRA_LSP_PATH}" STREQUAL "")
            # Nothing to do
        else()
            add_subdirectory(aspectCODE)
        endif()
    else()
        add_subdirectory(aspectCODE)
    endif()
endif()

if(NOT CODIRA_BUILD_WITHOUT_EFFECT_HANDLERS)
    add_subdirectory(effect)
endif()
