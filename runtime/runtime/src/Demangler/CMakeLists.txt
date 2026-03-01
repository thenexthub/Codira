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

if (COV MATCHES 0)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/third_party_bounds_checking_function/include)
    set(DEMANGLER_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/../Base/CString.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Base/FixedCString.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Demangler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DeCompression.cpp)
    add_library(demangler STATIC ${DEMANGLER_SRC})
    # codefilt
    add_executable(codefilt ${CMAKE_CURRENT_SOURCE_DIR}/Demangler.cpp ${CMAKE_CURRENT_SOURCE_DIR}/DeCompression.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Codefilt.cpp)
    add_dependencies(codefilt demangler)
    target_compile_definitions(codefilt PRIVATE BUILD_LIB_CODIRA_DEMANGLE)
    target_link_libraries(codefilt PRIVATE demangler)
    # libcangjie-demangle
    add_library(cangjie-demangle STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/DeCompression.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Demangler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CodiraDemangle.cpp)
    target_compile_definitions(cangjie-demangle PRIVATE BUILD_LIB_CODIRA_DEMANGLE)
    if (OHOS_FLAG IN_LIST OHOS_FLAG_LIST)
        add_library(cangjie-demangle-shared SHARED
            ${CMAKE_CURRENT_SOURCE_DIR}/DeCompression.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/Demangler.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/CodiraDemangle.cpp)
        target_compile_definitions(cangjie-demangle-shared PRIVATE BUILD_LIB_CODIRA_DEMANGLE)
        install(TARGETS cangjie-demangle-shared LIBRARY DESTINATION runtime/lib/${TARGET_ARCH})
        set_target_properties(cangjie-demangle-shared PROPERTIES OUTPUT_NAME "cangjie-demangle")
    endif()
    # demangleTest
    add_executable(demangleTest ${CMAKE_CURRENT_SOURCE_DIR}/Demangler.cpp ${CMAKE_CURRENT_SOURCE_DIR}/DeCompression.cpp ${CMAKE_CURRENT_SOURCE_DIR}/CheckDemangle.cpp)
    add_dependencies(demangleTest demangler cangjie-demangle)
    if (OHOS_FLAG IN_LIST OHOS_FLAG_LIST)
        target_link_libraries(demangleTest PRIVATE cangjie-demangle demangler boundscheck hilog_ndk.z hitrace_ndk.z)
    elseif(ANDROID_FLAG MATCHES 1 OR ANDROID_FLAG MATCHES 2)
        target_link_libraries(demangleTest PRIVATE cangjie-demangle demangler boundscheck "-landroid -llog")
    else ()
        target_link_libraries(demangleTest PRIVATE cangjie-demangle demangler boundscheck)
    endif ()
    if (WINDOWS_FLAG MATCHES 1)
        target_link_libraries(demangleTest PRIVATE unwind)
        target_link_libraries(codefilt PRIVATE unwind)
    endif()
    # install
    install(TARGETS codefilt DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CodiraDemangle.h DESTINATION include)
    install(TARGETS cangjie-demangle DESTINATION ${CMAKE_INSTALL_PREFIX}/runtime/lib/${TARGET_ARCH})
endif()
