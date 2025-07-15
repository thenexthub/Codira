#===--- CMakeLists.txt - Differentiable programming support library ------===#
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2019 - 2020 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
#
#===----------------------------------------------------------------------===#

if(LANGUAGE_STDLIB_ENABLE_VECTOR_TYPES)
set(languageDifferentiationSIMDFiles SIMDDifferentiation.code.gyb)
else()
set(languageDifferentiationSIMDFiles)
endif()

add_language_target_library(language_Differentiation ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
  Differentiable.code
  DifferentialOperators.code
  DifferentiationUtilities.code
  AnyDifferentiable.code
  ArrayDifferentiation.code
  OptionalDifferentiation.code

  "${LANGUAGE_SOURCE_DIR}/stdlib/linker-support/magic-symbols-for-install-name.c"

  GYB_SOURCES
    FloatingPointDifferentiation.code.gyb
    TgmathDerivatives.code.gyb
    ${languageDifferentiationSIMDFiles}

  LANGUAGE_MODULE_DEPENDS_ANDROID Android
  LANGUAGE_MODULE_DEPENDS_LINUX Glibc
  LANGUAGE_MODULE_DEPENDS_LINUX_STATIC Musl
  LANGUAGE_MODULE_DEPENDS_FREEBSD Glibc
  LANGUAGE_MODULE_DEPENDS_OPENBSD Glibc
  LANGUAGE_MODULE_DEPENDS_CYGWIN Glibc
  LANGUAGE_MODULE_DEPENDS_HAIKU Glibc
  LANGUAGE_MODULE_DEPENDS_WASI WASILibc
  LANGUAGE_MODULE_DEPENDS_WINDOWS CRT

  C_COMPILE_FLAGS
    -Dlanguage_Differentiation_EXPORTS
  LANGUAGE_COMPILE_FLAGS
    ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
    -parse-stdlib
  LINK_FLAGS "${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}"
  INSTALL_IN_COMPONENT stdlib
  LANGUAGE_MODULE_DEPENDS_MACCATALYST ${languageDifferentiationDarwinDependencies}
  MACCATALYST_BUILD_FLAVOR "zippered")
