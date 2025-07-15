#===--- CMakeLists.txt - String processing support library -----------------===#
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

set(language_regex_builder_link_libraries
  languageCore
  language_RegexParser
  language_StringProcessing)

file(GLOB_RECURSE _REGEX_BUILDER_SOURCES
  "${LANGUAGE_PATH_TO_STRING_PROCESSING_SOURCE}/Sources/RegexBuilder/*.code")
set(REGEX_BUILDER_SOURCES)
foreach(source ${_REGEX_BUILDER_SOURCES})
  file(TO_CMAKE_PATH "${source}" source)
  list(APPEND REGEX_BUILDER_SOURCES ${source})
endforeach()
message(STATUS "Using Experimental String Processing library for RegexBuilder (${LANGUAGE_PATH_TO_STRING_PROCESSING_SOURCE}).")

add_language_target_library(languageRegexBuilder ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
  "${REGEX_BUILDER_SOURCES}"

  LANGUAGE_MODULE_DEPENDS_ANDROID Android
  LANGUAGE_MODULE_DEPENDS_LINUX Glibc
  LANGUAGE_MODULE_DEPENDS_LINUX_STATIC Musl
  LANGUAGE_MODULE_DEPENDS_FREEBSD Glibc
  LANGUAGE_MODULE_DEPENDS_OPENBSD Glibc
  LANGUAGE_MODULE_DEPENDS_CYGWIN Glibc
  LANGUAGE_MODULE_DEPENDS_HAIKU Glibc
  LANGUAGE_MODULE_DEPENDS_WINDOWS CRT

  LINK_LIBRARIES ${language_regex_builder_link_libraries}

  C_COMPILE_FLAGS
    -DlanguageRegexBuilder_EXPORTS
  LANGUAGE_COMPILE_FLAGS
    ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
  LINK_FLAGS "${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}"

  LANGUAGE_MODULE_DEPENDS _StringProcessing
  INSTALL_IN_COMPONENT stdlib
)
