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

set(CODIRA_CORE_SIMD_FFI_SRC string_SIMD.h string_SIMD.c)

set(libsimd coreSIMDFFI)
# The fuzzing engine depends on std/core.
# std/core compiled with -fsanitize-coverage may cause incorrectly coverage feedback.
string(REGEX REPLACE "-fsanitize-coverage=[^ ]+" "" CMAKE_C_FLAGS_NO_COVERAGE "${CMAKE_C_FLAGS}" )
string(REGEX REPLACE "-fsanitize-coverage=[^ ]+" "" CMAKE_CXX_FLAGS_NO_COVERAGE "${CMAKE_CXX_FLAGS}" )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_NO_COVERAGE}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_NO_COVERAGE}")

add_library(${libsimd}-objs OBJECT ${CODIRA_CORE_SIMD_FFI_SRC})
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
    target_compile_options(${libsimd}-objs PRIVATE -mfma -mavx2)
endif()
target_compile_options(${libsimd}-objs PRIVATE ${CMAKE_C_COVERAGE_FLAGS})

set(CODIRA_CORE_STRSTR_FFI_SRC core.h string_strstr.c)
set(libstrstr coreSTRSTRFFI)
add_library(${libstrstr}-objs OBJECT ${CODIRA_CORE_STRSTR_FFI_SRC})
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64" AND NOT ANDROID)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${libstrstr}-objs PRIVATE -mavx2 -mllvm -force-vector-interleave=32)
    endif()
endif()
target_compile_options(${libstrstr}-objs PRIVATE ${CMAKE_C_COVERAGE_FLAGS})

set(libmath coreMathFFI)
set(libprint corePrintFFI)
set(libstring coreStringFFI)
# compile math.c alone to control compiling option range
add_library(${libmath}-objs OBJECT math.c)
add_library(${libprint}-objs OBJECT print.c)
add_library(${libstring}-objs OBJECT string.c)
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64" AND (NOT NDEBUG))
    # link libgcc for function `isfinte()` to link symbols `__unordtf2` on
    # O0(debug) mode and for aarch64
    target_compile_options(${libmath}-objs PRIVATE -O1)
    target_compile_options(${libprint}-objs PRIVATE -O1)
    target_compile_options(${libstring}-objs PRIVATE -O1)
endif()

set(CODIRA_CORE_OTHER_FFI_SRC
    core.h
    string_SIMD.h
    SIMD_support.c
    atexit.c
    endian.c
    string_utf8.c
    console_read.c)
set(libcore-other coreOtherFFI)
add_library(${libcore-other}-objs OBJECT ${CODIRA_CORE_OTHER_FFI_SRC})
target_compile_options(${libcore-other}-objs PRIVATE ${CMAKE_C_COVERAGE_FLAGS})

# we are not expect typetemplate.c to be instrumented by asan or hwasan.
set(TYPETEMPLATE_SRC $ENV{CODIRA_HOME}/include/cangjie/Basic/UGTypeKind.h typetemplate.c)
add_library(template-objs OBJECT ${TYPETEMPLATE_SRC})
target_compile_options(template-objs PRIVATE ${CMAKE_C_COVERAGE_FLAGS})

if(APPLE)
    set(TARGET_OBJCOPY llvm-objcopy)
else()
    if(CMAKE_CROSSCOMPILING)
        if(${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
            set(TARGET_OBJCOPY ${CODIRA_TARGET_TOOLCHAIN}/llvm-objcopy)
        else()
            set(TARGET_OBJCOPY ${CODIRA_TARGET_TOOLCHAIN}/${TRIPLE}-objcopy)
        endif()
    else()
        set(TARGET_OBJCOPY objcopy)
    endif()
endif()

if(APPLE)
    set(SYM_PREFIX _)
else()
    set(SYM_PREFIX)
endif()

add_custom_target(modify_typetemplate_object ALL
    COMMENT "Modifying symbol names in typetemplate.c.o"
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}Int64_ti=${SYM_PREFIX}Int64.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}Nothing_ti=${SYM_PREFIX}Nothing.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}Bool_ti=${SYM_PREFIX}Bool.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}Rune_ti=${SYM_PREFIX}Rune.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}UInt8_ti=${SYM_PREFIX}UInt8.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}UInt16_ti=${SYM_PREFIX}UInt16.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}UInt32_ti=${SYM_PREFIX}UInt32.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}UInt64_ti=${SYM_PREFIX}UInt64.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}UIntNative_ti=${SYM_PREFIX}UIntNative.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}Int8_ti=${SYM_PREFIX}Int8.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}Int16_ti=${SYM_PREFIX}Int16.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}Int32_ti=${SYM_PREFIX}Int32.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}IntNative_ti=${SYM_PREFIX}IntNative.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}Float16_ti=${SYM_PREFIX}Float16.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}Float32_ti=${SYM_PREFIX}Float32.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}Float64_ti=${SYM_PREFIX}Float64.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}Unit_ti=${SYM_PREFIX}Unit.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}Tuple_tt=${SYM_PREFIX}Tuple.tt $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}Box_tt=${SYM_PREFIX}Box.tt $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}VArray_tt=${SYM_PREFIX}VArray.tt $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}RawArray_tt=${SYM_PREFIX}RawArray.tt $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}CPointer_tt=${SYM_PREFIX}CPointer.tt $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}CString_ti=${SYM_PREFIX}CString.ti $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}CFunc_tt=${SYM_PREFIX}CFunc.tt $<TARGET_OBJECTS:template-objs>
    COMMAND ${TARGET_OBJCOPY} --redefine-sym ${SYM_PREFIX}Closure_tt=${SYM_PREFIX}Closure.tt $<TARGET_OBJECTS:template-objs>
    VERBATIM
    DEPENDS template-objs # make sure template-objs build finished
    )

set(CORE_FFI_OBJECTS_LIST
    $<TARGET_OBJECTS:${libcore-other}-objs>
    $<TARGET_OBJECTS:${libmath}-objs>
    $<TARGET_OBJECTS:${libprint}-objs>
    $<TARGET_OBJECTS:${libstring}-objs>
    $<TARGET_OBJECTS:${libsimd}-objs>
    $<TARGET_OBJECTS:${libstrstr}-objs>
    $<TARGET_OBJECTS:template-objs>
    CACHE INTERNAL "")
set(libname cangjie-std-coreFFI)
add_library(${libname} STATIC ${CORE_FFI_OBJECTS_LIST})
target_compile_options(${libname} PRIVATE -fno-builtin-bcmp)
install_cangjie_library_ffi(${libname})
# make sure modify_typetemplate_object build finished
add_dependencies(${libname} modify_typetemplate_object)