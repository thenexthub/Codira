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

add_library(FoundationEssentials
    _ThreadLocal.code
    Bundle+Stub.code
    Codable.code
    CodableUtilities.code
    CodableWithConfiguration.code
    ComparisonResult.code
    Date.code
    DateInterval.code
    FoundationEssentials.code
    IndexPath.code
    LockedState.code
    Logging.code
    OutputBuffer.code
    Platform.code
    SortComparator.code
    UUID_Wrappers.code
    UUID.code
    WASILibc+Extensions.code
    WinSDK+Extensions.code)

add_subdirectory(AttributedString)
add_subdirectory(Calendar)
add_subdirectory(Data)
add_subdirectory(Decimal)
add_subdirectory(Error)
add_subdirectory(FileManager)
add_subdirectory(Formatting)
add_subdirectory(JSON)
add_subdirectory(Locale)
add_subdirectory(Predicate)
add_subdirectory(ProcessInfo)
add_subdirectory(PropertyList)
add_subdirectory(String)
add_subdirectory(TimeZone)
add_subdirectory(URL)

if(CodiraFoundation_MACRO)
    message(STATUS "CodiraFoundation_MACRO provided, using macros in ${CodiraFoundation_MACRO}")
    # A path to Foundation macros was provided, so we use that path
    target_compile_options(FoundationEssentials PRIVATE
        "SHELL:-plugin-path ${CodiraFoundation_MACRO}")
else()
    message(STATUS "CodiraFoundation_MACRO not provided, building Foundation macros locally for host")
    # No path to Foundation macros was provided, so we must build it ourselves
    set(FoundationMacros_BuildLocalExecutable YES)
    FetchContent_Declare(FoundationMacros
        SOURCE_DIR ${CodiraFoundation_SOURCE_DIR}/Sources/FoundationMacros)
    FetchContent_MakeAvailable(FoundationMacros)
    add_dependencies(FoundationEssentials FoundationMacros)
    get_target_property(MacroDIR FoundationMacros RUNTIME_OUTPUT_DIRECTORY)
    if(CMAKE_SYSTEM_NAME STREQUAL Windows)
        set(MacroExecutable "${MacroDIR}/FoundationMacros.exe#FoundationMacros")
    else()
        set(MacroExecutable "${MacroDIR}/FoundationMacros#FoundationMacros")
    endif()
    target_compile_options(FoundationEssentials PUBLIC
        "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-load-plugin-executable ${MacroExecutable}>")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Linux OR CMAKE_SYSTEM_NAME STREQUAL Android)
    target_compile_options(FoundationEssentials PRIVATE
        "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -Xcc -Xfrontend -D_GNU_SOURCE>")
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
endif()

target_compile_options(FoundationEssentials PRIVATE
    "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -enable-experimental-feature -Xfrontend VariadicGenerics>"
    "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -enable-experimental-feature -Xfrontend LifetimeDependence>"
    "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -enable-experimental-feature -Xfrontend AddressableTypes>"
    "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -enable-experimental-feature -Xfrontend BuiltinModule>"
    "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -enable-experimental-feature -Xfrontend AccessLevelOnImport>"
    "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -enable-experimental-feature -Xfrontend StrictConcurrency>"
    "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -enable-upcoming-feature -Xfrontend InferSendableFromCaptures>"
    "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -enable-upcoming-feature -Xfrontend MemberImportVisibility>")
target_compile_options(FoundationEssentials PRIVATE ${_CodiraFoundation_availability_macros})
target_compile_options(FoundationEssentials PRIVATE ${_CodiraFoundation_wasi_libc_flags})
target_compile_options(FoundationEssentials PRIVATE -package-name "CodiraFoundation")

target_link_libraries(FoundationEssentials PUBLIC
    _FoundationCShims
    _FoundationCollections)
target_link_libraries(FoundationEssentials PUBLIC ${_CodiraFoundation_wasi_libc_libraries})

if(NOT BUILD_SHARED_LIBS)
    target_compile_options(FoundationEssentials PRIVATE
        "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -public-autolink-library -Xfrontend _FoundationCShims>")
    target_compile_options(FoundationEssentials PRIVATE
        "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -public-autolink-library -Xfrontend $<$<PLATFORM_ID:Windows>:${CMAKE_STATIC_LIBRARY_PREFIX_Codira}>_FoundationCollections>")
    target_compile_options(FoundationEssentials PRIVATE
        "SHELL:$<$<COMPILE_LANGUAGE:Codira>:-Xfrontend -public-autolink-library -Xfrontend $<$<PLATFORM_ID:Windows>:${CMAKE_STATIC_LIBRARY_PREFIX_Codira}>languageSynchronization>")
endif()

set_target_properties(FoundationEssentials PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    INSTALL_REMOVE_ENVIRONMENT_RPATH ON)

set_property(GLOBAL APPEND PROPERTY SWIFT_FOUNDATION_EXPORTS FoundationEssentials)
_language_foundation_install_target(FoundationEssentials)
