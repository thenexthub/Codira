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

set(process_ffi_src)

if(NOT WIN32)
    list(APPEND process_ffi_src process_ffi_unix.c)
    if(DARWIN)
        list(APPEND process_ffi_src process_impl_macos.c)
    else()
        list(APPEND process_ffi_src process_impl_linux.c)
    endif()
else()
    list(APPEND process_ffi_src process_ffi_win.c)
endif()

set(libname cangjie-std-processFFI)

add_library(${libname}-objs OBJECT ${process_ffi_src})
target_compile_options(${libname}-objs PRIVATE ${CMAKE_C_COVERAGE_FLAGS})
add_library(${libname} STATIC $<TARGET_OBJECTS:${libname}-objs>)
install_cangjie_library_ffi(${libname})