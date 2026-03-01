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

# The fuzzing engine depends on fuzz/fuzz.
# fuzz/fuzz compiled with -fsanitize-coverage may cause incorrectly coverage feedback.

string(REGEX REPLACE "-fsanitize-coverage=[^ ]+" "" CMAKE_C_FLAGS_NO_COVERAGE "${CMAKE_C_FLAGS}" )
string(REGEX REPLACE "-fsanitize-coverage=[^ ]+" "" CMAKE_CXX_FLAGS_NO_COVERAGE "${CMAKE_CXX_FLAGS}" )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_NO_COVERAGE}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_NO_COVERAGE}")
FILE(GLOB fuzz_ffi_src 
    ./code_sancov_standard.c
    ./code_sancov_standard.h
    ./fake_stacktrace_printer.c
    ./fake_stacktrace_printer.h
    ./utf8_fix.c
    ./utf8_fix.h
    ./codenative_sanitizer_coverage.h)
set(libname stdx.fuzz.fuzzFFI)

add_library(${libname}-objs OBJECT ${fuzz_ffi_src})

if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_library(${libname} STATIC $<TARGET_OBJECTS:${libname}-objs>)
endif()
install_cangjie_library_ffi(${libname})
