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

# Generator of arch-dependent values for cross-compiling
#
# The target platforms should be specified with `create_cross_values_target(arch_name toolchain)` function.
# Basically, the function adds an `ExternalProject` with ${PANDA_ROOT} as a source, configures it with
# the specified toolchain and generates a header with values based on `asm_defines.def`. After that,
# an umbrella-header `cross_values.h` is being generated (with the values included)
#
# Currently, only AMD64 cross-compiling is supported.

# Prevent recursive generation:
if (NOT CROSS_VALUES_CONFIG)
    add_custom_target(cross_values_generator)
    set(GENERATED_CROSS_VALUES_DIR "${PANDA_BINARY_ROOT}/cross_values/generated_values")
    file(MAKE_DIRECTORY ${GENERATED_CROSS_VALUES_DIR})

    include(ExternalProject)
    function(create_cross_values_target arch toolchain)
        set(CROSS_ASM_DEF_TARGET_NAME "cross_asm_defines_${arch}")
        set(PANDA_AUX_PROJECT_SOURCE ${PANDA_ROOT})
        set(PANDA_AUX_BINARY_DIR auxiliary_panda_binary_dirs/${arch})
        set(CMAKE_VARIABLES
            "'-G${CMAKE_GENERATOR}'
            -DPANDA_USE_64_BIT_POINTER=${PANDA_USE_64_BIT_POINTER}
            -DES2PANDA_PATH=${ES2PANDA_PATH}
            -DCMAKE_TOOLCHAIN_FILE=${toolchain}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DANDROID_ABI=${ANDROID_ABI}
            -DANDROID_PLATFORM=${ANDROID_PLATFORM}
            -DPANDA_TARGET_MOBILE_WITH_MANAGED_LIBS=${PANDA_TARGET_MOBILE_WITH_MANAGED_LIBS}
            -DMOBILE_NATIVE_LIBS_SOURCE_PATH=${MOBILE_NATIVE_LIBS_SOURCE_PATH}
            -DPANDA_TARGET_ARM32_ABI_HARD=${PANDA_TARGET_ARM32_ABI_HARD}
            -DPANDA_PRODUCT_BUILD=${PANDA_PRODUCT_BUILD}
            -DTOOLCHAIN_CLANG_ROOT=${TOOLCHAIN_CLANG_ROOT}
            -DTOOLCHAIN_SYSROOT=${TOOLCHAIN_SYSROOT}
            -DPANDA_WITH_TESTS=FALSE
            -DPANDA_WITH_BENCHMARKS=FALSE
            -DPANDA_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER=${PANDA_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER}
            -DPANDA_ENABLE_ADDRESS_SANITIZER=${PANDA_ENABLE_ADDRESS_SANITIZER}
            -DCROSS_VALUES_CONFIG=TRUE
            --no-warn-unused-cli
            -Wno-deprecated"
        )

        foreach(plugin ${PLUGINS})
            string(TOUPPER ${plugin} plugin_name_upper)
            string(CONCAT PANDA_WITH_PLUGIN "PANDA_WITH_" ${plugin_name_upper})
            string(APPEND CMAKE_VARIABLES "
            -DPANDA_WITH_${plugin_name_upper}=${${PANDA_WITH_PLUGIN}}")
        endforeach()
        
        set(BUILD_COMMAND "${CMAKE_COMMAND} --build <BINARY_DIR> --target asm_defines")
        separate_arguments(CMAKE_VARIABLES_SEP UNIX_COMMAND ${CMAKE_VARIABLES})
        separate_arguments(BUILD_COMMAND UNIX_COMMAND ${BUILD_COMMAND})

        # Configure subprojects at main project's configure step:
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${PANDA_AUX_BINARY_DIR}")
        message(STATUS "  Configuring subproject in ${CMAKE_CURRENT_BINARY_DIR}/${PANDA_AUX_BINARY_DIR} via `execute_process()`")
        set(TEMP_CMAKE_LOG_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PANDA_AUX_BINARY_DIR}/cmake_log.tmp")
        execute_process(
            COMMAND ${CMAKE_COMMAND} ${PANDA_AUX_PROJECT_SOURCE} ${CMAKE_VARIABLES_SEP}
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${PANDA_AUX_BINARY_DIR}"
            RESULT_VARIABLE ret
            OUTPUT_FILE ${TEMP_CMAKE_LOG_FILE}
            ERROR_FILE ${TEMP_CMAKE_LOG_FILE}
        )
        if (NOT ret EQUAL "0")
            file(READ ${TEMP_CMAKE_LOG_FILE} TEMP_CMAKE_LOG)
            string(REPLACE "\n" "\n " TEMP_CMAKE_LOG ${TEMP_CMAKE_LOG})
            string(REPLACE "\n" " " CMAKE_VARIABLES ${CMAKE_VARIABLES})
            message(FATAL_ERROR
                " ${CMAKE_CURRENT_BINARY_DIR}/${PANDA_AUX_BINARY_DIR} configuration failed:\n"
                " ${TEMP_CMAKE_LOG}\n"
                " Reproduce:"
                " cd ${CMAKE_CURRENT_BINARY_DIR}/${PANDA_AUX_BINARY_DIR} && ${CMAKE_COMMAND} ${PANDA_AUX_PROJECT_SOURCE} ${CMAKE_VARIABLES}\n"
            )
        endif()

        file(WRITE ${TEMP_CMAKE_LOG_FILE} "")
        message(STATUS "  Building `asm_defines` in ${CMAKE_CURRENT_BINARY_DIR}/${PANDA_AUX_BINARY_DIR} via `execute_process()`")
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build . --target asm_defines
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${PANDA_AUX_BINARY_DIR}"
            RESULT_VARIABLE ret
            OUTPUT_FILE ${TEMP_CMAKE_LOG_FILE}
            ERROR_FILE ${TEMP_CMAKE_LOG_FILE}
        )
        if (NOT ret EQUAL "0")
            file(READ ${TEMP_CMAKE_LOG_FILE} TEMP_CMAKE_LOG)
            string(REPLACE "\n" "\n " TEMP_CMAKE_LOG ${TEMP_CMAKE_LOG})
            message(FATAL_ERROR
                " ${PANDA_AUX_BINARY_DIR}/runtime/asm_defines failed:\n"
                " ${TEMP_CMAKE_LOG}\n"
                " Reproduce:"
                " cd ${CMAKE_CURRENT_BINARY_DIR}/${PANDA_AUX_BINARY_DIR} && ${CMAKE_COMMAND} --build . --target asm_defines\n"
            )
        endif()

        # Add targets to the main project:
        ExternalProject_Add(${CROSS_ASM_DEF_TARGET_NAME}
                SOURCE_DIR        "${PANDA_AUX_PROJECT_SOURCE}"
                TMP_DIR           "${PANDA_AUX_BINARY_DIR}/${CROSS_ASM_DEF_TARGET_NAME}_tmp"
                STAMP_DIR         "${PANDA_AUX_BINARY_DIR}/${CROSS_ASM_DEF_TARGET_NAME}_stamp"
                DOWNLOAD_DIR      "${PANDA_AUX_BINARY_DIR}"
                BINARY_DIR        "${PANDA_AUX_BINARY_DIR}"
                INSTALL_DIR       "${PANDA_AUX_BINARY_DIR}"
                BUILD_IN_SOURCE   FALSE
                BUILD_ALWAYS      TRUE
                CONFIGURE_COMMAND ""
                BUILD_COMMAND     ${BUILD_COMMAND}
                INSTALL_COMMAND   ""
        )

        set(ASM_DIR "${CMAKE_CURRENT_BINARY_DIR}/${PANDA_AUX_BINARY_DIR}/runtime/asm_defines")
        set(ASM_FILE "libasm_defines.S")

        add_custom_target(cross_asm_defines_${arch}_gen
            COMMAND ${CMAKE_COMMAND} --build . --target asm_defines
                    >${CMAKE_CURRENT_BINARY_DIR}/${PANDA_AUX_BINARY_DIR}/cross_asm_defines.log
                    || (cat ${CMAKE_CURRENT_BINARY_DIR}/${PANDA_AUX_BINARY_DIR}/cross_asm_defines.log && false)
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${PANDA_AUX_BINARY_DIR}"
            BYPRODUCTS ${ASM_DIR}/${ASM_FILE}
        )

        set(OUTPUT_DIR ${GENERATED_CROSS_VALUES_DIR})
        set(OUTPUT_FILE "${arch}_values_gen.h")

        add_custom_command(OUTPUT ${OUTPUT_DIR}/${OUTPUT_FILE}
            COMMAND ruby ${CMAKE_CURRENT_SOURCE_DIR}/cross_values_generator.rb ${ASM_DIR}/${ASM_FILE} ${OUTPUT_DIR}/${OUTPUT_FILE} ${arch}
            DEPENDS ${ASM_DIR}/${ASM_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/cross_values_generator.rb
        )

        add_custom_target(cross_values_generate_${arch}
            DEPENDS ${OUTPUT_DIR}/${OUTPUT_FILE}
        )
        add_dependencies(cross_values_generator cross_values_generate_${arch})
        set(GENERATED_CROSS_VALUES_HEADERS "${GENERATED_CROSS_VALUES_HEADERS} ${OUTPUT_DIR}/${OUTPUT_FILE}" PARENT_SCOPE)
    endfunction()

    # By default, gcc-toolchain is used:
    set(PANDA_CROSS_X86_64_DEFAULT_TOOLCHAIN_FILE ${PANDA_ROOT}/cmake/toolchain/host_gcc_default.cmake)
    set(PANDA_CROSS_AARCH64_DEFAULT_TOOLCHAIN_FILE ${PANDA_ROOT}/cmake/toolchain/cross-gcc-default-qemu-aarch64.cmake)
    set(PANDA_CROSS_AARCH32_DEFAULT_TOOLCHAIN_FILE ${PANDA_ROOT}/cmake/toolchain/cross-gcc-default-qemu-arm-linux-gnueabi.cmake)

    # Enable cross-target offsets for x86_64 only:
    if (PANDA_TARGET_AMD64)
        if (HOST_TOOLS)
            create_cross_values_target(${PANDA_HOST_TOOLS_TARGET_ARCH} ${PANDA_HOST_TOOLS_TARGET_TOOLCHAIN})
        else()
            set(TARGET_CROSS_VALUES ${PANDA_BINARY_ROOT}/cross_values/generated_values/X86_64_values_gen.h)
            set(TARGET_ARCH X86_64)
            if (PANDA_COMPILER_TARGET_X86_64)
                message(STATUS "Generating cross-values for X86_64")
                if (PANDA_CROSS_X86_64_TOOLCHAIN_FILE)
                    create_cross_values_target(X86_64 ${PANDA_CROSS_X86_64_TOOLCHAIN_FILE})
                else()
                    create_cross_values_target(X86_64 ${PANDA_CROSS_X86_64_DEFAULT_TOOLCHAIN_FILE})
                endif()
            endif()
            if (PANDA_COMPILER_TARGET_AARCH64)
                message(STATUS "Generating cross-values for AARCH64")
                if (PANDA_CROSS_AARCH64_TOOLCHAIN_FILE)
                    create_cross_values_target(AARCH64 ${PANDA_CROSS_AARCH64_TOOLCHAIN_FILE})
                else()
                    create_cross_values_target(AARCH64 ${PANDA_CROSS_AARCH64_DEFAULT_TOOLCHAIN_FILE})
                endif()
            endif()
            if (PANDA_COMPILER_TARGET_AARCH32)
                message(STATUS "Generating cross-values for ARM")
                if (PANDA_CROSS_AARCH32_TOOLCHAIN_FILE)
                    create_cross_values_target(AARCH32 ${PANDA_CROSS_AARCH32_TOOLCHAIN_FILE})
                else()
                    create_cross_values_target(AARCH32 ${PANDA_CROSS_AARCH32_DEFAULT_TOOLCHAIN_FILE})
                endif()
            endif()
        endif()
    elseif(PANDA_TARGET_ARM64)
        message(STATUS "Generating cross-values for AARCH64")
        set(TARGET_CROSS_VALUES ${PANDA_BINARY_ROOT}/cross_values/generated_values/AARCH64_values_gen.h)
        set(TARGET_ARCH AARCH64)
        create_cross_values_target(AARCH64 "${CMAKE_TOOLCHAIN_FILE}")
        if(PANDA_COMPILER_TARGET_X86_64 OR PANDA_COMPILER_TARGET_AARCH32)
            message(FATAL_ERROR "Cross-compiling is supported for AMD64 only")
        endif()
    elseif(PANDA_TARGET_ARM32)
        message(STATUS "Generating cross-values for ARM")
        set(TARGET_CROSS_VALUES ${PANDA_BINARY_ROOT}/cross_values/generated_values/AARCH32_values_gen.h)
        set(TARGET_ARCH AARCH32)
        create_cross_values_target(AARCH32 "${CMAKE_TOOLCHAIN_FILE}")
        if(PANDA_COMPILER_TARGET_X86_64 OR PANDA_COMPILER_TARGET_AARCH64)
            message(FATAL_ERROR "Cross-compiling is supported for AMD64 only")
        endif()
    endif()

    set(OUTPUT_DIR "${PANDA_BINARY_ROOT}/cross_values")
    set(OUTPUT_FILE "cross_values.h")

    set(ASM_DEFINES_INPUT "${CMAKE_SOURCE_DIR}/runtime/asm_defines/asm_defines.def")
    set(ASM_DEFINES_OUTPUT "${OUTPUT_DIR}/asm_defines_def.cpp")

    add_custom_command(OUTPUT ${ASM_DEFINES_OUTPUT}
        COMMAND cmake -E copy ${ASM_DEFINES_INPUT} ${ASM_DEFINES_OUTPUT}
        DEPENDS ${ASM_DEFINES_INPUT}
    )

    panda_add_library(asm_defines_process OBJECT ${ASM_DEFINES_OUTPUT})
    panda_target_compile_options(asm_defines_process PRIVATE -E -P)
    panda_target_include_directories(asm_defines_process
        PUBLIC ${CMAKE_SOURCE_DIR}
        PUBLIC ${CMAKE_BINARY_DIR}
        PUBLIC ${CMAKE_BINARY_DIR}/runtime/asm_defines
    )

    add_dependencies(cross_values_generator asm_defines_process)

    string(REPLACE " " ";" GENERATED_CROSS_VALUES_HEADERS_LIST ${GENERATED_CROSS_VALUES_HEADERS})
    add_custom_command(OUTPUT ${OUTPUT_DIR}/${OUTPUT_FILE}
        COMMAND ruby ${CMAKE_CURRENT_SOURCE_DIR}/cross_values_getters_generator.rb $<TARGET_OBJECTS:asm_defines_process> ${GENERATED_CROSS_VALUES_DIR} ${OUTPUT_DIR}/${OUTPUT_FILE}
        DEPENDS cross_values_generator ${CMAKE_CURRENT_SOURCE_DIR}/cross_values_getters_generator.rb $<TARGET_OBJECTS:asm_defines_process> ${GENERATED_CROSS_VALUES_HEADERS_LIST}
    )

    add_custom_target(cross_values
        DEPENDS ${OUTPUT_DIR}/${OUTPUT_FILE}
    )
    set(GENERATED_CROSS_VALUES_HEADERS ${GENERATED_CROSS_VALUES_HEADERS} PARENT_SCOPE)
    add_dependencies(panda_gen_files cross_values)

    if (NOT (HOST_TOOLS OR PANDA_TARGET_WINDOWS))
        # Generate `cross-values` using `asm_defines` from root-build to ensure later that they are the same as generated in `auxiliary_panda_binary_dirs`.
        # The reason for this extra-generation is to check that `auxiliary_panda_binary_dirs` are being generated correctly, e.g. all
        # of the `cmake`-flags are being passed correctly.
        set(ROOT_ASM_DIR "${CMAKE_BINARY_DIR}/runtime/asm_defines")
        set(ROOT_ASM_FILE "libasm_defines.S")
        set(OUTPUT_DIR ${GENERATED_CROSS_VALUES_DIR}/test)
        file(MAKE_DIRECTORY ${GENERATED_CROSS_VALUES_DIR}/test)
        set(OUTPUT_FILE "values_root_build_gen.h")
        set(ROOT_CROSS_VALUES ${OUTPUT_DIR}/${OUTPUT_FILE})
        add_dependencies(cross_values asm_defines_generator)
        add_custom_command(TARGET cross_values POST_BUILD
            COMMAND ruby ${CMAKE_CURRENT_SOURCE_DIR}/cross_values_generator.rb ${ROOT_ASM_DIR}/${ROOT_ASM_FILE} ${ROOT_CROSS_VALUES} ${TARGET_ARCH}
            COMMAND ${PANDA_ROOT}/cross_values/diff_check_values.sh ${TARGET_CROSS_VALUES} ${ROOT_CROSS_VALUES}
        )
    endif()
else()
    # Add mock-targets because of external dependencies on `cross_values`:
    add_custom_target(asm_defines_process)
    add_custom_target(cross_values)
    add_custom_target(cross_values_generator)
endif()

if (CMAKE_CROSSCOMPILING)
    add_dependencies(host_tools_depends cross_values)
endif()
