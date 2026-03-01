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

file(GLOB env_ffi_src ./*.c)
set(libname cangjie-std-envFFI)
include(AddAndCombineStaticLib)
add_library(${libname}-objs OBJECT ${env_ffi_src})
target_compile_options(${libname}-objs PRIVATE ${CMAKE_C_COVERAGE_FLAGS})
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_and_combine_static_lib(
        TARGET ${libname}
        OUTPUT_NAME lib${libname}.a
        LIBRARIES
            $<TARGET_OBJECTS:${libname}-objs>
            $<TARGET_OBJECTS:cangjie-std-processFFI-objs>
        DEPENDS ${libname}-objs cangjie-std-processFFI )
        
    string(TOLOWER ${TARGET_TRIPLE_DIRECTORY_PREFIX} output_lib_dir)
    set(CODENATIVE_BACKEND "codenative")
    install(FILES ${CMAKE_BINARY_DIR}/lib/lib${libname}.a DESTINATION lib/${output_lib_dir}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH})
endif()
