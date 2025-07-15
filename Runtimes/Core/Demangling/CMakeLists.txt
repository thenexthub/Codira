# FIXME: Refactor demangling library so that we aren't pulling sources from
#        the compiler.
add_library(languageDemangling OBJECT
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Demangling/Context.cpp"
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Demangling/Demangler.cpp"
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Demangling/ManglingUtils.cpp"
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Demangling/NodePrinter.cpp"
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Demangling/Punycode.cpp"
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Demangling/Remangler.cpp"
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Demangling/NodeDumper.cpp"
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Demangling/Errors.cpp")
target_compile_definitions(languageDemangling PRIVATE
  $<$<BOOL:${BUILD_SHARED_LIBS}>:-DlanguageCore_EXPORTS>
  $<$<BOOL:${CodiraCore_ENABLE_OBJC_INTEROP}>:-DLANGUAGE_SUPPORT_OLD_MANGLING>
  $<$<BOOL:${CodiraCore_ENABLE_TYPE_PRINTING}>:-DLANGUAGE_STDLIB_HAS_TYPE_PRINTING>
  $<$<BOOL:${CodiraCore_ENABLE_CRASH_REPORTER_CLIENT}>:-DLANGUAGE_HAVE_CRASHREPORTERCLIENT>
  $<$<BOOL:${CodiraCore_HAS_ASL}>:-DLANGUAGE_STDLIB_HAS_ASL>)

# Target libraries that include libDemangling must define the name to use for
# the inline namespace to distinguish symbols from those built for the
# compiler, in order to avoid possible ODR violations if both are statically
# linked into the same binary. (see also commit message for 5b1daa9055c99904c84862ecc313641fd9b26e63)
target_compile_definitions(languageDemangling PUBLIC
    $<$<COMPILE_LANGUAGE:C,CXX>:-DLANGUAGE_INLINE_NAMESPACE=__runtime>)

target_include_directories(languageDemangling
  PRIVATE
    "${CodiraCore_LANGUAGEC_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include")

target_link_libraries(languageDemangling PRIVATE languageShims)

if(CodiraCore_ENABLE_CRASH_REPORTER_CLIENT)
  # We could likely pull the copy from the runtime sources
  add_library(languageDemanglingCR OBJECT
    "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Demangling/CrashReporter.cpp")
  target_link_libraries(languageDemanglingCR PRIVATE languageShims)
endif()

if(CodiraCore_ENABLE_OBJC_INTEROP)
  target_sources(languageDemangling PRIVATE
    "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Demangling/OldDemangler.cpp"
    "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Demangling/OldRemangler.cpp")
endif()

if(LINUX OR BSD)
  target_compile_options(languageDemangling PRIVATE -fno-lto)
endif()

if(NOT BUILD_SHARED_LIBS)
  install(TARGETS languageDemangling
    EXPORT CodiraCoreTargets
    COMPONENT CodiraCore_runtime)
endif()
