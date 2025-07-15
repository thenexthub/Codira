# TODO: due to the use of `add_language_target_library` rather than `add_library`
# we cannot use `target_sources` and thus must resort to list manipulations to
# adjust the source list.
set(languageDemanglingSources
  "${LANGUAGE_SOURCE_DIR}/lib/Demangling/Context.cpp"
  "${LANGUAGE_SOURCE_DIR}/lib/Demangling/Demangler.cpp"
  "${LANGUAGE_SOURCE_DIR}/lib/Demangling/ManglingUtils.cpp"
  "${LANGUAGE_SOURCE_DIR}/lib/Demangling/NodePrinter.cpp"
  "${LANGUAGE_SOURCE_DIR}/lib/Demangling/Punycode.cpp"
  "${LANGUAGE_SOURCE_DIR}/lib/Demangling/Remangler.cpp"
  "${LANGUAGE_SOURCE_DIR}/lib/Demangling/NodeDumper.cpp"
  "${LANGUAGE_SOURCE_DIR}/lib/Demangling/Errors.cpp")
set(languageDemanglingCRSources
  "${LANGUAGE_SOURCE_DIR}/lib/Demangling/CrashReporter.cpp")

set(language_demangling_cflags)

if(LANGUAGE_RUNTIME_CRASH_REPORTER_CLIENT)
  list(APPEND language_demangling_cflags
    "-DLANGUAGE_HAVE_CRASHREPORTERCLIENT=1")
endif()

# The old mangling support is only needed on platforms with ObjC.
if(LANGUAGE_STDLIB_ENABLE_OBJC_INTEROP)
  list(APPEND languageDemanglingSources
    "${LANGUAGE_SOURCE_DIR}/lib/Demangling/OldDemangler.cpp"
    "${LANGUAGE_SOURCE_DIR}/lib/Demangling/OldRemangler.cpp"
  )
  list(APPEND language_demangling_cflags -DLANGUAGE_SUPPORT_OLD_MANGLING=1)
else()
  list(APPEND language_demangling_cflags -DLANGUAGE_SUPPORT_OLD_MANGLING=0)
endif()

if(LANGUAGE_STDLIB_HAS_TYPE_PRINTING)
  list(APPEND language_demangling_cflags -DLANGUAGE_STDLIB_HAS_TYPE_PRINTING)
endif()

# Gold LTO is unsupported. To prevent tests from failing when building
# with LTO, force languageDemangling library to compile without LTO for Linux.
add_language_target_library(languageDemangling OBJECT_LIBRARY
  ${languageDemanglingSources}
  C_COMPILE_FLAGS
    -DlanguageCore_EXPORTS
    ${language_demangling_cflags}
  C_COMPILE_FLAGS_LINUX -fno-lto
  INSTALL_IN_COMPONENT never_install)

add_language_target_library(languageDemanglingCR OBJECT_LIBRARY
  ${languageDemanglingCRSources}
  C_COMPILE_FLAGS
    -DlanguageCore_EXPORTS
    ${language_demangling_cflags}
  INSTALL_IN_COMPONENT never_install)
