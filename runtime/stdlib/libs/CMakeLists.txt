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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/boundscheck-v1.1.16/include)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

set(BACKEND "codenative")
if(CODIRALIB_IS_CODENATIVE_BACKEND)
    set(BACKEND "codenative")
endif()

if(CODIRA_BUILD_STDLIB_WITH_COVERAGE)
    set(CMAKE_C_COVERAGE_FLAGS "--coverage")
endif()

add_subdirectory(std)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
# The code codes are used only when building libs(.so, .a, .codeo etc.),
# so it should be controlled by CODIRA_BUILD_STDLIB.
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    include(MakeCODENATIVEStdSharedLib)
    include(AddCODENATIVELibrary)
endif()

if(CODIRA_CODEGEN_CODENATIVE_BACKEND AND NOT TRIPLE STREQUAL "arm-linux-ohos" AND NOT CODIRA_SANITIZER_SUPPORT_ENABLED)
    set(CODENATIVE_BACKEND "codenative")
    set(libname stdFFI)
    add_library(${libname} SHARED ${CODIRA_CORE_FFI_SRC})
    if(CMAKE_BUILD_TYPE MATCHES Release)
        target_link_options( ${libname} PRIVATE ${LINK_FLAGS} ${STRIP_FLAG} ${useld})
    else()
        target_link_options(${libname} PRIVATE ${LINK_FLAGS} ${useld})
    endif()
    target_link_directories(${libname} PRIVATE ${CMAKE_BINARY_DIR}/lib
        PRIVATE ${CMAKE_BINARY_DIR}/lib/${TARGET_TRIPLE_DIRECTORY_PREFIX}_${CODENATIVE_BACKEND}
        PRIVATE ${CMAKE_BINARY_DIR}/runtime/lib/${TARGET_TRIPLE_DIRECTORY_PREFIX}_${CODENATIVE_BACKEND})
    set(pcre2_lib "pcre2-8")
    if(DARWIN)
        set(pcre2_lib -L${CMAKE_BINARY_DIR}/third_party/pcre2/lib -l${pcre2_lib})
    elseif(MINGW)
        set(pcre2_lib -L${CMAKE_BINARY_DIR}/third_party/pcre2/lib -l:lib${pcre2_lib}${CMAKE_SHARED_LIBRARY_SUFFIX}.a)
    else()
        set(pcre2_lib -L${CMAKE_BINARY_DIR}/third_party/pcre2/lib -l:lib${pcre2_lib}${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
    target_link_libraries(${libname} boundscheck)
    target_link_libraries(${libname} ${CORE_FFI_OBJECTS_LIST})
    target_link_libraries(${libname} cangjie-std-mathFFI-objs)
    target_link_libraries(${libname} cangjie-std-convertFFI-objs)
    target_link_libraries(${libname} cangjie-std-fsFFI-objs)
    target_link_libraries(${libname} cangjie-std-envFFI-objs)
    target_link_libraries(${libname} cangjie-std-posixFFI-objs)
    target_link_libraries(${libname} cangjie-std-processFFI-objs)
    target_link_libraries(${libname} cangjie-std-randomFFI-objs)
    target_link_libraries(${libname} cangjie-std-regexFFI-objs)
    target_link_libraries(${libname} cangjie-std-netFFI-objs)
    target_link_libraries(${libname} cangjie-std-timeFFI-objs)
    target_link_libraries(${libname} cangjie-std-unittestFFI-objs)
    target_link_libraries(${libname} cangjie-std-objectpoolFFI-objs)
    target_link_libraries(${libname} cangjie-std-runtimeFFI-objs)
    target_link_libraries(${libname} cangjie-std-regexFFI)
    target_link_libraries(${libname} ${pcre2_lib})
    add_dependencies(${libname} modify_typetemplate_object)
    target_link_libraries(${libname} m)
    if(CODIRA_BUILD_STDLIB_WITH_COVERAGE)
        target_link_libraries(${libname} clang_rt-profile)
    endif()

    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        if(DARWIN)
            target_link_libraries(${libname} "-Wl,-force_load,${RUNTIME_COMMON_LIB_DIR}/libcangjie-aio.a")
        else()
            target_link_libraries(${libname} "-Wl,--whole-archive" ${RUNTIME_COMMON_LIB_DIR}/libcangjie-aio.a "-Wl,--no-whole-archive" )
        endif()
        if(IOS AND IOS_PLATFORM MATCHES "SIMULATOR")
            string(TOLOWER "ios_simulator_${CMAKE_SYSTEM_PROCESSOR}_${CODENATIVE_BACKEND}" tmpdir)
        else()
            string(TOLOWER "${target_folder_name}_${CMAKE_SYSTEM_PROCESSOR}_${CODENATIVE_BACKEND}" tmpdir)
        endif()
        target_link_directories(${libname} PRIVATE ${RUNTIME_COMMON_LIB_DIR}/../../runtime/lib/${tmpdir})
        set(runtime_lib cangjie-runtime)
        if(MINGW)
            set(runtime_lib -L${RUNTIME_COMMON_LIB_DIR} -l:libcangjie-runtime.dll.a)
        endif()
        target_link_libraries(${libname} ${runtime_lib})
    endif()
    if(MINGW)
        target_link_libraries(${libname} crypt32)
        target_link_libraries(${libname} ws2_32)
    endif()
    set(tmpdir "lib")
    install(TARGETS ${libname} DESTINATION lib)
endif()

