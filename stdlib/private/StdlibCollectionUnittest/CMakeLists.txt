set(language_stdlib_unittest_compile_flags)

add_language_target_library(languageStdlibCollectionUnittest ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
  # This file should be listed the first.  Module name is inferred from the
  # filename.
  StdlibCollectionUnittest.code

  CheckCollectionInstance.code
  CheckCollectionType.code
  CheckMutableCollectionType.code
  CheckRangeReplaceableCollectionType.code
  CheckRangeReplaceableSliceType.code
  CheckSequenceInstance.code
  CheckSequenceType.code
  COWLoggingArray.code
  LoggingWrappers.code
  MinimalCollections.code
  RangeSelection.code
  WriteBackMutableSlice.code

  LANGUAGE_MODULE_DEPENDS StdlibUnittest
  LANGUAGE_MODULE_DEPENDS_ANDROID Android
  LANGUAGE_MODULE_DEPENDS_LINUX Glibc
  LANGUAGE_MODULE_DEPENDS_LINUX_STATIC Musl
  LANGUAGE_MODULE_DEPENDS_FREEBSD Glibc
  LANGUAGE_MODULE_DEPENDS_OPENBSD Glibc
  LANGUAGE_MODULE_DEPENDS_CYGWIN Glibc
  LANGUAGE_MODULE_DEPENDS_HAIKU Glibc
  LANGUAGE_MODULE_DEPENDS_WINDOWS CRT
  LANGUAGE_COMPILE_FLAGS ${language_stdlib_unittest_compile_flags} ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
  INSTALL_IN_COMPONENT stdlib-experimental
  DARWIN_INSTALL_NAME_DIR "${LANGUAGE_DARWIN_STDLIB_PRIVATE_INSTALL_NAME_DIR}"
  MACCATALYST_BUILD_FLAVOR "zippered")
