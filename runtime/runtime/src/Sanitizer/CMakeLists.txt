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

set(COMMON_SRC_LIST
        "SanitizerSymbols.cpp"
)

if (ASAN_SUPPORT_FLAG)
    add_library(Sanitizer STATIC ${COMMON_SRC_LIST} "AddressSanitizer/AsanInterface.cpp")
elseif (TSAN_SUPPORT_FLAG)
    add_library(Sanitizer STATIC ${COMMON_SRC_LIST} "ThreadSanitizer/TsanInterface.cpp")
elseif (HWASAN_SUPPORT_FLAG)
    add_library(Sanitizer STATIC ${COMMON_SRC_LIST} "HwAddressSanitizer/HwasanInterface.cpp")
elseif (GWPASAN_SUPPORT_FLAG)
    add_library(Sanitizer STATIC
            "GwpAddressSanitizer/GwpAsanInterface.cpp"
    )
endif ()

target_include_directories(Sanitizer PRIVATE ${CMAKE_SOURCE_DIR}/src)