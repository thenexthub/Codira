set(language_private_libc_extras_flags)
if(LANGUAGE_STDLIB_HAS_ENVIRON)
  set(language_private_libc_extras_flags "-D" "LANGUAGE_STDLIB_HAS_ENVIRON")
endif()

if(LANGUAGE_STDLIB_STATIC_PRINT)
  list(APPEND language_private_libc_extras_flags "-D" "LANGUAGE_STDLIB_STATIC_PRINT")
endif()

set(language_private_libc_extras_incorporate_object_libraries)
if(LANGUAGE_STDLIB_HAS_COMMANDLINE)
  list(APPEND language_private_libc_extras_flags "-D" "LANGUAGE_STDLIB_HAS_COMMANDLINE")
else()
  set(language_private_libc_extras_incorporate_object_libraries "languageCommandLineSupport")
endif()

set(language_private_libc_extras_darwin_dependencies)

if((LANGUAGE_BUILD_CLANG_OVERLAYS
    OR LANGUAGE_BUILD_TEST_SUPPORT_MODULES)
   AND (NOT DEFINED LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT
        OR NOT LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT))
  # This library imports Darwin from the SDK, which re-exports
  # _Builtin_float which is part of this build.
  list(APPEND language_private_libc_extras_darwin_dependencies _Builtin_float)
endif()

add_language_target_library(languageCodiraPrivateLibcExtras ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
  # This file should be listed the first.  Module name is inferred from the
  # filename.
  CodiraPrivateLibcExtras.code
  Subprocess.c
  Subprocess.code

  "${LANGUAGE_SOURCE_DIR}/stdlib/linker-support/magic-symbols-for-install-name.c"

  LANGUAGE_MODULE_DEPENDS CodiraPrivate
  LANGUAGE_COMPILE_FLAGS ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS} ${language_private_libc_extras_flags}
  LANGUAGE_MODULE_DEPENDS_OSX ${language_private_libc_extras_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_IOS ${language_private_libc_extras_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_TVOS ${language_private_libc_extras_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_WATCHOS ${language_private_libc_extras_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_XROS ${language_private_libc_extras_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_MACCATALYST ${language_private_libc_extras_darwin_dependencies}
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
  INCORPORATE_OBJECT_LIBRARIES ${language_private_libc_extras_incorporate_object_libraries}
  INSTALL_IN_COMPONENT stdlib-experimental
  DARWIN_INSTALL_NAME_DIR "${LANGUAGE_DARWIN_STDLIB_PRIVATE_INSTALL_NAME_DIR}"
  MACCATALYST_BUILD_FLAVOR "zippered")
