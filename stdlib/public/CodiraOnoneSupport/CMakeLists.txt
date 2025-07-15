set(languageOnoneSupport_common_options
  IS_STDLIB

  # This file should be listed the first.  Module name is inferred from the
  # filename.
  CodiraOnoneSupport.code

  "${LANGUAGE_SOURCE_DIR}/stdlib/linker-support/magic-symbols-for-install-name.c"

  LANGUAGE_COMPILE_FLAGS "-parse-stdlib" "-Xtoolchain" "-sil-inline-generics=false" "-Xfrontend" "-validate-tbd-against-ir=none" "-Xfrontend" "-check-onone-completeness" "-Xfrontend" "-disable-access-control" "-strict-memory-safety" "-enable-experimental-feature" "AllowUnsafeAttribute" "${LANGUAGE_RUNTIME_LANGUAGE_COMPILE_FLAGS}" "${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}"
  LINK_FLAGS "${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND BOOTSTRAPPING_MODE STREQUAL "BOOTSTRAPPING")

  set(languageOnoneSupport_common_bootstrapping_options
    SHARED
    ${languageOnoneSupport_common_options}
    SDK ${LANGUAGE_HOST_VARIANT_SDK}
    ARCHITECTURE ${LANGUAGE_HOST_VARIANT_ARCH}
    INSTALL_IN_COMPONENT
      "never_install")

  # Bootstrapping - stage 0

  add_language_target_library_single(languageCodiraOnoneSupport-bootstrapping0 languageCodiraOnoneSupport
    ${languageOnoneSupport_common_bootstrapping_options}
    FILE_DEPENDS languageCore-bootstrapping0
    BOOTSTRAPPING 0)

  # Bootstrapping - stage 1

  add_language_target_library_single(languageCodiraOnoneSupport-bootstrapping1 languageCodiraOnoneSupport
    ${languageOnoneSupport_common_bootstrapping_options}
    FILE_DEPENDS languageCore-bootstrapping1
    BOOTSTRAPPING 1)

endif()

add_language_target_library(languageCodiraOnoneSupport
  ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES}
  ${languageOnoneSupport_common_options}
  MACCATALYST_BUILD_FLAVOR "zippered"
  INSTALL_IN_COMPONENT stdlib)

if(WINDOWS IN_LIST LANGUAGE_SDKS)
  # When building in Debug mode, the standard library provides the symbols that
  # we need and as such CodiraOnoneSupport does not need to provide any exported
  # interfaces.  This results in the import library being elided.  However,
  # we explicitly link against the CodiraOnoneSupport library when building
  # programs in Debug mode, and need the import library to be generated even if
  # nothing is exported.  Because we will still generate the DLL, create an
  # empty import library.
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/languageCodiraOnoneSupport.def
    "LIBRARY languageCodiraOnoneSupport\n"
    "EXPORTS\n")
  foreach(architecture ${LANGUAGE_SDK_WINDOWS_ARCHITECTURES})
    target_sources(languageCodiraOnoneSupport-windows-${architecture} PRIVATE
      ${CMAKE_CURRENT_BINARY_DIR}/languageCodiraOnoneSupport.def)
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/languageCodiraOnoneSupport.def
      PROPERTIES HEADER_FILE_ONLY TRUE)
  endforeach()
endif()
