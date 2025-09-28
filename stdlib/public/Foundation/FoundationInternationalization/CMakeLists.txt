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

add_library(FoundationInternationalization
    BinaryFloatingPoint.code
    Date+ICU.code
    Duration+Utils.code
    RangeExpression.code
    TimeInterval+Utils.code
    URLParser+ICU.code)

add_subdirectory(Calendar)
add_subdirectory(Formatting)
add_subdirectory(ICU)
add_subdirectory(Locale)
add_subdirectory(Predicate)
add_subdirectory(String)
add_subdirectory(TimeZone)

target_compile_options(FoundationInternationalization PRIVATE
    "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -enable-experimental-feature -Xfrontend AccessLevelOnImport>"
    "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -enable-experimental-feature -Xfrontend StrictConcurrency>"
    "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -enable-upcoming-feature -Xfrontend InferSendableFromCaptures>"
    "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -enable-upcoming-feature -Xfrontend MemberImportVisibility>")
target_compile_options(FoundationInternationalization PRIVATE ${_CodiraFoundation_availability_macros})
target_compile_options(FoundationInternationalization PRIVATE ${_CodiraFoundation_wasi_libc_flags})
target_compile_options(FoundationInternationalization PRIVATE -package-name "CodiraFoundation")

target_link_libraries(FoundationInternationalization PUBLIC
    FoundationEssentials
    _FoundationCShims
    _FoundationICU)
target_link_libraries(FoundationInternationalization PUBLIC ${_CodiraFoundation_wasi_libc_libraries})

if(NOT BUILD_SHARED_LIBS)
    target_compile_options(FoundationInternationalization PRIVATE
        "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -public-autolink-library -Xfrontend _FoundationCShims>")
    target_compile_options(FoundationInternationalization PRIVATE
        "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -public-autolink-library -Xfrontend _FoundationICU>")
    target_compile_options(FoundationEssentials PRIVATE
        "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -public-autolink-library -Xfrontend $<$<PLATFORM_ID:Windows>:${CMAKE_STATIC_LIBRARY_PREFIX_Codira}>languageSynchronization>")
endif()

set_target_properties(FoundationInternationalization PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    INSTALL_REMOVE_ENVIRONMENT_RPATH ON)

set_property(GLOBAL APPEND PROPERTY SWIFT_FOUNDATION_EXPORTS FoundationInternationalization)
_language_foundation_install_target(FoundationInternationalization)
