set(language_demangling_compile_flags
  TOOLCHAIN_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1
  LANGUAGE_SUPPORT_OLD_MANGLING=1
  LANGUAGE_STDLIB_HAS_TYPE_PRINTING=1)

if(LANGUAGE_RUNTIME_CRASH_REPORTER_CLIENT)
  list(APPEND language_demangling_compile_flags
      "-DLANGUAGE_HAVE_CRASHREPORTERCLIENT=1")
endif()

add_language_host_library(languageDemangling STATIC
  Demangler.cpp
  Context.cpp
  ManglingUtils.cpp
  NodeDumper.cpp
  NodePrinter.cpp
  OldDemangler.cpp
  OldRemangler.cpp
  Punycode.cpp
  Remangler.cpp
  Errors.cpp
  CrashReporter.cpp)
target_compile_definitions(languageDemangling PRIVATE
  ${language_demangling_compile_flags})

# NOTE: Runtime libraries that depend on languageDemangling should define
# LANGUAGE_INLINE_NAMESPACE to specify the identifier that will be used for an
# inline namespace that will be added around the symbols defined by this
# library. This keeps the demangling symbols in those libraries distinct
# from those in the compiler, which prevents ODR violations in certain
# contexts; for example, on platforms that support statically linking the
# Codira standard library, it allows this to happen safely when the binary
# also links in compiler libraries that may not match exactly with the
# runtime version.
