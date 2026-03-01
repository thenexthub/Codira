# Copyright (c) 2023-2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

if (NOT PANDA_LLVM_IRTOC)
    message(FATAL_ERROR "LLVMInlineModules must be included before Definitions.cmake")
endif ()

if (NOT DEFINED PANDA_LLVM_INTERPRETER_INLINING)
    set(PANDA_LLVM_INTERPRETER_INLINING true)
endif ()

if (NOT PANDA_LLVM_INTERPRETER_INLINING)
    message(STATUS "LLVM IR inlining disabled manually, PANDA_LLVM_INTERPRETER_INLINING = '${PANDA_LLVM_INTERPRETER_INLINING}'")
endif ()

if (PANDA_LLVM_INTERPRETER_INLINING
        AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
        # HostTools build, compiler does not matter. Parent project built bitcode files for inlining, and set
        # IRTOC_INT_LLVM_INL_MODULE_FILES_LIST, the file has list of bitcode files which we'll link later and use as an inline
        # module
        AND "${IRTOC_INT_LLVM_INL_MODULE_FILES_LIST}" STREQUAL "")
    message(STATUS "LLVM IR inlining disabled because compiler is not 'Clang' but = '${CMAKE_CXX_COMPILER_ID}'")
    message(STATUS "and IRTOC_INT_LLVM_INL_MODULE_FILES_LIST = '${IRTOC_INT_LLVM_INL_MODULE_FILES_LIST}'")
    set(PANDA_LLVM_INTERPRETER_INLINING false)
endif ()

if (PANDA_LLVM_INTERPRETER_INLINING)
    message(STATUS "LLVM IR inlining enabled")
endif ()

if (NOT "${IRTOC_INT_LLVM_INL_MODULE_FILES_LIST}" STREQUAL "")
    # Parent project set it using -DIRTOC_INT_LLVM_INL_MODULE_FILES_LIST
    # The file contains list of paths of bitcode files delimited by ';'
    # Example /tmp/a.o;/tmp/b.o
    # Read the file
    file(READ ${IRTOC_INT_LLVM_INL_MODULE_FILES_LIST} IRTOC_INT_LLVM_INL_MODULE)
    # Split the contents into a list
    string(REPLACE "\\;" ";" "${IRTOC_INT_LLVM_INL_MODULE}" IRTOC_INT_LLVM_INL_MODULE)
elseif (NOT PANDA_LLVM_INTERPRETER_INLINING)
    # Must be unused outside
    set(IRTOC_INT_LLVM_INL_MODULE "")
else ()
    # Pre-define the path for a file with a list of bitcode files for inlining.
    # Define the path here to configure HostTools
    set(IRTOC_INT_LLVM_INL_MODULE_FILES_LIST "${CMAKE_BINARY_DIR}/irtoc/irtoc_interpreter/interpreter_inline_files.txt")
endif ()

# Produce a list of llvm bitcode files for given sources
#
# Parameters:
#
#    TARGET - name of cmake target
#    OUTPUT_VARIABLE - name of the variable in parent scope to set paths to bitcode files
#    SOURCES - source files to build the module
#    INCLUDES - paths to includes when compiling SOURCES
#    DEPENDS - dependencies of the TARGET
function(panda_add_llvm_bc_lib)
    set(prefix ARG)
    set(singleValues TARGET OUTPUT_VARIABLE)
    set(multiValues SOURCES INCLUDES DEPENDS)
    cmake_parse_arguments(${prefix}
            "${noValues}"
            "${singleValues}"
            "${multiValues}"
            ${ARGN})
    if (NOT PANDA_LLVM_INTERPRETER_INLINING)
        add_custom_target(${ARG_TARGET} COMMENT "Do not generate llvm inline module because NOT PANDA_LLVM_INTERPRETER_INLINING")
        return()
    endif ()
    if (NOT DEFINED ARG_SOURCES)
        message(FATAL_ERROR "Mandatory SOURCES argument is not defined.")
    endif ()
    if (NOT DEFINED ARG_TARGET)
        message(FATAL_ERROR "Mandatory TARGET argument is not defined.")
    endif ()
    if (NOT DEFINED ARG_OUTPUT_VARIABLE)
        message(FATAL_ERROR "Mandatory OUTPUT_VARIABLE argument is not defined.")
    endif ()
    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(FATAL_ERROR "Compiler must be clang but CMAKE_CXX_COMPILER_ID = '${CMAKE_CXX_COMPILER_ID}'")
    endif ()

    add_custom_target("${ARG_TARGET}")
    if (HOST_TOOLS)
        message(STATUS "Skipping 'panda_add_llvm_bc_lib'. Reason: HOST_TOOLS")
        return()
    endif ()

    # Form bitcode files as an object library
    set(BITCODE_OBJECTS "${ARG_TARGET}_bitcode")
    panda_add_library(${BITCODE_OBJECTS} OBJECT ${ARG_SOURCES})
    # Mark all includes as SYSTEM and PRIVATE.
    # SYSTEM because passed INCLUDES contain third party headers
    # PRIVATE because we do not expect anyone to link against the bitcode file
    foreach (include IN LISTS ARG_INCLUDES)
        panda_target_include_directories(${BITCODE_OBJECTS} SYSTEM PRIVATE ${include})
    endforeach ()
    add_dependencies(${BITCODE_OBJECTS} ${ARG_DEPENDS})
    set_property(TARGET ${BITCODE_OBJECTS} PROPERTY POSITION_INDEPENDENT_CODE ON)
    set(optimization_options -O1 "SHELL:-mllvm -disable-llvm-optzns")
    if (PANDA_SANITIZERS_LIST)
        # -disable-llvm-optzns disables ASan
        set(optimization_options -O1)
    endif ()
    panda_target_compile_options(${BITCODE_OBJECTS} PUBLIC -Wno-invalid-offsetof -emit-llvm ${optimization_options})
    panda_add_sanitizers(TARGET ${BITCODE_OBJECTS} SANITIZERS ${PANDA_SANITIZERS_LIST})
    set(${ARG_OUTPUT_VARIABLE} $<TARGET_OBJECTS:${BITCODE_OBJECTS}> PARENT_SCOPE)
    add_dependencies(${ARG_TARGET} ${BITCODE_OBJECTS})

    if (TARGET host_tools_depends)
        add_dependencies(host_tools_depends ${ARG_TARGET})
    endif ()

endfunction()
