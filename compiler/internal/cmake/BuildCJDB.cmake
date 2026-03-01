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

externalproject_get_property(codenative BINARY_DIR)
set(LLVM_GC_BINARY_DIR "${BINARY_DIR}")
find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(LLVM_GC_LLDB_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/third_party/lldb")
string(REPLACE ";" "|" CMAKE_PREFIX_PATH_ALT_SEP "${CMAKE_PREFIX_PATH}")
set(CMAKE_PREFIX_PATH_ALT_SEP "${CMAKE_PREFIX_PATH_ALT_SEP}|${CMAKE_BINARY_DIR}/third_party/xml2")
set(LLDB_CMAKE_ARGS
    # Hide warnings during configure which are intended for VM developers.
    -Wno-dev
    -DCMAKE_BUILD_TYPE=${CODIRA_CODEDB_BUILD_TYPE}
    -DCMAKE_C_COMPILER=${LLVM_BUILD_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${LLVM_BUILD_CXX_COMPILER}
    -DCMAKE_CXX_STANDARD=17
    -DCMAKE_INSTALL_PREFIX=${LLVM_GC_LLDB_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_ALT_SEP}
    -DLLVM_ENABLE_PROJECTS=lldb
    -DLLVM_TARGETS_TO_BUILD=AArch64|X86
    -DLLVM_DIR=${LLVM_GC_BINARY_DIR}/lib/cmake/llvm
    -DLLVM_EXTERNAL_LIT=${LLVM_GC_BINARY_DIR}/bin/llvm-lit${CMAKE_EXECUTABLE_SUFFIX}
    -DLLVM_ENABLE_RTTI=ON)

set(LLDB_CMAKE_EXE_LINKER_FLAGS "-fstack-protector-strong")
set(LLDB_CMAKE_SHARED_LINKER_FLAGS "-fstack-protector-strong")
if(WIN32)
    list(APPEND LLDB_CMAKE_ARGS -DLLDB_ENABLE_LIBEDIT=OFF)
    set(LLDB_CMAKE_C_FLAGS "${LLDB_CMAKE_C_FLAGS} -fstack-protector-strong")
    set(LLDB_CMAKE_CXX_FLAGS "${LLDB_CMAKE_CXX_FLAGS} -fstack-protector-strong")
    set(LLDB_CMAKE_EXE_LINKER_FLAGS "${LLDB_CMAKE_EXE_LINKER_FLAGS} -Wl,--no-insert-timestamp")
    set(LLDB_CMAKE_SHARED_LINKER_FLAGS "${LLDB_CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-insert-timestamp")
else()
    if(DARWIN)
        if(NOT CODIRA_BUILD_CODEDB_DISABLE_PYTHON)
            set(TARGET_PYTHON_PATH $ENV{TARGET_PYTHON_PATH})
            if (TARGET_PYTHON_PATH)
                list(APPEND LLDB_CMAKE_ARGS -DPython3_EXECUTABLE=${TARGET_PYTHON_PATH}/bin/python3)
                list(APPEND LLDB_CMAKE_ARGS -DLLDB_PYTHON_EXE_RELATIVE_PATH=bin/python3)
            endif()
        endif()
        list(APPEND LLDB_CMAKE_ARGS -DLLDB_INCLUDE_TESTS=OFF)
        set(LLDB_CMAKE_INSTALL_RPATH "@loader_path/../lib")
    else()
        set(LLDB_CMAKE_C_FLAGS "${LLDB_CMAKE_C_FLAGS} ${GCC_TOOLCHAIN_FLAG} -fstack-protector-strong -D_FORTIFY_SOURCE=2")
        set(LLDB_CMAKE_CXX_FLAGS "${LLDB_CMAKE_CXX_FLAGS} ${GCC_TOOLCHAIN_FLAG} -fstack-protector-strong -ftrapv -fpie -D_FORTIFY_SOURCE=2")
        set(LLDB_CMAKE_EXE_LINKER_FLAGS "${LLDB_CMAKE_EXE_LINKER_FLAGS} -fpie -Wl,-z,relro,-z,now,-z,noexecstack")
        set(LLDB_CMAKE_SHARED_LINKER_FLAGS "${LLDB_CMAKE_SHARED_LINKER_FLAGS} -fpie -Wl,-z,relro,-z,now,-z,noexecstack")
        set(LLDB_CMAKE_INSTALL_RPATH "\\$ORIGIN/../lib")
        if(CODIRA_CODEDB_BUILD_TYPE MATCHES "^(Release|MinSizeRel)$")
            set(LLDB_CMAKE_CXX_FLAGS "${LLDB_CMAKE_CXX_FLAGS} -O2")
            set(LLDB_CMAKE_EXE_LINKER_FLAGS "${LLDB_CMAKE_EXE_LINKER_FLAGS} -s")
            set(LLDB_CMAKE_SHARED_LINKER_FLAGS "${LLDB_CMAKE_SHARED_LINKER_FLAGS} -s")
        endif()
    endif()
    if (OHOS)
        list(APPEND LLDB_CMAKE_ARGS ${LLVM_BUILD_ARG}
            -DLLDB_ENABLE_PYTHON=OFF
            -DLLDB_INCLUDE_TESTS=OFF
            -DLLDB_ENABLE_LIBEDIT=OFF
            -DLLVM_ENABLE_TERMINFO=OFF
            -DNATIVE_Clang_DIR=${LLVM_GC_BINARY_DIR}/NATIVE/tools/bin
            -DNATIVE_LLVM_DIR=${LLVM_GC_BINARY_DIR}/NATIVE/tools/bin
            -DCODIRA_TARGET_TOOLCHAIN=${CODIRA_TARGET_TOOLCHAIN}
            -DCODIRA_TARGET_SYSROOT=${CODIRA_TARGET_SYSROOT}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/cmake/aarch64-linux-ohos_toolchain.cmake
        )
    else()
        list(APPEND LLDB_CMAKE_ARGS ${LLVM_BUILD_ARG}
            -DLLDB_ENABLE_LIBEDIT=1
            -DLLVM_ENABLE_TERMINFO=1
            -DCURSES_NEED_NCURSES=1)
    endif()
endif()

if(NOT "${LLDB_CMAKE_C_FLAGS}" STREQUAL "")
    list(APPEND LLDB_CMAKE_ARGS -DCMAKE_C_FLAGS=${LLDB_CMAKE_C_FLAGS})
endif()
if(NOT "${LLDB_CMAKE_CXX_FLAGS}" STREQUAL "")
    list(APPEND LLDB_CMAKE_ARGS -DCMAKE_CXX_FLAGS=${LLDB_CMAKE_CXX_FLAGS})
endif()
list(APPEND LLDB_CMAKE_ARGS -DCMAKE_EXE_LINKER_FLAGS=${LLDB_CMAKE_EXE_LINKER_FLAGS})
list(APPEND LLDB_CMAKE_ARGS -DCMAKE_SHARED_LINKER_FLAGS=${LLDB_CMAKE_SHARED_LINKER_FLAGS})
if(NOT "${LLDB_CMAKE_INSTALL_RPATH}" STREQUAL "")
    list(APPEND LLDB_CMAKE_ARGS -DCMAKE_INSTALL_RPATH=${LLDB_CMAKE_INSTALL_RPATH})
endif()

set(CODIRA_FRONTEND_LIB ${CMAKE_BINARY_DIR}/lib/libcodira-frontend${CMAKE_SHARED_LIBRARY_SUFFIX})
set(CODIRA_LSP_LIB ${CMAKE_BINARY_DIR}/lib/libcodira-lsp${CMAKE_SHARED_LIBRARY_SUFFIX})
if(WIN32)
    set(CODIRA_FRONTEND_LIB ${CMAKE_BINARY_DIR}/bin/libcodira-frontend.dll)
    set(CODIRA_LSP_LIB ${CMAKE_BINARY_DIR}/bin/libcodira-lsp.dll)
endif()
set(CODIRA_FRONTEND_IMPLIB ${CMAKE_BINARY_DIR}/lib/libcodira-frontend.dll.a)
set(CODIRA_LSP_IMPLIB ${CMAKE_BINARY_DIR}/lib/libcodira-lsp.dll.a)

if (DARWIN)
    execute_process(
        COMMAND ${TARGET_PYTHON_PATH}/bin/python3 -c "import sys; print(f'{sys.version_info[0]}.{sys.version_info[1]}')"
        OUTPUT_VARIABLE TARGET_PATHON_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    if(Python3_VERSION MATCHES "([0-9]+)\\.([0-9]+)\\.([0-9]+)")
        set(MAJOR_VERSION "${CMAKE_MATCH_1}")
        set(MINOR_VERSION "${CMAKE_MATCH_2}")
        set(TARGET_PATHON_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}")
    endif()
endif()

if (NOT OHOS)
    if (CODIRA_BUILD_CODEDB_DISABLE_PYTHON)
        list(APPEND LLDB_CMAKE_ARGS
            -DLLDB_ENABLE_PYTHON=OFF
            -DLLDB_RELOCATABLE_PYTHON=OFF
            -DLLDB_EMBED_PYTHON_HOME=OFF
            -DLLDB_INCLUDE_TESTS=OFF
        )
    else()
        list(APPEND LLDB_CMAKE_ARGS
            -DLLDB_ENABLE_PYTHON=ON
            -DLLDB_RELOCATABLE_PYTHON=ON
            -DLLDB_EMBED_PYTHON_HOME=OFF
            -DLLDB_PYTHON_RELATIVE_PATH=lib/python${TARGET_PATHON_VERSION}/site-packages
            -DLLDB_INCLUDE_TESTS=OFF
        )
    endif()
endif()

list(APPEND LLDB_CMAKE_ARGS
    -DLLDB_ENABLE_LZMA=false
    -DLLDB_ENABLE_LIBXML2=true
    -DCODIRA_ROOT=${CMAKE_SOURCE_DIR}
    -DCODIRA_FRONTEND_LIB=${CODIRA_FRONTEND_LIB}
    -DCODIRA_FRONTEND_IMPLIB=${CODIRA_FRONTEND_IMPLIB}
    -DCODIRA_LSP_LIB=${CODIRA_LSP_LIB}
    -DCODIRA_LSP_IMPLIB=${CODIRA_LSP_IMPLIB})

if(CMAKE_CROSSCOMPILING AND WIN32)
    set(MINGW_STRIP "x86_64-w64-mingw32-strip -D")
    if (NOT CODIRA_BUILD_CODEDB_DISABLE_PYTHON)
        set(TARGET_PYTHON_PATH $ENV{TARGET_PYTHON_PATH})
        list(APPEND LLDB_CMAKE_ARGS
            -DPython3_EXECUTABLE=${Python3_EXECUTABLE}
            -DLLDB_PYTHON_EXE_RELATIVE_PATH=python.exe
            -DLLDB_PYTHON_EXT_SUFFIX=.pyd
            -DPython3_INCLUDE_DIRS=${TARGET_PYTHON_PATH}/include/python${MAJOR_VERSION}.${MINOR_VERSION}
            -DPython3_LIBRARIES=${TARGET_PYTHON_PATH}/python${MAJOR_VERSION}${MINOR_VERSION}.dll
        )
    endif()
    list(APPEND LLDB_CMAKE_ARGS -DCMAKE_STRIP=${MINGW_STRIP})
    list(APPEND LLDB_CMAKE_ARGS -DCMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR})
    list(APPEND LLDB_CMAKE_ARGS -DCMAKE_SYSTEM_NAME=Windows)
    list(APPEND LLDB_CMAKE_ARGS -DLLVM_HOST_TRIPLE=x86_64-w64-mingw32)
    list(APPEND LLDB_CMAKE_ARGS -DNATIVE_Clang_DIR=${LLVM_GC_BINARY_DIR}/NATIVE/tools/bin)
    list(APPEND LLDB_CMAKE_ARGS -DNATIVE_LLVM_DIR=${LLVM_GC_BINARY_DIR}/NATIVE/tools/bin)
    list(APPEND LLDB_CMAKE_ARGS -DLLDB_INCLUDE_TESTS=OFF)
    set(NATIVE_BUILD_CMAKE_ARGS -Wno-dev|-Wno-deprecated|-DLLDB_INCLUDE_TESTS=OFF) 
    list(APPEND LLDB_CMAKE_ARGS -DCROSS_TOOLCHAIN_FLAGS_lldb_NATIVE=${NATIVE_BUILD_CMAKE_ARGS})
endif()

if(CMAKE_HOST_WIN32)
    list(APPEND LLDB_CMAKE_ARGS -DLLVM_PARALLEL_LINK_JOBS=1)
endif()

externalproject_get_property(codenative SOURCE_DIR)
ExternalProject_Add(
    lldb
    SOURCE_DIR ${SOURCE_DIR}
    DOWNLOAD_COMMAND ""
    BUILD_BYPRODUCTS ${LLVM_GC_LLDB_INSTALL_PREFIX}/bin/lldb
    SOURCE_SUBDIR lldb
    BINARY_DIR ${CMAKE_BINARY_DIR}/third_party/lldb-build
    LIST_SEPARATOR |
    CMAKE_ARGS ${LLDB_CMAKE_ARGS}
    USES_TERMINAL_BUILD ON
    DEPENDS codenative)
# Install lldb binaries, lldb, lldb-server, etc.
install(
    DIRECTORY ${LLVM_GC_LLDB_INSTALL_PREFIX}/bin
    USE_SOURCE_PERMISSIONS
    DESTINATION third_party/llvm)

#Install lldb python modules
if(NOT CODIRA_BUILD_CODEDB_DISABLE_PYTHON)
    if(CMAKE_CROSSCOMPILING AND WIN32)
        install(
            DIRECTORY ${LLVM_GC_LLDB_INSTALL_PREFIX}/lib/python${TARGET_PATHON_VERSION}
            DESTINATION tools/lib
            USE_SOURCE_PERMISSIONS
            PATTERN ${LLVM_GC_LLDB_INSTALL_PREFIX}/lib/python${TARGET_PATHON_VERSION}/site-packages/lldb/lldb-argdumper.exe EXCLUDE
        )
    elseif(NOT OHOS)
        install(
            DIRECTORY ${LLVM_GC_LLDB_INSTALL_PREFIX}/lib/python${TARGET_PATHON_VERSION}
            DESTINATION third_party/llvm/lib
            USE_SOURCE_PERMISSIONS
        )
        if(CODIRA_INSTALL_CODEDB_SCRIPT)
            install(
                FILES ${SOURCE_DIR}/lldb/source/Plugins/Platform/MacOSX/codira_codedb.py
                DESTINATION tools/script/)
        endif()
    endif()
endif()

# For Windows, dlls are installed in bin directory. Import libraries are not required.
if(DARWIN)
    install(FILES
        ${CMAKE_BINARY_DIR}/third_party/xml2/lib/libxml2.dylib
        ${CMAKE_BINARY_DIR}/third_party/xml2/lib/libxml2.16.dylib
        ${CMAKE_BINARY_DIR}/third_party/xml2/lib/libxml2.2.14.0.dylib
        DESTINATION third_party/llvm/lib)
    # Install lldb libraries, liblldb.so, liblldb.so.15, etc. liblldbIntelFeatures.so is not installed.
    install(
        DIRECTORY ${LLVM_GC_LLDB_INSTALL_PREFIX}/lib
        DESTINATION third_party/llvm
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING
            REGEX "lib/liblldb")
elseif(NOT WIN32)
    install(FILES
        ${CMAKE_BINARY_DIR}/third_party/xml2/lib/libxml2.so
        ${CMAKE_BINARY_DIR}/third_party/xml2/lib/libxml2.so.16
        ${CMAKE_BINARY_DIR}/third_party/xml2/lib/libxml2.so.2.14.0
        DESTINATION third_party/llvm/lib)
    # Install lldb libraries, liblldb.so, liblldb.so.15, etc. liblldbIntelFeatures.so is not installed.
    install(
        DIRECTORY ${LLVM_GC_LLDB_INSTALL_PREFIX}/lib
        DESTINATION third_party/llvm
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING
            REGEX "lib/liblldb")
endif()
# Generate binary codedb and its dependencies. A symbolic link of lldb will be created if it's possible,
# otherwise a copy of lldb will be created.
if(WIN32)
    install(FILES
        ${CMAKE_BINARY_DIR}/third_party/xml2/bin/libxml2.dll
        DESTINATION tools/bin/)
    install(
        PROGRAMS ${CMAKE_BINARY_DIR}/third_party/lldb/bin/lldb${CMAKE_EXECUTABLE_SUFFIX}
        DESTINATION tools/bin/
        RENAME codedb${CMAKE_EXECUTABLE_SUFFIX})
    install(FILES
        ${CMAKE_BINARY_DIR}/third_party/llvm/bin/libclang.dll
        ${CMAKE_BINARY_DIR}/third_party/llvm/bin/libclang-cpp.dll
        ${CMAKE_BINARY_DIR}/third_party/llvm/bin/libLTO.dll
        ${CMAKE_BINARY_DIR}/third_party/llvm/bin/libLLVM-15.dll
        ${CMAKE_BINARY_DIR}/third_party/llvm/bin/libLLVM-Foundation-15.dll
        ${CMAKE_BINARY_DIR}/third_party/lldb/bin/liblldb.dll
        DESTINATION tools/bin/)
else()
    # Create symbolic link for codedb
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/tools/bin/codedb${CMAKE_EXECUTABLE_SUFFIX}
        COMMAND ${CMAKE_COMMAND}
            -D CMAKE_CROSSCOMPILING=${CMAKE_CROSSCOMPILING}
            -D WIN32=OFF
            -D LINK_TARGET=../../third_party/llvm/bin/lldb${CMAKE_EXECUTABLE_SUFFIX}
            -D LINK_NAME=codedb${CMAKE_EXECUTABLE_SUFFIX}
            -D WORKING_DIR=${CMAKE_BINARY_DIR}/tools/bin
            -P "${CMAKE_SOURCE_DIR}/cmake/modules/make-symlink.cmake"
        DEPENDS codenative)
    add_custom_target(
        codedb ALL
        DEPENDS ${CMAKE_BINARY_DIR}/tools/bin/codedb${CMAKE_EXECUTABLE_SUFFIX}
        COMMENT "Making codedb${CMAKE_EXECUTABLE_SUFFIX}")
    install(
        CODE "execute_process(COMMAND ${CMAKE_COMMAND}
        -D CMAKE_CROSSCOMPILING=${CMAKE_CROSSCOMPILING}
        -D WIN32=OFF
        -D LINK_TARGET=../../third_party/llvm/bin/lldb${CMAKE_EXECUTABLE_SUFFIX}
        -D LINK_NAME=codedb${CMAKE_EXECUTABLE_SUFFIX}
        -D WORKING_DIR=${CMAKE_INSTALL_PREFIX}/tools/bin
        -P ${CMAKE_SOURCE_DIR}/cmake/modules/make-symlink.cmake)")
    # Create symbolic link for boundscheck
    set(CODENATIVE_BACKEND "codenative")
    if (OHOS)
        set(SYS_NAME "_ohos")
    endif()
    string(TOLOWER ${CMAKE_SYSTEM_NAME}${SYS_NAME}_${CMAKE_SYSTEM_PROCESSOR}_${CODENATIVE_BACKEND} output_lib_dir)
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/third_party/lldb/lib/libboundscheck${CMAKE_SHARED_LIBRARY_SUFFIX}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/runtime/lib/${output_lib_dir}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/lib/libboundscheck${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_BINARY_DIR}/runtime/lib/${output_lib_dir}
        COMMAND ${CMAKE_COMMAND}
            -D CMAKE_CROSSCOMPILING=${CMAKE_CROSSCOMPILING}
            -D WIN32=OFF
            -D LINK_TARGET=../../../runtime/lib/${output_lib_dir}/libboundscheck${CMAKE_SHARED_LIBRARY_SUFFIX}
            -D LINK_NAME=libboundscheck${CMAKE_SHARED_LIBRARY_SUFFIX}
            -D WORKING_DIR=${CMAKE_BINARY_DIR}/third_party/llvm/lib
            -P "${CMAKE_SOURCE_DIR}/cmake/modules/make-symlink.cmake"
        DEPENDS codenative)
    add_custom_target(
        lldb-boundscheck ALL
        DEPENDS ${CMAKE_BINARY_DIR}/third_party/lldb/lib/libboundscheck${CMAKE_SHARED_LIBRARY_SUFFIX}
        COMMENT "Making third_party/llvm/lib/libboundscheck${CMAKE_SHARED_LIBRARY_SUFFIX}")
    install(
        CODE "execute_process(COMMAND ${CMAKE_COMMAND}
        -D CMAKE_CROSSCOMPILING=${CMAKE_CROSSCOMPILING}
        -D WIN32=OFF
        -D LINK_TARGET=../../../runtime/lib/${output_lib_dir}/libboundscheck${CMAKE_SHARED_LIBRARY_SUFFIX}
        -D LINK_NAME=libboundscheck${CMAKE_SHARED_LIBRARY_SUFFIX}
        -D WORKING_DIR=${CMAKE_INSTALL_PREFIX}/third_party/llvm/lib
        -P ${CMAKE_SOURCE_DIR}/cmake/modules/make-symlink.cmake)")

    # Create symbolic link for codira-lsp
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/third_party/lldb/lib/libcodira-lsp${CMAKE_SHARED_LIBRARY_SUFFIX}
        COMMAND ${CMAKE_COMMAND}
            -D CMAKE_CROSSCOMPILING=${CMAKE_CROSSCOMPILING}
            -D WIN32=OFF
            -D LINK_TARGET=../../../tools/lib/libcodira-lsp${CMAKE_SHARED_LIBRARY_SUFFIX}
            -D LINK_NAME=libcodira-lsp${CMAKE_SHARED_LIBRARY_SUFFIX}
            -D WORKING_DIR=${CMAKE_BINARY_DIR}/third_party/llvm/lib
            -P "${CMAKE_SOURCE_DIR}/cmake/modules/make-symlink.cmake"
        DEPENDS lldb)
    add_custom_target(
        lldb-codira-lsp ALL
        DEPENDS ${CMAKE_BINARY_DIR}/third_party/lldb/lib/libcodira-lsp${CMAKE_SHARED_LIBRARY_SUFFIX}
        COMMENT "Making third_party/llvm/lib/libcodira-lsp${CMAKE_SHARED_LIBRARY_SUFFIX}")
    install(
        CODE "execute_process(COMMAND ${CMAKE_COMMAND}
        -D CMAKE_CROSSCOMPILING=${CMAKE_CROSSCOMPILING}
        -D WIN32=OFF
        -D LINK_TARGET=../../../tools/lib/libcodira-lsp${CMAKE_SHARED_LIBRARY_SUFFIX}
        -D LINK_NAME=libcodira-lsp${CMAKE_SHARED_LIBRARY_SUFFIX}
        -D WORKING_DIR=${CMAKE_INSTALL_PREFIX}/third_party/llvm/lib
        -P ${CMAKE_SOURCE_DIR}/cmake/modules/make-symlink.cmake)")
    # Create symbolic link for codira-frontend
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/third_party/lldb/lib/libcodira-frontend${CMAKE_SHARED_LIBRARY_SUFFIX}
        COMMAND ${CMAKE_COMMAND}
            -D CMAKE_CROSSCOMPILING=${CMAKE_CROSSCOMPILING}
            -D WIN32=OFF
            -D LINK_TARGET=../../../tools/lib/libcodira-frontend${CMAKE_SHARED_LIBRARY_SUFFIX}
            -D LINK_NAME=libcodira-frontend${CMAKE_SHARED_LIBRARY_SUFFIX}
            -D WORKING_DIR=${CMAKE_BINARY_DIR}/third_party/llvm/lib
            -P "${CMAKE_SOURCE_DIR}/cmake/modules/make-symlink.cmake"
        DEPENDS lldb)
    add_custom_target(
        lldb-codira-frontend ALL
        DEPENDS ${CMAKE_BINARY_DIR}/third_party/lldb/lib/libcodira-frontend${CMAKE_SHARED_LIBRARY_SUFFIX}
        COMMENT "Making third_party/llvm/lib/libcodira-frontend${CMAKE_SHARED_LIBRARY_SUFFIX}")
    install(
        CODE "execute_process(COMMAND ${CMAKE_COMMAND}
        -D CMAKE_CROSSCOMPILING=${CMAKE_CROSSCOMPILING}
        -D WIN32=OFF
        -D LINK_TARGET=../../../tools/lib/libcodira-frontend${CMAKE_SHARED_LIBRARY_SUFFIX}
        -D LINK_NAME=libcodira-frontend${CMAKE_SHARED_LIBRARY_SUFFIX}
        -D WORKING_DIR=${CMAKE_INSTALL_PREFIX}/third_party/llvm/lib
        -P ${CMAKE_SOURCE_DIR}/cmake/modules/make-symlink.cmake)")
endif()
