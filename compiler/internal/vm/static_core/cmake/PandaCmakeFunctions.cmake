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

# 32-bits pointers optimization in Panda for objects => addresses usage low 4GB
# We need use linker scripts for section replacement above 4GB
# Note: AddressSanitizer reserves (mmap) addresses for its own needs,
#       so we have difference start addresses for asan and default buildings
function(panda_add_executable target)
    # When using rapidcheck we should use linker scripts with
    # definitions for exception handling sections
    cmake_parse_arguments(ARG "RAPIDCHECK_ON;FORCE_REBUILD" "OUTPUT_DIRECTORY" "" ${ARGN})

    if(PANDA_USE_PREBUILT_TARGETS AND NOT ARG_FORCE_REBUILD)
        if(NOT DEFINED ARG_OUTPUT_DIRECTORY)
            set(ARG_OUTPUT_DIRECTORY "${PANDA_BINARY_ROOT}/bin/${target}")
        endif()

        message(VERBOSE "Use prebuilt ${target}")
        add_executable(${target} IMPORTED GLOBAL)
        set_property(TARGET ${target} PROPERTY
                     IMPORTED_LOCATION ${ARG_OUTPUT_DIRECTORY})
        return()
    endif()

    add_executable(${target} ${ARG_UNPARSED_ARGUMENTS})

    if(DEFINED ARG_OUTPUT_DIRECTORY)
        set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${ARG_OUTPUT_DIRECTORY}")
    endif()

    if(PANDA_32_BIT_MANAGED_POINTER AND PANDA_TARGET_64 AND NOT (PANDA_TARGET_MOBILE OR PANDA_TARGET_WINDOWS OR PANDA_TARGET_MACOS OR PANDA_TARGET_OHOS))
        if(ARG_RAPIDCHECK_ON)
            set(LINKER_SCRIPT_ARG "-Wl,-T,${PANDA_ROOT}/ldscripts/panda_test_asan.ld")
        else()
            set(LINKER_SCRIPT_ARG "-Wl,-T,${PANDA_ROOT}/ldscripts/panda.ld")
        endif()
        if(PANDA_ENABLE_ADDRESS_SANITIZER)
            # For cross-aarch64 with ASAN with linker script we need use additional path-link
            # Here we use default addresses space (without ASAN flag). It is nuance of cross-building.
            if(PANDA_TARGET_ARM64)
                set(LINKER_SCRIPT_ARG "-Wl,-rpath-link,${PANDA_SYSROOT}/lib ${LINKER_SCRIPT_ARG}")
            else()
                set(LINKER_SCRIPT_ARG "-Wl,--defsym,LSFLAG_ASAN=1 ${LINKER_SCRIPT_ARG}")
            endif()
        endif()
        # We need use specific options for AMD64 building with Clang compiler
        # because it is necessary for placement of sections above 4GB
        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND PANDA_TARGET_AMD64)
            set(LINKER_SCRIPT_ARG "${LINKER_SCRIPT_ARG} -pie")
            # -mcmodel=large with clang and asan on x64 leads to bugs in rapidcheck with high-mem mappings
            # so for rapidcheck test binaries panda_test_asan.ld is used which does not require
            # -mcmodel=large option
            if(NOT ARG_RAPIDCHECK_ON)
                target_compile_options(${target} PUBLIC "-mcmodel=large")
            endif()
        endif()
        target_link_libraries(${target} ${LINKER_SCRIPT_ARG})
    endif()
endfunction()

# This function need for non-SHARED libraries
# It is necessary for 32-bits pointers via amd64 building with Clang compiler
function(panda_set_lib_32bit_property target)
    if(PANDA_32_BIT_MANAGED_POINTER AND PANDA_TARGET_AMD64 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set_property(TARGET ${target} PROPERTY POSITION_INDEPENDENT_CODE ON)
    endif()
endfunction()

function(panda_add_library target lib_type)
    cmake_parse_arguments(ARG "FORCE_REBUILD" "" "" ${ARGN})
    if(ARG_FORCE_REBUILD OR
       NOT PANDA_USE_PREBUILT_TARGETS OR
       NOT (lib_type STREQUAL SHARED) OR
       NOT ARG_UNPARSED_ARGUMENTS)
        add_library(${target} ${lib_type} ${ARG_UNPARSED_ARGUMENTS})
        return()
    endif()

    set(RELATIVE_PATH "lib/lib${target}.so")
    set(IMPORTED_LOCATION "${PANDA_BINARY_ROOT}/${RELATIVE_PATH}")
    if(NOT EXISTS ${IMPORTED_LOCATION})
        set(IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/${RELATIVE_PATH}")
    endif()

    message(VERBOSE "Use prebuilt lib${target}.so")
    add_library(${target} ${lib_type} IMPORTED GLOBAL)
    set_property(TARGET ${target} PROPERTY
                 IMPORTED_LOCATION ${IMPORTED_LOCATION})
endfunction()

function(panda_target_link_libraries target)
    get_target_property(target_is_imported ${target} IMPORTED)
    if(NOT target_is_imported)
        target_link_libraries(${ARGV})
        return()
    endif()

    cmake_parse_arguments(ARG "" "" "PUBLIC;PRIVATE;INTERFACE" ${ARGN})
    set(NON_PRIVATE_LIBRARIES ${ARG_PUBLIC} ${ARG_INTERFACE} ${ARG_UNPARSED_ARGUMENTS})

    foreach(lib IN LISTS NON_PRIVATE_LIBRARIES ARG_PRIVATE)
        if(TARGET ${lib})
            get_target_property(lib_type ${lib} TYPE)
            if(lib_type STREQUAL SHARED_LIBRARY)
                add_dependencies(${target} ${lib})
            endif()
        endif()
    endforeach()
    target_link_libraries(${target} INTERFACE ${NON_PRIVATE_LIBRARIES})
endfunction()

function(panda_target_include_directories target)
    get_target_property(target_is_imported ${target} IMPORTED)
    if(NOT target_is_imported)
        target_include_directories(${ARGV})
        return()
    endif()

    cmake_parse_arguments(ARG "SYSTEM" "" "PUBLIC;PRIVATE;INTERFACE" ${ARGN})
    if(ARG_SYSTEM)
        set(SYSTEM_ON "SYSTEM")
    endif()

    target_include_directories(${target} ${SYSTEM_ON} INTERFACE ${ARG_PUBLIC} ${ARG_INTERFACE})
endfunction()

function(panda_target_compile_options target)
    get_target_property(target_is_imported ${target} IMPORTED)
    if(NOT target_is_imported)
        target_compile_options(${ARGV})
        return()
    endif()

    cmake_parse_arguments(ARG "" "" "PUBLIC;PRIVATE;INTERFACE" ${ARGN})
    target_compile_options(${target} INTERFACE ${ARG_PUBLIC} ${ARG_INTERFACE})
endfunction()

function(panda_target_compile_definitions target)
    get_target_property(target_is_imported ${target} IMPORTED)
    if(NOT target_is_imported)
        target_compile_definitions(${ARGV})
        return()
    endif()

    cmake_parse_arguments(ARG "" "" "PUBLIC;PRIVATE;INTERFACE" ${ARGN})
    target_compile_definitions(${target} INTERFACE ${ARG_PUBLIC} ${ARG_INTERFACE})
endfunction()

function(panda_target_sources target)
    get_target_property(target_is_imported ${target} IMPORTED)
    if(NOT target_is_imported)
        target_sources(${ARGV})
    endif()
endfunction()
