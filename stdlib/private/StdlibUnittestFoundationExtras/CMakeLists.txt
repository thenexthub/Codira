add_language_target_library(languageStdlibUnittestFoundationExtras ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
  # This file should be listed the first.  Module name is inferred from the
  # filename.
  StdlibUnittestFoundationExtras.code
  UnavailableFoundationMethodThunks.mm

  LANGUAGE_MODULE_DEPENDS StdlibUnittest
  LANGUAGE_COMPILE_FLAGS ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
  INSTALL_IN_COMPONENT stdlib-experimental
  DARWIN_INSTALL_NAME_DIR "${LANGUAGE_DARWIN_STDLIB_PRIVATE_INSTALL_NAME_DIR}")

