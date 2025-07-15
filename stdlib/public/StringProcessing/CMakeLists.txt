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

set(language_string_processing_link_libraries
  languageCore
  language_RegexParser)

set(language_string_processing_compile_flags)

# Pass a flag to the build indicating that _StringProcessing's dependencies
# will be built with library evolution.
list(APPEND language_string_processing_compile_flags
  "-DRESILIENT_LIBRARIES")

if(LANGUAGE_BUILD_STATIC_STDLIB)
  # Explicitly autolink language_RegexParser because it's imported with @_implementationOnly
  list(APPEND language_string_processing_compile_flags
    "-Xfrontend" "-public-autolink-library" "-Xfrontend" "language_RegexParser")
endif()

file(GLOB_RECURSE _STRING_PROCESSING_SOURCES
  "${LANGUAGE_PATH_TO_STRING_PROCESSING_SOURCE}/Sources/_StringProcessing/*.code"
  "${LANGUAGE_PATH_TO_STRING_PROCESSING_SOURCE}/Sources/_CUnicode/*.h"
  "${LANGUAGE_PATH_TO_STRING_PROCESSING_SOURCE}/Sources/_CUnicode/*.c")
set(STRING_PROCESSING_SOURCES)
foreach(source ${_STRING_PROCESSING_SOURCES})
  file(TO_CMAKE_PATH "${source}" source)
  list(APPEND STRING_PROCESSING_SOURCES ${source})
endforeach()
message(STATUS "Using Experimental String Processing library for _StringProcessing (${LANGUAGE_PATH_TO_STRING_PROCESSING_SOURCE}).")

add_language_target_library(language_StringProcessing ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
  "${STRING_PROCESSING_SOURCES}"

  LANGUAGE_MODULE_DEPENDS_ANDROID Android
  LANGUAGE_MODULE_DEPENDS_LINUX Glibc
  LANGUAGE_MODULE_DEPENDS_LINUX_STATIC Musl
  LANGUAGE_MODULE_DEPENDS_FREEBSD Glibc
  LANGUAGE_MODULE_DEPENDS_OPENBSD Glibc
  LANGUAGE_MODULE_DEPENDS_CYGWIN Glibc
  LANGUAGE_MODULE_DEPENDS_HAIKU Glibc
  LANGUAGE_MODULE_DEPENDS_WINDOWS CRT

  LINK_LIBRARIES ${language_string_processing_link_libraries}

  C_COMPILE_FLAGS
    -Dlanguage_StringProcessing_EXPORTS
  LANGUAGE_COMPILE_FLAGS
    ${language_string_processing_compile_flags}
    ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
  LINK_FLAGS "${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}"

  LANGUAGE_MODULE_DEPENDS _RegexParser
  INSTALL_IN_COMPONENT stdlib
  MACCATALYST_BUILD_FLAVOR "zippered"
)
