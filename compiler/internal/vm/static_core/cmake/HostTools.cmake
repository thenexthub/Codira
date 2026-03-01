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

# To build parts of the platform written in Panda Assembly,
# we need to have the assembler binary. If we are not
# cross-compiling, we are good to go. Otherwise a subset
# of source tree is exposed as a separate project for
# "host tools", which is built solely for host.

function(panda_configure_host_tools)
    if(NOT CMAKE_CROSSCOMPILING)
        return()
    endif()

    include(ExternalProject)

    set(host_tools_build_dir "${CMAKE_CURRENT_BINARY_DIR}/host-tools-build")


    if ($ENV{NPROC_PER_JOB})
        set(PANDA_HOST_TOOLS_JOBS_NUMBER $ENV{NPROC_PER_JOB})
    else()
        set(PANDA_HOST_TOOLS_JOBS_NUMBER 16)
    endif()
    if (PANDA_TARGET_AMD64)
        set(TARGET_ARCH X86_64)
        set(IRTOC_TARGET x86_64)
    elseif(PANDA_TARGET_ARM64)
        set(TARGET_ARCH AARCH64)
        set(IRTOC_TARGET arm64)
    elseif(PANDA_TARGET_ARM32)
        set(TARGET_ARCH AARCH32)
        set(IRTOC_TARGET arm32)
    else()
        message(FATAL_ERROR "Unsupported target")
    endif()

    add_custom_target(host_tools_depends)

    set(HOST_TOOLS_CMAKE_ARGS
        -DPANDA_USE_64_BIT_POINTER=${PANDA_USE_64_BIT_POINTER}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DPANDA_ROOT=${PANDA_ROOT}
        -DPANDA_BINARY_ROOT=${PANDA_BINARY_ROOT}
        -DPANDA_ROOT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DPANDA_THIRD_PARTY_SOURCES_DIR=${PANDA_THIRD_PARTY_SOURCES_DIR}
        -DPANDA_THIRD_PARTY_CONFIG_DIR=${PANDA_THIRD_PARTY_CONFIG_DIR}
        -DPANDA_PRODUCT_BUILD=${PANDA_PRODUCT_BUILD}
        -DPANDA_TARGET_MOBILE_WITH_MANAGED_LIBS=${PANDA_TARGET_MOBILE_WITH_MANAGED_LIBS}
        -DPANDA_TARGET_CPU_FEATURES=${PANDA_TARGET_CPU_FEATURES}
        -DIRTOC_TARGET=${IRTOC_TARGET}
        -DPANDA_ENABLE_ADDRESS_SANITIZER=${PANDA_ENABLE_ADDRESS_SANITIZER}
        -DPANDA_ENABLE_THREAD_SANITIZER=${PANDA_ENABLE_THREAD_SANITIZER}
        -DPANDA_WITH_BYTECODE_OPTIMIZER=true
        -DPANDA_LLVM_BACKEND=${PANDA_LLVM_BACKEND}
        -DPANDA_LLVM_INTERPRETER=${PANDA_LLVM_INTERPRETER}
        -DPANDA_LLVM_FASTPATH=${PANDA_LLVM_FASTPATH}
        -DPANDA_BUILD_LLVM_BINARIES=${PANDA_BUILD_LLVM_BINARIES}
        -DPANDA_BUILD_LLVM_BINARIES_PATH_ROOT=${PANDA_BUILD_LLVM_BINARIES_PATH_ROOT}
        # LLVM AOT is used only by target build, cross-tools do not compile anything by llvm aot
        -DPANDA_LLVM_AOT=false
        -DLLVM_TARGET_PATH=${LLVM_HOST_PATH}
        -DPANDA_WITH_TESTS=false
        -DHOST_TOOLS=true
        -DPANDA_HOST_TOOLS_TARGET_TOOLCHAIN=${CMAKE_TOOLCHAIN_FILE}
        -DPANDA_HOST_TOOLS_TARGET_ARCH=${TARGET_ARCH}
        -DTOOLCHAIN_CLANG_ROOT=${TOOLCHAIN_CLANG_ROOT}
        # The file contains list of bitcode files to create inline module
        -DIRTOC_INT_LLVM_INL_MODULE_FILES_LIST=${IRTOC_INT_LLVM_INL_MODULE_FILES_LIST}
        -DPANDA_LLVM_INTERPRETER_INLINING=${PANDA_LLVM_INTERPRETER_INLINING}
        -DTOOLCHAIN_SYSROOT=${TOOLCHAIN_SYSROOT}
        -DANDROID_ABI=${ANDROID_ABI}
        -DANDROID_PLATFORM=${ANDROID_PLATFORM}
        -DMOBILE_NATIVE_LIBS_SOURCE_PATH=${MOBILE_NATIVE_LIBS_SOURCE_PATH}
        -DPANDA_TARGET_ARM32_ABI_HARD=${PANDA_TARGET_ARM32_ABI_HARD}
        -DPANDA_MINIMAL_VIXL=false
        --no-warn-unused-cli
    )

    set(HOST_TOOLS_TARGETS
        cross_values
        irtoc_fastpath
        irtoc_interpreter
        irtoc_tests
        ark_asm
        arkbytecodeopt
        arkcompiler
        c_secshared
        zlib
        aspt_converter
    )

    set(HOST_TOOLS_BYPRODUCTS
        "${host_tools_build_dir}/irtoc/irtoc_fastpath/irtoc_fastpath.o"
        "${host_tools_build_dir}/irtoc/irtoc_fastpath/irtoc_fastpath_llvm.o"
        "${host_tools_build_dir}/irtoc/irtoc_interpreter/irtoc_interpreter.o"
        "${host_tools_build_dir}/irtoc/irtoc_interpreter/irtoc_interpreter_llvm.o"
    )

    if (PANDA_WITH_QUICKENER)
        set(HOST_TOOLS_TARGETS ${HOST_TOOLS_TARGETS} arkquick)
    endif()

    add_custom_target(irtoc_fastpath)
    add_custom_target(irtoc_interpreter)
    add_custom_target(irtoc_tests)

    if(EXISTS ${ES2PANDA_PATH})
        list(APPEND HOST_TOOLS_TARGETS es2panda)
    endif()

    foreach(plugin ${PLUGINS})
        string(TOUPPER ${plugin} plugin_name_upper)
        string(CONCAT PANDA_WITH_PLUGIN "PANDA_WITH_" ${plugin_name_upper})
        list(APPEND HOST_TOOLS_CMAKE_ARGS -D${PANDA_WITH_PLUGIN}=${${PANDA_WITH_PLUGIN}})
        if(${${PANDA_WITH_PLUGIN}})
            string(CONCAT PLUGIN_SOURCE "PANDA_" ${plugin_name_upper} "_PLUGIN_SOURCE")
            if(EXISTS ${${PLUGIN_SOURCE}}/HostTools.cmake)
                include(${${PLUGIN_SOURCE}}/HostTools.cmake)
            endif()
        endif()
    endforeach()

    set(BUILD_COMMAND)

    if(PANDA_USE_PREBUILT_TARGETS)
        # Skip building when we use brebuilt targets, because we already have all needed binaries unstashed in host_tools_build_dir
        set(BUILD_COMMAND "${CMAKE_COMMAND} -E echo Skipping build step")
    else()
        # After CMake 3.15 all targets can be specified in a single `--target` option. But we use CMake 3.10 at the moment.
        foreach(target ${HOST_TOOLS_TARGETS})
            string(APPEND BUILD_COMMAND "${CMAKE_COMMAND} --build ${host_tools_build_dir} --target ${target} -- -j${PANDA_HOST_TOOLS_JOBS_NUMBER} && ")
        endforeach()
        string(APPEND BUILD_COMMAND " true")
    endif()
    separate_arguments(BUILD_COMMAND UNIX_COMMAND ${BUILD_COMMAND})

    # Ensuring that cross-values from host-tools and from the main project are the same:
    set(TARGET_CROSS_VALUES ${PANDA_BINARY_ROOT}/cross_values/generated_values/${TARGET_ARCH}_values_gen.h)
    set(HOST_CROSS_VALUES ${host_tools_build_dir}/cross_values/generated_values/${TARGET_ARCH}_values_gen.h)
    set(CHECK_COMMAND "${PANDA_ROOT}/cross_values/diff_check_values.sh ${TARGET_CROSS_VALUES} ${HOST_CROSS_VALUES}")
    separate_arguments(CHECK_COMMAND UNIX_COMMAND ${CHECK_COMMAND})

    message(STATUS "Host-tools configure args: '${HOST_TOOLS_CMAKE_ARGS}'")

    # In case of dependency on panda_host_tools it's mandatory to use the panda_host_tools-build target as the dependency target
    ExternalProject_Add(panda_host_tools
        DEPENDS           host_tools_depends
        SOURCE_DIR        "${PANDA_ROOT}"
        BINARY_DIR        "${host_tools_build_dir}"
        BUILD_IN_SOURCE   FALSE
        CONFIGURE_COMMAND ${CMAKE_COMMAND} <SOURCE_DIR> -G "${CMAKE_GENERATOR}"
                          ${HOST_TOOLS_CMAKE_ARGS}
        BUILD_COMMAND     ${BUILD_COMMAND}
        INSTALL_COMMAND   ${CMAKE_COMMAND} -E echo "Skipping install step"
        TEST_COMMAND      ${CHECK_COMMAND}
        BUILD_BYPRODUCTS  ${HOST_TOOLS_BYPRODUCTS}
        BUILD_ALWAYS      TRUE
        STEP_TARGETS      build
    )
endfunction()

panda_configure_host_tools()
