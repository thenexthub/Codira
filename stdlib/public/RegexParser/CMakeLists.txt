#===--- CMakeLists.txt - Pattern matching engine support library -----------===#
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2021 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
#
#===------------------------------------------------------------------------===#

set(language_matching_engine_link_libraries
  languageCore)

file(GLOB_RECURSE _MATCHING_ENGINE_SOURCES
  "${LANGUAGE_PATH_TO_STRING_PROCESSING_SOURCE}/Sources/_RegexParser/*.code")
set(MATCHING_ENGINE_SOURCES)
foreach(source ${_MATCHING_ENGINE_SOURCES})
  file(TO_CMAKE_PATH "${source}" source)
  list(APPEND MATCHING_ENGINE_SOURCES ${source})
endforeach()
message(STATUS "Using Experimental String Processing library for _RegexParser (${LANGUAGE_PATH_TO_STRING_PROCESSING_SOURCE}).")

set(LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS_NO_RESILIENCE)
string(REGEX REPLACE "-enable-library-evolution" ""
  LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS_NO_RESILIENCE
  "${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}")

add_language_target_library(language_RegexParser ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
  "${MATCHING_ENGINE_SOURCES}"

  LANGUAGE_MODULE_DEPENDS_LINUX Glibc
  LANGUAGE_MODULE_DEPENDS_LINUX_STATIC Musl
  LANGUAGE_MODULE_DEPENDS_FREEBSD Glibc
  LANGUAGE_MODULE_DEPENDS_OPENBSD Glibc
  LANGUAGE_MODULE_DEPENDS_CYGWIN Glibc
  LANGUAGE_MODULE_DEPENDS_HAIKU Glibc
  LANGUAGE_MODULE_DEPENDS_WINDOWS CRT

  LINK_LIBRARIES ${language_matching_engine_link_libraries}

  C_COMPILE_FLAGS
    -Dlanguage_RegexParser_EXPORTS
  LANGUAGE_COMPILE_FLAGS
    ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS_NO_RESILIENCE}
    # Workaround until `_RegexParser` is imported as implementation-only
    # by `_StringProcessing`.
    -Xfrontend -disable-implicit-string-processing-module-import
    -enable-experimental-feature AllowRuntimeSymbolDeclarations
  LINK_FLAGS "${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}"

  INSTALL_IN_COMPONENT stdlib
  MACCATALYST_BUILD_FLAVOR "zippered"
)
