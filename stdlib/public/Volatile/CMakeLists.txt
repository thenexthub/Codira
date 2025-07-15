#===--- CMakeLists.txt ---------------------------------------------------===#
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2019 - 2022 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
#
#===----------------------------------------------------------------------===#

add_language_target_library(language_Volatile ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
  Volatile.code

  LANGUAGE_COMPILE_FLAGS
    ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
    -parse-stdlib
  LINK_FLAGS "${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}"

  INSTALL_IN_COMPONENT stdlib
)

if(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB)
  add_custom_target(embedded-volatile)
  add_dependencies(embedded-libraries embedded-volatile)
  foreach(entry ${EMBEDDED_STDLIB_TARGET_TRIPLES})
    string(REGEX REPLACE "[ \t]+" ";" list "${entry}")
    list(GET list 0 arch)
    list(GET list 1 mod)
    list(GET list 2 triple)

    set(LANGUAGE_SDK_embedded_ARCH_${arch}_MODULE "${mod}")
    set(LANGUAGE_SDK_embedded_LIB_SUBDIR "embedded")
    set(LANGUAGE_SDK_embedded_ARCH_${arch}_TRIPLE "${triple}")
    add_language_target_library_single(
      embedded-volatile-${mod}
      language_Volatile
      ONLY_LANGUAGEMODULE
      IS_SDK_OVERLAY IS_FRAGILE

      Volatile.code

      LANGUAGE_COMPILE_FLAGS
        -Xcc -ffreestanding -enable-experimental-feature Embedded
        -parse-stdlib
      C_COMPILE_FLAGS
        -ffreestanding
      MODULE_DIR "${CMAKE_BINARY_DIR}/lib/language/embedded"
      SDK "embedded"
      ARCHITECTURE "${arch}"
      DEPENDS embedded-stdlib-${mod}
      INSTALL_IN_COMPONENT stdlib
      )
    add_dependencies(embedded-volatile embedded-volatile-${mod})
  endforeach()
endif()
