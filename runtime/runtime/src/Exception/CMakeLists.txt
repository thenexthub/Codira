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

if ((CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64") AND (MACOS_FLAG MATCHES 1))
    set(SRC_LIST
        "Exception.cpp"
        "EhTable.cpp"
        "ExceptionCApi.cpp"
        "${CMAKE_SOURCE_DIR}/src/arch/x86_64_macos/RestoreContextForEH.S"
    )
elseif ((CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64") AND (WINDOWS_FLAG MATCHES 0) AND (IOS_FLAG MATCHES 0) AND (IOS_SIMULATOR_FLAG MATCHES 0))
    set(SRC_LIST
        "Exception.cpp"
        "EhTable.cpp"
        "ExceptionCApi.cpp"
        "${CMAKE_SOURCE_DIR}/src/arch/x86_64_linux/RestoreContextForEH.S"
    )
elseif ((CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" AND IOS_FLAG MATCHES 1) OR IOS_SIMULATOR_FLAG MATCHES 2)
    set(SRC_LIST
        "Exception.cpp"
        "EhTable.cpp"
        "ExceptionCApi.cpp"
        "${CMAKE_SOURCE_DIR}/src/arch/x86_64_ios/RestoreContextForEH.S"
    )
elseif ((CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64") AND (MACOS_FLAG MATCHES 1))
    set(SRC_LIST
        "Exception.cpp"
        "EhTable.cpp"
        "ExceptionCApi.cpp"
        "${CMAKE_SOURCE_DIR}/src/arch/aarch64_macos/RestoreContextForEH.S"
    )
elseif ((IOS_FLAG MATCHES 1 OR IOS_SIMULATOR_FLAG MATCHES 1) AND (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64"))
    set(SRC_LIST
        "Exception.cpp"
        "EhTable.cpp"
        "ExceptionCApi.cpp"
        "${CMAKE_SOURCE_DIR}/src/arch/aarch64_ios/RestoreContextForEH.S"
    )
elseif ((CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64") AND (WINDOWS_FLAG MATCHES 0))
    set(SRC_LIST
        "Exception.cpp"
        "EhTable.cpp"
        "ExceptionCApi.cpp"
        "${CMAKE_SOURCE_DIR}/src/arch/aarch64_linux/RestoreContextForEH.S"
    )
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    set(SRC_LIST
        "Exception.cpp"
        "EhTable.cpp"
        "ExceptionCApi.cpp"
        "${CMAKE_SOURCE_DIR}/src/arch/arm_linux/RestoreContextForEH.S"
    )
elseif (WINDOWS_FLAG MATCHES 1)
    set(SRC_LIST
        "Exception.cpp"
        "EhTable.cpp"
        "ExceptionCApi.cpp"
        "${CMAKE_SOURCE_DIR}/src/arch/x86_64_windows/RestoreContextForEH.S"
    )
endif()
add_library(Exception STATIC ${SRC_LIST})
