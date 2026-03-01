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
if (WINDOWS_FLAG MATCHES 1)
    set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
    set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
    set(WARNING_FLAGS "-w")
    set(SECURE_CFLAG_FOR_SHARED_LIBRARY "-static ${SECURE_CFLAG_FOR_SHARED_LIBRARY}")
    set(CMAKE_C_FLAGS "${SECURE_CFLAG_FOR_SHARED_LIBRARY} ${WARNING_FLAGS}")
endif()

string(REPLACE "-fvisibility=hidden" "" CMAKE_C_FLAGS_NO_VISIBILITY "${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_NO_VISIBILITY}")
 
if (WINDOWS_FLAG MATCHES 1)
    # MSVCRT already provides most of secure functions,
    # so only files listed below need to be compiled from boundscheck library,
    # to avoid "multiple definition" error.
    # Refer to offiical manual of boundscheck library for more information.
    list(APPEND SRC_FILE src/memset_s.c src/snprintf_s.c src/vsnprintf_s.c src/secureprintoutput_a.c)
else()
    aux_source_directory(./src SRC_FILE)
endif()

if (MACOS_FLAG MATCHES 1)
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -shared")
    endif ()
endif()

# Build shared library.
add_library(boundscheck SHARED ${SRC_FILE})
