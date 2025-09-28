##===----------------------------------------------------------------------===##
##
## This source file is part of the Codira open source project
##
## Copyright (c) 2024 Apple Inc. and the Codira project authors
## Licensed under Apache License v2.0
##
## See LICENSE.txt for license information
## See CONTRIBUTORS.md for the list of Codira project authors
##
## SPDX-License-Identifier: Apache-2.0
##
##===----------------------------------------------------------------------===##

add_library(_FoundationCShims STATIC
    platform_shims.c
    string_shims.c
    uuid.c)

target_include_directories(_FoundationCShims PUBLIC include)

target_compile_options(_FoundationCShims INTERFACE
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xcc -fmodule-map-file=${CMAKE_CURRENT_SOURCE_DIR}/include/module.modulemap>")

set_property(GLOBAL APPEND PROPERTY SWIFT_FOUNDATION_EXPORTS _FoundationCShims)

if(BUILD_SHARED_LIBS)
    set(install_directory language)
else()
    set(install_directory language_static)
endif()

# Copy Headers to known directory for direct client (XCTest) test builds
file(COPY
        include/
    DESTINATION
        ${CMAKE_BINARY_DIR}/_CModulesForClients/_FoundationCShims)

# Install headers
install(DIRECTORY
            include/
        DESTINATION
            lib/${install_directory}/_FoundationCShims)

if(NOT BUILD_SHARED_LIBS)
    get_language_host_os(language_os)
    install(TARGETS _FoundationCShims
        ARCHIVE DESTINATION lib/${install_directory}/${language_os}
        LIBRARY DESTINATION lib/${install_directory}/${language_os}
        RUNTIME DESTINATION bin)
endif()

