#===--- CMakeLists.txt - Concurrency support library ---------------------===#
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

set(language_distributed_darwin_dependencies)

if((LANGUAGE_BUILD_CLANG_OVERLAYS
    OR LANGUAGE_BUILD_TEST_SUPPORT_MODULES)
   AND (NOT DEFINED LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT
        OR NOT LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT))
  # This library imports Darwin from the SDK, which re-exports
  # _Builtin_float which is part of this build.
  list(APPEND language_distributed_darwin_dependencies _Builtin_float)
endif()

set(language_distributed_link_libraries
  languageCore)


add_language_target_library(languageDistributed ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB IMPORTS_NON_OSSA 
  DistributedActor.cpp
  DistributedActor.code
  DistributedActorSystem.code
  DistributedAssertions.code
  DistributedDefaultExecutor.code
  DistributedMacros.code
  DistributedMetadata.code
  LocalTestingDistributedActorSystem.code

  LANGUAGE_MODULE_DEPENDS_IOS ${language_distributed_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_OSX ${language_distributed_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_TVOS ${language_distributed_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_WATCHOS ${language_distributed_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_ANDROID Android
  LANGUAGE_MODULE_DEPENDS_LINUX Glibc
  LANGUAGE_MODULE_DEPENDS_LINUX_STATIC Musl
  LANGUAGE_MODULE_DEPENDS_FREEBSD Glibc
  LANGUAGE_MODULE_DEPENDS_OPENBSD Glibc
  LANGUAGE_MODULE_DEPENDS_CYGWIN Glibc
  LANGUAGE_MODULE_DEPENDS_HAIKU Glibc
  LANGUAGE_MODULE_DEPENDS_WINDOWS CRT WinSDK

  LINK_LIBRARIES ${language_distributed_link_libraries}

  C_COMPILE_FLAGS
    -DlanguageDistributed_EXPORTS
    -I${LANGUAGE_SOURCE_DIR}/stdlib/include

  LANGUAGE_COMPILE_FLAGS
    ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
    -parse-stdlib
    -enable-experimental-feature AllowUnsafeAttribute
    -strict-memory-safety

  LINK_FLAGS
    "${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}"

  LANGUAGE_MODULE_DEPENDS _Concurrency
  INSTALL_IN_COMPONENT stdlib
)
