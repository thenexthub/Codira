set(language_stdlib_unittest_darwin_dependencies)

if((LANGUAGE_BUILD_CLANG_OVERLAYS
    OR LANGUAGE_BUILD_TEST_SUPPORT_MODULES)
   AND (NOT DEFINED LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT
        OR NOT LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT))
  # This library imports Darwin from the SDK, which re-exports
  # _Builtin_float which is part of this build.
  list(APPEND language_stdlib_unittest_darwin_dependencies _Builtin_float)
endif()

set(language_stdlib_unittest_compile_flags
  "-Xfrontend" "-disable-objc-attr-requires-foundation-module")

list(APPEND language_stdlib_unittest_compile_flags "-enable-experimental-feature" "Lifetimes")

if (LANGUAGE_RUNTIME_ENABLE_LEAK_CHECKER)
  list(APPEND language_stdlib_unittest_compile_flags "-DLANGUAGE_RUNTIME_ENABLE_LEAK_CHECKER")
endif()

is_build_type_optimized("${LANGUAGE_STDLIB_BUILD_TYPE}" IS_BUILD_TYPE_OPTIMIZED)
if (NOT IS_BUILD_TYPE_OPTIMIZED)
  list(APPEND language_stdlib_unittest_compile_flags "-DLANGUAGE_STDLIB_DEBUG")
endif()

if(LANGUAGE_ENABLE_REFLECTION)
  list(APPEND language_stdlib_unittest_compile_flags "-DLANGUAGE_ENABLE_REFLECTION")
endif()

if(LANGUAGE_STDLIB_STATIC_PRINT)
  list(APPEND language_stdlib_unittest_compile_flags "-D" "LANGUAGE_STDLIB_STATIC_PRINT")
endif()

set(language_stdlib_unittest_link_libraries "")
set(language_stdlib_unittest_modules "")
if (LANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY)
  list(APPEND language_stdlib_unittest_link_libraries "language_Concurrency")
  list(APPEND language_stdlib_unittest_modules "_Concurrency")
endif()
if (LANGUAGE_ENABLE_EXPERIMENTAL_STRING_PROCESSING)
  list(APPEND language_stdlib_unittest_modules "_StringProcessing")
endif()

add_language_target_library(languageStdlibUnittest ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
  # This file should be listed the first.  Module name is inferred from the
  # filename.
  StdlibUnittest.code

  CheckStrideable.code
  InspectValue.cpp
  InspectValue.code
  InterceptTraps.cpp
  LifetimeTracked.code
  MinimalTypes.code
  OpaqueIdentityFunctions.cpp
  OpaqueIdentityFunctions.code
  RaceTest.code
  Statistics.code
  StdlibCoreExtras.code
  StringConvertible.code
  StringTestHelpers.code
  TestHelpers.code
  TypeIndexed.code

  LANGUAGE_MODULE_DEPENDS CodiraPrivate CodiraPrivateThreadExtras CodiraPrivateLibcExtras ${language_stdlib_unittest_modules}
  LANGUAGE_MODULE_DEPENDS_IOS ${language_stdlib_unittest_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_OSX ${language_stdlib_unittest_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_TVOS ${language_stdlib_unittest_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_WATCHOS ${language_stdlib_unittest_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_XROS ${language_stdlib_unittest_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_MACCATALYST ${language_stdlib_unittest_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_FREESTANDING "${LANGUAGE_FREESTANDING_TEST_DEPENDENCIES}"
  LANGUAGE_MODULE_DEPENDS_ANDROID Android
  LANGUAGE_MODULE_DEPENDS_LINUX Glibc
  LANGUAGE_MODULE_DEPENDS_LINUX_STATIC Musl
  LANGUAGE_MODULE_DEPENDS_FREEBSD Glibc
  LANGUAGE_MODULE_DEPENDS_OPENBSD Glibc
  LANGUAGE_MODULE_DEPENDS_CYGWIN Glibc
  LANGUAGE_MODULE_DEPENDS_HAIKU Glibc
  LANGUAGE_MODULE_DEPENDS_WASI WASILibc
  LANGUAGE_MODULE_DEPENDS_WINDOWS CRT WinSDK
  C_COMPILE_FLAGS -I${LANGUAGE_SOURCE_DIR}/stdlib/include
  LANGUAGE_COMPILE_FLAGS ${language_stdlib_unittest_compile_flags} ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
  INSTALL_IN_COMPONENT stdlib-experimental
  DARWIN_INSTALL_NAME_DIR "${LANGUAGE_DARWIN_STDLIB_PRIVATE_INSTALL_NAME_DIR}"
  LINK_LIBRARIES ${language_stdlib_unittest_link_libraries}
  MACCATALYST_BUILD_FLAVOR "zippered")
set_source_files_properties(InspectValue.cpp PROPERTIES COMPILE_FLAGS -std=c++17)

