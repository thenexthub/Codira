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

if (NOT PANDA_BUILD_LLVM_BACKEND)
    message(FATAL_ERROR "PANDA_BUILD_LLVM_BACKEND must be true")
endif()

if (CROSS_VALUES_CONFIG)
    message(FATAL_ERROR "LLVM_BACKEND must be disabled in cross_values")
endif()

# Build our own llvm libraries
if (PANDA_BUILD_LLVM_BINARIES)

    if (NOT DEFINED PANDA_BUILD_LLVM_BINARIES_PATH_ROOT)
        message(FATAL_ERROR
        "PANDA_BUILD_LLVM_BINARIES_PATH_ROOT variable should be set, when PANDA_BUILD_LLVM_BINARIES is set"
        )
    endif()

    include(libllvmbackend/cmake/CommonDefines.cmake)
    include(ExternalProject)

    set(LLVM_SOURCES_DIR ${PANDA_THIRD_PARTY_SOURCES_DIR}/llvm-project)
    set(LLVM_SOURCES_SUBDIR ${LLVM_SOURCES_DIR}/llvm)

    set(GIT_CLONE_URL "https://gitee.com/openharmony/third_party_llvm-project.git")
    set(GIT_CLONE_BRANCH "2024_1127_llvm_ark_aot")
    # Even though ExternalProject_Add knows how to do "git clone"
    # there is problems with cloning because of
    # multiple simultaneously running configurations, so we have to do it manually
    if (NOT EXISTS ${LLVM_SOURCES_DIR})
        message(STATUS "Cloning llvm-project into ${LLVM_SOURCES_DIR}")
        execute_process(
            COMMAND git clone --depth 1 -b ${GIT_CLONE_BRANCH} ${GIT_CLONE_URL} ${LLVM_SOURCES_DIR}
        )
    endif()

    ExternalProject_Add(llvm-project
        PREFIX LLVMExternal

        USES_TERMINAL_CONFIGURE ON
        USES_TERMINAL_BUILD ON
        USES_TERMINAL_INSTALL ON

        EXCLUDE_FROM_ALL ON

        SOURCE_DIR ${LLVM_SOURCES_DIR}
    )

    set(LLVM_CMAKE_COMMON_VARIABLES -G${CMAKE_GENERATOR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DPACKAGE_VERSION=${REQUIRED_LLVM_VERSION}
        -DLLVM_ENABLE_FFI=OFF
        -DLLVM_ENABLE_TERMINFO=OFF
        -DLLVM_INCLUDE_BENCHMARKS=OFF
        -DLLVM_INCLUDE_EXAMPLES=OFF
        -DLLVM_INCLUDE_TESTS=OFF
        -DLLVM_BUILD_TOOLS=ON
        -DLLVM_BUILD_LLVM_DYLIB=ON
        -DLLVM_ENABLE_ZLIB=OFF
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    )
    set(COMMON_LLVM_COMPONENTS "cmake-exports;llvm-headers;LLVM")
    set(LLVM_COMMON_BUILD_PREFIX "${PANDA_BUILD_LLVM_BINARIES_PATH_ROOT}/BUILD")
    set(LLVM_COMMON_INSTALL_PREFIX "${PANDA_BUILD_LLVM_BINARIES_PATH_ROOT}/INSTALL")
    set(COMMON_RESULT_DIR_NAME_PREFIX "llvm-${REQUIRED_LLVM_VERSION}-${CMAKE_BUILD_TYPE}")

    # HACK: have to add this, because if this target not exist
    # LLVMExports.cmake file (which is imported in LLVMConfig.cmake)
    # trying to check that all packages are built. But we are trying to
    # use LLVMConfig.cmake at config time, and therefore there is nothing build yet
    if(NOT TARGET LLVMSupport)
        add_custom_target(LLVMSupport)
    endif()

    function(llvm_project_create_target_steps)
        set(oneValueArgs STEPS_NAME STEPS_BUILD_DIR)
        set(multiValueArgs STEPS_BUILD_DEPENDEES)
        cmake_parse_arguments(PARSE_ARGV 0 OPT "" "${oneValueArgs}" "${multiValueArgs}")

        ExternalProject_Add_Step(llvm-project ${OPT_STEPS_NAME}-build-distribution
            EXCLUDE_FROM_MAIN ON
            USES_TERMINAL ON
            DEPENDEES ${OPT_STEPS_BUILD_DEPENDEES}
            COMMAND ${CMAKE_COMMAND} --build ${OPT_STEPS_BUILD_DIR} --target distribution
        )
        ExternalProject_Add_Step(llvm-project ${OPT_STEPS_NAME}-install-distribution
            EXCLUDE_FROM_MAIN ON
            USES_TERMINAL ON
            DEPENDEES ${OPT_STEPS_NAME}-build-distribution
            BYPRODUCTS ${LIB_LLVM}
            COMMAND ${CMAKE_COMMAND} --build ${OPT_STEPS_BUILD_DIR} --target install-distribution
        )
        ExternalProject_Add_StepTargets(llvm-project ${OPT_STEPS_NAME}-install-distribution)
    endfunction(llvm_project_create_target_steps)

    function(config_llvm_and_install_package)
        set(oneValueArgs BUILD_DIR TARGET_NAME)
        set(multiValueArgs CMAKE_VARIABLES CMAKE_LLVM_COMPONENTS CMAKE_LLVM_TARGETS_TO_BUILD)
        cmake_parse_arguments(PARSE_ARGV 0 OPT "" "${oneValueArgs}" "${multiValueArgs}")
        if(EXISTS ${OPT_BUILD_DIR}/configuring_done)
            # Do not reconfigure projects on subprojects build
            # since some parameters might be changed and though
            # new parameters doesn't matter for us, our config will be broken
            message(STATUS "Configuring for ${OPT_TARGET_NAME} already done. Skip.")
            return()
        endif()
        message(STATUS "Configuring LLVM and installing LLVMConfig.cmake file for ${OPT_TARGET_NAME}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -B ${OPT_BUILD_DIR} ${OPT_CMAKE_VARIABLES}
                                     -DLLVM_DISTRIBUTION_COMPONENTS=${OPT_CMAKE_LLVM_COMPONENTS}
                                     -DLLVM_TARGETS_TO_BUILD=${OPT_CMAKE_LLVM_TARGETS_TO_BUILD}
                                     ${LLVM_SOURCES_SUBDIR}
            COMMAND_ECHO STDOUT
            COMMAND_ERROR_IS_FATAL ANY
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build ${OPT_BUILD_DIR} --target install-cmake-exports
            COMMAND_ECHO STDOUT
            COMMAND_ERROR_IS_FATAL ANY
        )
        file(TOUCH ${OPT_BUILD_DIR}/configuring_done)
    endfunction(config_llvm_and_install_package)

    # x86_64-linux-gnu (expected to be host)
    # Always register - no special dependencies, but it have parts, 
    # that other configuration depends on
    set(X86_64_RESULT_DIR ${COMMON_RESULT_DIR_NAME_PREFIX}-x86_64)
    config_llvm_and_install_package(
        BUILD_DIR ${LLVM_COMMON_BUILD_PREFIX}/${X86_64_RESULT_DIR}
        TARGET_NAME x86_64
        CMAKE_VARIABLES ${LLVM_CMAKE_COMMON_VARIABLES}
                        -DCMAKE_INSTALL_PREFIX=${LLVM_COMMON_INSTALL_PREFIX}/${X86_64_RESULT_DIR}
        CMAKE_LLVM_COMPONENTS "${COMMON_LLVM_COMPONENTS};llvm-link"
        CMAKE_LLVM_TARGETS_TO_BUILD "X86;AArch64"
    )
    if (PANDA_COMPILER_TARGET_X86_64)
        set(libllvm-dependency llvm-project-x86_64-install-distribution)
        set(LLVM_TARGET_PATH ${LLVM_COMMON_INSTALL_PREFIX}/${X86_64_RESULT_DIR})
        set(LIB_LLVM ${LLVM_TARGET_PATH}/lib/libLLVM.so)
    endif()
    llvm_project_create_target_steps(
        STEPS_NAME x86_64
        STEPS_BUILD_DIR ${LLVM_COMMON_BUILD_PREFIX}/${X86_64_RESULT_DIR}
    )
    ExternalProject_Add_Step(llvm-project x86_64-install-llvm-link
        EXCLUDE_FROM_MAIN ON
        USES_TERMINAL ON
        COMMAND ${CMAKE_COMMAND} --build ${LLVM_COMMON_BUILD_PREFIX}/${X86_64_RESULT_DIR} --target install-llvm-link
    )
    ExternalProject_Add_StepTargets(llvm-project x86_64-install-llvm-link)
    # add simple alias
    add_custom_target(x86_64-llvm-link DEPENDS llvm-project-x86_64-install-llvm-link)
    set(LLVM_LINK ${LLVM_COMMON_INSTALL_PREFIX}/${X86_64_RESULT_DIR}/bin/llvm-link)

    ExternalProject_Add_Step(llvm-project x86_64-install-llvm-tblgen
        EXCLUDE_FROM_MAIN ON
        USES_TERMINAL ON
        COMMAND ${CMAKE_COMMAND} --build ${LLVM_COMMON_BUILD_PREFIX}/${X86_64_RESULT_DIR} --target install-llvm-tblgen
    )
    # used for aarch64-linux-gnu
    ExternalProject_Add_StepTargets(llvm-project x86_64-install-llvm-tblgen)

    if (PANDA_TARGET_LINUX AND PANDA_TARGET_ARM64)
        # aarch64-linux-gnu
        set(RESULT_DIR ${COMMON_RESULT_DIR_NAME_PREFIX}-aarch64)
        set(AARCH64_CMAKE_C_FLAGS "--target=aarch64-linux-gnu\ -I/usr/aarch64-linux-gnu/include")
        set(AARCH64_CMAKE_CXX_FLAGS
            "--target=aarch64-linux-gnu\ -I/usr/aarch64-linux-gnu/include/c++/8/aarch64-linux-gnu\ -I/usr/aarch64-linux-gnu/include")
        set(LLVM_TARGET_PATH ${LLVM_COMMON_INSTALL_PREFIX}/${RESULT_DIR})
        set(LLVM_CMAKE_TARGET_VARIABLES
            -DCMAKE_INSTALL_PREFIX=${LLVM_TARGET_PATH}
            -DCMAKE_CROSSCOMPILING=ON
            -DLLVM_TARGET_ARCH=AArch64
            -DLLVM_DEFAULT_TARGET_TRIPLE=aarch64-linux-gnu
            -DCMAKE_C_FLAGS=${AARCH64_CMAKE_C_FLAGS}
            -DCMAKE_CXX_FLAGS=${AARCH64_CMAKE_CXX_FLAGS}
            -DLLVM_TABLEGEN=${LLVM_COMMON_INSTALL_PREFIX}/${X86_64_RESULT_DIR}/bin/llvm-tblgen
        )
        set(libllvm-dependency llvm-project-aarch64-install-distribution)
        set(LIB_LLVM ${LLVM_TARGET_PATH}/lib/libLLVM.so)
        llvm_project_create_target_steps(
            STEPS_NAME aarch64
            STEPS_BUILD_DIR ${LLVM_COMMON_BUILD_PREFIX}/${RESULT_DIR}
            STEPS_BUILD_DEPENDEES x86_64-install-llvm-tblgen
        )
        config_llvm_and_install_package(
            BUILD_DIR ${LLVM_COMMON_BUILD_PREFIX}/${RESULT_DIR}
            TARGET_NAME aarch64
            CMAKE_VARIABLES ${LLVM_CMAKE_COMMON_VARIABLES} ${LLVM_CMAKE_TARGET_VARIABLES}
            CMAKE_LLVM_COMPONENTS "${COMMON_LLVM_COMPONENTS};llvm-link"
            CMAKE_LLVM_TARGETS_TO_BUILD AArch64
        )
    endif()

    if (PANDA_TARGET_OHOS AND PANDA_TARGET_ARM64)
        # aarch64-linux-ohos
        if (NOT DEFINED ENV{OHOS_SDK_NATIVE})
            message(FATAL_ERROR "OHOS_SDK_NATIVE environment variable should be set to build LLVM for OHOS")
        endif()
        set(RESULT_DIR ${COMMON_RESULT_DIR_NAME_PREFIX}-ohos)
        set(LLVM_TARGET_PATH ${LLVM_COMMON_INSTALL_PREFIX}/${RESULT_DIR})
        set(LLVM_CMAKE_TARGET_VARIABLES
            -DCMAKE_TOOLCHAIN_FILE=$ENV{OHOS_SDK_NATIVE}/build/cmake/ohos.toolchain.cmake
            -DOHOS_ALLOW_UNDEFINED_SYMBOLS=ON
            -DLLVM_DEFAULT_TARGET_TRIPLE=aarch64-linux-ohos
            -DCMAKE_INSTALL_PREFIX=${LLVM_TARGET_PATH}
        )
        set(libllvm-dependency llvm-project-ohos-install-distribution)
        set(LIB_LLVM ${LLVM_TARGET_PATH}/lib/libLLVM.so)
        llvm_project_create_target_steps(
            STEPS_NAME ohos
            STEPS_BUILD_DIR ${LLVM_COMMON_BUILD_PREFIX}/${RESULT_DIR}
        )
        config_llvm_and_install_package(
            BUILD_DIR ${LLVM_COMMON_BUILD_PREFIX}/${RESULT_DIR}
            TARGET_NAME ohos
            CMAKE_VARIABLES ${LLVM_CMAKE_COMMON_VARIABLES} ${LLVM_CMAKE_TARGET_VARIABLES}
            CMAKE_LLVM_COMPONENTS "${COMMON_LLVM_COMPONENTS}"
            CMAKE_LLVM_TARGETS_TO_BUILD AArch64
        )
    endif()
    message(STATUS "libLLVM depends on \"${libllvm-dependency}\"")
endif()

find_package(LLVM 15 REQUIRED CONFIG NO_DEFAULT_PATH CMAKE_FIND_ROOT_PATH_BOTH PATHS ${LLVM_TARGET_PATH})
message(STATUS "LLVM backend:")
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# use prebuilds
if (NOT PANDA_BUILD_LLVM_BINARIES)
    find_library(LIB_LLVM LLVM REQUIRED NO_DEFAULT_PATH CMAKE_FIND_ROOT_PATH_BOTH PATHS ${LLVM_LIBRARY_DIR})

    if (NOT CMAKE_CROSSCOMPILING AND PANDA_LLVM_INTERPRETER_INLINING)
        find_program(LLVM_LINK NAMES llvm-link REQUIRED NO_DEFAULT_PATH CMAKE_FIND_ROOT_PATH_BOTH PATHS "${LLVM_BINARY_DIR}/bin")
    endif()
endif()

message(STATUS "LLVM_LINK ${LLVM_LINK}")
message(STATUS "LIB_LLVM ${LIB_LLVM}")
set(LLVM_COPY_NAME "libLLVM-${LLVM_VERSION_MAJOR}.so")
add_custom_target(copy-libLLVM.so
    COMMENT "Copying ${LIB_LLVM} into ${PANDA_BINARY_ROOT}/lib/${LLVM_COPY_NAME}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${LIB_LLVM}" "${PANDA_BINARY_ROOT}/lib/${LLVM_COPY_NAME}"
    DEPENDS ${libllvm-dependency})
