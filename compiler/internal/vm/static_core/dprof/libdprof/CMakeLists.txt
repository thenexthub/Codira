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

project(libdprof)

set(SOURCES
    dprof/ipc/ipc_message.cpp
    dprof/profiling_data.cpp
)

set(UNIX_SOURCES
    dprof/ipc/ipc_unix_socket.cpp
)

if(PANDA_TARGET_UNIX)
    set(SOURCES ${SOURCES} ${UNIX_SOURCES})
else ()
    message(FATAL_ERROR "Platform ${CMAKE_SYSTEM_NAME} is not supported")
endif ()

panda_add_library(dprof STATIC ${SOURCES})
set_property(TARGET dprof PROPERTY POSITION_INDEPENDENT_CODE ON)

panda_target_link_libraries(dprof arkbase)
panda_target_include_directories(dprof INTERFACE ".")

# Add checkers and sanitizers
panda_add_sanitizers(TARGET dprof SANITIZERS ${PANDA_SANITIZERS_LIST})
