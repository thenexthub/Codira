add_language_target_library(languageCommandLineSupport
  STATIC DONT_EMBED_BITCODE NOLANGUAGERT
  CommandLine.cpp
  C_COMPILE_FLAGS ${LANGUAGE_RUNTIME_CXX_FLAGS}
  LINK_FLAGS ${LANGUAGE_RUNTIME_LINK_FLAGS}
  LANGUAGE_COMPILE_FLAGS ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
  INSTALL_IN_COMPONENT never_install
  )
