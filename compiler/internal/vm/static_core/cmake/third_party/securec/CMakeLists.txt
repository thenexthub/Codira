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

cmake_minimum_required(VERSION 3.10)

project(securec)

set(SOURCES
    ${SECUREC_ROOT}/src/securec/vsprintf_s.c
    ${SECUREC_ROOT}/src/securec/wmemmove_s.c
    ${SECUREC_ROOT}/src/securec/strncat_s.c
    ${SECUREC_ROOT}/src/securec/vsnprintf_s.c
    ${SECUREC_ROOT}/src/securec/fwscanf_s.c
    ${SECUREC_ROOT}/src/securec/scanf_s.c
    ${SECUREC_ROOT}/src/securec/strcat_s.c
    ${SECUREC_ROOT}/src/securec/sscanf_s.c
    ${SECUREC_ROOT}/src/securec/secureprintoutput_w.c
    ${SECUREC_ROOT}/src/securec/wmemcpy_s.c
    ${SECUREC_ROOT}/src/securec/wcsncat_s.c
    ${SECUREC_ROOT}/src/securec/secureprintoutput_a.c
    ${SECUREC_ROOT}/src/securec/secureinput_w.c
    ${SECUREC_ROOT}/src/securec/memcpy_s.c
    ${SECUREC_ROOT}/src/securec/fscanf_s.c
    ${SECUREC_ROOT}/src/securec/vswscanf_s.c
    ${SECUREC_ROOT}/src/securec/secureinput_a.c
    ${SECUREC_ROOT}/src/securec/sprintf_s.c
    ${SECUREC_ROOT}/src/securec/memmove_s.c
    ${SECUREC_ROOT}/src/securec/swscanf_s.c
    ${SECUREC_ROOT}/src/securec/snprintf_s.c
    ${SECUREC_ROOT}/src/securec/vscanf_s.c
    ${SECUREC_ROOT}/src/securec/vswprintf_s.c
    ${SECUREC_ROOT}/src/securec/wcscpy_s.c
    ${SECUREC_ROOT}/src/securec/vfwscanf_s.c
    ${SECUREC_ROOT}/src/securec/memset_s.c
    ${SECUREC_ROOT}/src/securec/wscanf_s.c
    ${SECUREC_ROOT}/src/securec/vwscanf_s.c
    ${SECUREC_ROOT}/src/securec/strtok_s.c
    ${SECUREC_ROOT}/src/securec/wcsncpy_s.c
    ${SECUREC_ROOT}/src/securec/vfscanf_s.c
    ${SECUREC_ROOT}/src/securec/vsscanf_s.c
    ${SECUREC_ROOT}/src/securec/wcstok_s.c
    ${SECUREC_ROOT}/src/securec/securecutil.c
    ${SECUREC_ROOT}/src/securec/gets_s.c
    ${SECUREC_ROOT}/src/securec/swprintf_s.c
    ${SECUREC_ROOT}/src/securec/strcpy_s.c
    ${SECUREC_ROOT}/src/securec/wcscat_s.c
    ${SECUREC_ROOT}/src/securec/strncpy_s.c
)

panda_add_library(c_secshared ${PANDA_DEFAULT_LIB_TYPE} ${SOURCES})
panda_target_include_directories(c_secshared SYSTEM PUBLIC ${SECUREC_ROOT}/include)
panda_target_compile_definitions(c_secshared PRIVATE NDEBUG SECUREC_SUPPORT_STRTOLD=1)
if (PANDA_TARGET_WINDOWS)
    panda_target_compile_definitions(c_secshared PUBLIC __LIBMSVCRT__)
endif()
panda_target_compile_options(c_secshared PRIVATE -Wall -O1)

if(HOST_TOOLS)
    set_target_properties(c_secshared
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/third_party/libc_sec
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/third_party/libc_sec
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/third_party/libc_sec
    )
endif()

if (PANDA_ENABLE_AFL)
    panda_add_library(c_secshared_fuzz ${PANDA_DEFAULT_LIB_TYPE} ${SOURCES})
    panda_target_include_directories(c_secshared_fuzz SYSTEM PUBLIC ${SECUREC_ROOT}/include)
    panda_target_compile_definitions(c_secshared_fuzz PRIVATE NDEBUG SECUREC_SUPPORT_STRTOLD=1)
    if (PANDA_TARGET_WINDOWS)
        panda_target_compile_definitions(c_secshared_fuzz PUBLIC _CRTBLD __LIBMSVCRT__)
    endif()

    panda_target_compile_options(c_secshared_fuzz PRIVATE -Wall -O1 ${AFL_NOOPT})
endif()
