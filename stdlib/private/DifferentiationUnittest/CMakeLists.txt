add_language_target_library(languageDifferentiationUnittest ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
  # This file should be listed first. Module name is inferred from the filename.
  GYB_SOURCES DifferentiationUnittest.code.gyb

  LANGUAGE_MODULE_DEPENDS _Differentiation StdlibUnittest
  INSTALL_IN_COMPONENT stdlib-experimental
  DARWIN_INSTALL_NAME_DIR "${LANGUAGE_DARWIN_STDLIB_PRIVATE_INSTALL_NAME_DIR}"
  MACCATALYST_BUILD_FLAVOR "zippered")
