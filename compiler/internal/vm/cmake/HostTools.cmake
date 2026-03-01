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
        set(ARCH_NAME X86_64)
        set(IRTOC_TARGET x86_64)
    elseif(PANDA_TARGET_ARM64)
        set(ARCH_NAME AARCH64)
        set(IRTOC_TARGET arm64)
    elseif(PANDA_TARGET_ARM32)
        set(ARCH_NAME AARCH32)
        set(IRTOC_TARGET arm32)
    else()
        message(FATAL_ERROR "Unsupported target")
    endif()

    add_custom_target(host_tools_depends)

    if (PANDA_TARGET_ARM64)
        set(TARGET_ARCH "AARCH64")
    elseif (PANDA_TARGET_ARM32)
        set(TARGET_ARCH "AARCH32")
    else()
        set(TARGET_ARCH "UNKNOWN")
    endif()

    set(HOST_TOOLS_CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DPANDA_ROOT=${PANDA_ROOT}
        -DPANDA_BINARY_ROOT=${PANDA_BINARY_ROOT}
        -DPANDA_ROOT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DPANDA_THIRD_PARTY_SOURCES_DIR=${PANDA_THIRD_PARTY_SOURCES_DIR}
        -DPANDA_THIRD_PARTY_CONFIG_DIR=${PANDA_THIRD_PARTY_CONFIG_DIR}
        -DPANDA_PRODUCT_BUILD=true
        -DPANDA_WITH_JAVA=${PANDA_WITH_JAVA}
        -DPANDA_TARGET_MOBILE_WITH_MANAGED_LIBS=${PANDA_TARGET_MOBILE_WITH_MANAGED_LIBS}
        -DPANDA_WITH_ECMASCRIPT=${PANDA_WITH_ECMASCRIPT}
        -DPANDA_WITH_ACCORD=${PANDA_WITH_ACCORD}
        -DPANDA_WITH_CODIRA=${PANDA_WITH_CODIRA}
        -DIRTOC_TARGET=${IRTOC_TARGET}
        -DPANDA_ENABLE_ADDRESS_SANITIZER=${PANDA_ENABLE_ADDRESS_SANITIZER}
        -DPANDA_ENABLE_THREAD_SANITIZER=${PANDA_ENABLE_THREAD_SANITIZER}
        -DPANDA_ENABLE_CLANG_TIDY=false
        -DPANDA_WITH_BYTECODE_OPTIMIZER=false
        -DPANDA_WITH_TESTS=false
        -DHOST_TOOLS=true
        -DPANDA_HOST_TOOLS_TARGET_TOOLCHAIN=${CMAKE_TOOLCHAIN_FILE}
        -DPANDA_HOST_TOOLS_TARGET_ARCH=${TARGET_ARCH}
        -DTOOLCHAIN_CLANG_ROOT=${TOOLCHAIN_CLANG_ROOT}
        -DTOOLCHAIN_SYSROOT=${TOOLCHAIN_SYSROOT}
        -DMOBILE_NATIVE_LIBS_SOURCE_PATH=${MOBILE_NATIVE_LIBS_SOURCE_PATH}
        -DPANDA_TARGET_ARM32_ABI_HARD=${PANDA_TARGET_ARM32_ABI_HARD}
        -DPANDA_RELEASE_BUILD=${PANDA_RELEASE_BUILD}
        --no-warn-unused-cli
    )

    set(HOST_TOOLS_TARGETS
        cross_values
        irtoc_fastpath
        irtoc_interpreter
        irtoc_tests
        ark_asm
        c_secshared
        zlib
    )

    add_custom_target(irtoc_fastpath)
    add_custom_target(irtoc_interpreter)
    add_custom_target(irtoc_tests)

    if(PANDA_WITH_JAVA)
        list(APPEND HOST_TOOLS_TARGETS c2p irtoc_java_fastpath)
        if(DEFINED PANDA_JAVA_LIBCORE_VERSION)
            list(APPEND HOST_TOOLS_CMAKE_ARGS -DPANDA_JAVA_LIBCORE_VERSION=${PANDA_JAVA_LIBCORE_VERSION})
        endif()
    endif()

    if (PANDA_WITH_ECMASCRIPT)
        list(APPEND HOST_TOOLS_TARGETS es2panda)
    endif()

    set(BUILD_COMMAND)

    # After CMake 3.15 all targets can be specified in a single `--target` option. But we use CMake 3.10 at the moment.
    foreach(target ${HOST_TOOLS_TARGETS})
        string(APPEND BUILD_COMMAND "${CMAKE_COMMAND} --build ${host_tools_build_dir} --target ${target} -- -j${PANDA_HOST_TOOLS_JOBS_NUMBER} && ")
    endforeach()
    string(APPEND BUILD_COMMAND " true")
    separate_arguments(BUILD_COMMAND UNIX_COMMAND ${BUILD_COMMAND})

    # Ensuring that cross-values from host-tools and from the main project are the same:
    set(TARGET_CROSS_VALUES ${PANDA_BINARY_ROOT}/cross_values/generated_values/${ARCH_NAME}_values_gen.h)
    set(HOST_CROSS_VALUES ${host_tools_build_dir}/cross_values/generated_values/${ARCH_NAME}_values_gen.h)
    set(CHECK_COMMAND "${PANDA_ROOT}/cross_values/diff_check_values.sh ${TARGET_CROSS_VALUES} ${HOST_CROSS_VALUES}")
    separate_arguments(CHECK_COMMAND UNIX_COMMAND ${CHECK_COMMAND})

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
    )

    add_custom_target(build_host_tools
        COMMAND           ${BUILD_COMMAND}
        COMMAND           ${CHECK_COMMAND}
        WORKING_DIRECTORY ${host_tools_build_dir}
        DEPENDS           panda_host_tools
    )
endfunction()

panda_configure_host_tools()
