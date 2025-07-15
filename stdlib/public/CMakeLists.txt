add_subdirectory(LLVMSupport)

set(LANGUAGE_RUNTIME_CXX_FLAGS)
set(LANGUAGE_RUNTIME_LINK_FLAGS)
set(LANGUAGE_RUNTIME_LANGUAGE_COMPILE_FLAGS)
set(LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS)

if(LANGUAGE_RUNTIME_USE_SANITIZERS)
  # TODO: Refactor this
  if("Thread" IN_LIST LANGUAGE_RUNTIME_USE_SANITIZERS)
    list(APPEND LANGUAGE_RUNTIME_CXX_FLAGS "-fsanitize=thread")
    list(APPEND LANGUAGE_RUNTIME_LINK_FLAGS "-fsanitize=thread")
    list(APPEND LANGUAGE_RUNTIME_LANGUAGE_COMPILE_FLAGS "-sanitize=thread")
    list(APPEND LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS "-fsanitize=thread")
  endif()
endif()

if(LANGUAGE_STDLIB_SIL_DEBUGGING)
  list(APPEND LANGUAGE_RUNTIME_LANGUAGE_COMPILE_FLAGS "-Xfrontend" "-sil-based-debuginfo")
endif()

option(LANGUAGE_RUNTIME_DYLD_SHARED_CACHE_COMPATIBLE
       "On Darwin platforms, link the standard library in a way that
       allows inclusion in the dyld shared cache"
       FALSE)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND
   TOOLCHAIN_LINKER_IS_APPLE AND
   NOT LANGUAGE_RUNTIME_DYLD_SHARED_CACHE_COMPATIBLE)
  set(shared_cache_link_flags "LINKER:-not_for_dyld_shared_cache")
  list(APPEND LANGUAGE_RUNTIME_LINK_FLAGS ${shared_cache_link_flags})
  list(APPEND LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS ${shared_cache_link_flags})
endif()

# Build the runtime with -Wall to catch, e.g., uninitialized variables
# warnings.
if(LANGUAGE_COMPILER_IS_MSVC_LIKE)
  list(APPEND LANGUAGE_RUNTIME_CXX_FLAGS "/W3")
else()
  list(APPEND LANGUAGE_RUNTIME_CXX_FLAGS "-Wall")
endif()

set(LANGUAGE_RUNTIME_CORE_CXX_FLAGS "${LANGUAGE_RUNTIME_CXX_FLAGS}")
set(LANGUAGE_RUNTIME_CORE_LINK_FLAGS "${LANGUAGE_RUNTIME_LINK_FLAGS}")

if(CMAKE_SYSTEM_NAME STREQUAL "CYGWIN")
  list(APPEND LANGUAGE_RUNTIME_CORE_CXX_FLAGS "-mcmodel=large")
elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
  list(APPEND LANGUAGE_RUNTIME_CORE_CXX_FLAGS "-xc++")
endif()

# We should avoid non-literals in format strings, or appropriately mark
# functions.
check_cxx_compiler_flag("-Wformat-nonliteral -Werror=format-nonliteral" CXX_SUPPORTS_FORMAT_NONLITERAL_WARNING)
if (CXX_SUPPORTS_FORMAT_NONLITERAL_WARNING)
  list(APPEND LANGUAGE_RUNTIME_CORE_CXX_FLAGS "-Wformat-nonliteral"
    "-Werror=format-nonliteral")
endif()

# C++ code in the runtime and standard library should generally avoid
# introducing static constructors or destructors.
check_cxx_compiler_flag("-Wglobal-constructors -Werror=global-constructors" CXX_SUPPORTS_GLOBAL_CONSTRUCTORS_WARNING)
if(CXX_SUPPORTS_GLOBAL_CONSTRUCTORS_WARNING)
  list(APPEND LANGUAGE_RUNTIME_CORE_CXX_FLAGS "-Wglobal-constructors"
    "-Werror=global-constructors")
endif()

# C++ code in the runtime and standard library should generally avoid
# introducing static constructors or destructors.
check_cxx_compiler_flag("-Wexit-time-destructors -Werror=exit-time-destructors" CXX_SUPPORTS_EXIT_TIME_DESTRUCTORS_WARNING)
if(CXX_SUPPORTS_EXIT_TIME_DESTRUCTORS_WARNING)
  list(APPEND LANGUAGE_RUNTIME_CORE_CXX_FLAGS "-Wexit-time-destructors"
    "-Werror=exit-time-destructors")
endif()

add_subdirectory(CodiraShims/language/shims)
add_subdirectory(CommandLineSupport)
if(LANGUAGE_ENABLE_EXPERIMENTAL_CXX_INTEROP)
  add_subdirectory(Cxx)
else()
  # CodiraCompilerSources rely on C++ interop, specifically on being able to
  # import the C++ standard library into Codira. On Linux, this is only possible
  # when using the modulemap that Codira provides for libstdc++. Make sure we
  # include the libstdc++ modulemap into the build even when not building
  # C++ interop modules.
  add_subdirectory(Cxx/libstdcxx)
endif()
add_subdirectory(Threading)

# This static library is shared across languageCore and languageRemoteInspection
if(LANGUAGE_BUILD_STDLIB OR LANGUAGE_BUILD_REMOTE_MIRROR)
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
endif()

option(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB
      "Enable build of the embedded Codira standard library and runtime"
      TRUE)

option(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB_CROSS_COMPILING
      "Enable build of the embedded Codira standard library and runtime for cross-compiling targets"
      FALSE)

set(LANGUAGE_EMBEDDED_STDLIB_EXTRA_TARGET_TRIPLES "" CACHE STRING
    "List of extra target triples to build the embedded Codira standard library for")

if((NOT LANGUAGE_HOST_VARIANT STREQUAL "macosx") AND
  (NOT LANGUAGE_HOST_VARIANT STREQUAL "linux") AND
  (NOT LANGUAGE_HOST_VARIANT STREQUAL "wasi") AND
  (NOT LANGUAGE_HOST_VARIANT STREQUAL "windows"))
  set(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB FALSE)
elseif(BOOTSTRAPPING_MODE STREQUAL "OFF")
  set(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB FALSE)
endif()

if(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB)
  add_custom_target(embedded-libraries ALL)
endif()

set(EMBEDDED_STDLIB_TARGET_TRIPLES)

if(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB_CROSS_COMPILING)
  if(LANGUAGE_WASI_SYSROOT_PATH)
    # Don't attempt to build any other Embedded Codira stdlib triples
    # when building for WASI.
    list(APPEND EMBEDDED_STDLIB_TARGET_TRIPLES
      "wasm32    wasm32-unknown-wasip1  wasm32-unknown-wasip1"
    )
  else()
    if("ARM" IN_LIST TOOLCHAIN_TARGETS_TO_BUILD)
      list(APPEND EMBEDDED_STDLIB_TARGET_TRIPLES
        # arch    module_name               target triple
        "armv6    armv6-apple-none-macho    armv6-apple-none-macho"
        "armv6m   armv6m-apple-none-macho   armv6m-apple-none-macho"
        "armv7    armv7-apple-none-macho    armv7-apple-none-macho"
        "armv7m   armv7m-apple-none-macho   armv7m-apple-none-macho"
        "armv7em  armv7em-apple-none-macho  armv7em-apple-none-macho"
      )
    endif()
    if("AArch64" IN_LIST TOOLCHAIN_TARGETS_TO_BUILD)
      list(APPEND EMBEDDED_STDLIB_TARGET_TRIPLES
        "arm64    arm64-apple-none-macho    arm64-apple-none-macho"
        "arm64e   arm64e-apple-none-macho   arm64e-apple-none-macho"
        "arm64    arm64-apple-ios           arm64-apple-ios18"
        "arm64e   arm64e-apple-ios          arm64e-apple-ios18"
      )
    endif()

    # the following are all ELF targets
    if("ARM" IN_LIST TOOLCHAIN_TARGETS_TO_BUILD)
      list(APPEND EMBEDDED_STDLIB_TARGET_TRIPLES
        "armv4t   armv4t-none-none-eabi     armv4t-none-none-eabi"
        "armv6    armv6-none-none-eabi      armv6-none-none-eabi"
        "armv6m   armv6m-none-none-eabi     armv6m-none-none-eabi"
        "armv7    armv7-none-none-eabi      armv7-none-none-eabi"
        "armv7em  armv7em-none-none-eabi    armv7em-none-none-eabi"
      )
    endif()
    if("AArch64" IN_LIST TOOLCHAIN_TARGETS_TO_BUILD)
      list(APPEND EMBEDDED_STDLIB_TARGET_TRIPLES
        "aarch64  aarch64-none-none-elf     aarch64-none-none-elf"
      )
    endif()
    if("RISCV" IN_LIST TOOLCHAIN_TARGETS_TO_BUILD)
      list(APPEND EMBEDDED_STDLIB_TARGET_TRIPLES
        "riscv32  riscv32-none-none-eabi    riscv32-none-none-eabi"
        "riscv64  riscv64-none-none-eabi    riscv64-none-none-eabi"
      )
    endif()
    if("X86" IN_LIST TOOLCHAIN_TARGETS_TO_BUILD)
      list(APPEND EMBEDDED_STDLIB_TARGET_TRIPLES
        "i686    i686-unknown-none-elf       i686-unknown-none-elf"
        "x86_64  x86_64-unknown-none-elf     x86_64-unknown-none-elf"
      )
    endif()

    if("WebAssembly" IN_LIST TOOLCHAIN_TARGETS_TO_BUILD)
      list(APPEND EMBEDDED_STDLIB_TARGET_TRIPLES
        "wasm32    wasm32-unknown-none-wasm    wasm32-unknown-none-wasm"
        "wasm64    wasm64-unknown-none-wasm    wasm64-unknown-none-wasm"
      )
    endif()

    if("AVR" IN_LIST TOOLCHAIN_TARGETS_TO_BUILD)
      list(APPEND EMBEDDED_STDLIB_TARGET_TRIPLES
        "avr  avr-none-none-elf    avr-none-none-elf"
      )
    endif()
  endif()
endif()

if(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB AND NOT LANGUAGE_WASI_SYSROOT_PATH)
  if (LANGUAGE_HOST_VARIANT STREQUAL "linux")
    list(APPEND EMBEDDED_STDLIB_TARGET_TRIPLES
      "${LANGUAGE_HOST_VARIANT_ARCH} ${LANGUAGE_HOST_VARIANT_ARCH}-unknown-linux-gnu ${LANGUAGE_HOST_VARIANT_ARCH}-unknown-linux-gnu"
      # In practice this expands to either:
      # "x86_64   x86_64-unknown-linux-gnu  x86_64-unknown-linux-gnu"
      # "aarch64  aarch64-unknown-linux-gnu aarch64-unknown-linux-gnu"
    )
  elseif (LANGUAGE_HOST_VARIANT STREQUAL "macosx")
    list(APPEND EMBEDDED_STDLIB_TARGET_TRIPLES
      "x86_64   x86_64-apple-macos        x86_64-apple-macos14"
      "arm64    arm64-apple-macos         arm64-apple-macos14"
      "arm64e   arm64e-apple-macos        arm64e-apple-macos14"
    )
  elseif (LANGUAGE_HOST_VARIANT STREQUAL "windows")
    list(APPEND EMBEDDED_STDLIB_TARGET_TRIPLES
      # Without specifying the `windows` component LLVM refuses to
      # generate valid COFF object files.
      "i686    i686-unknown-windows-msvc   i686-unknown-windows-msvc"
      "x86_64  x86_64-unknown-windows-msvc x86_64-unknown-windows-msvc"
    )
  endif()
endif()

foreach(triple ${LANGUAGE_EMBEDDED_STDLIB_EXTRA_TARGET_TRIPLES})
  if(triple STREQUAL "")
    continue()
  endif()

  string(REGEX REPLACE "-" ";" list "${triple}")
  list(GET list 0 arch)
  list(GET list 1 vendor)
  list(GET list 2 os)
  string(REGEX REPLACE "[0-9]+(\\.[0-9]+)+" " " mod "${triple}")

  list(FILTER EMBEDDED_STDLIB_TARGET_TRIPLES EXCLUDE REGEX " ${mod} ")

  list(APPEND EMBEDDED_STDLIB_TARGET_TRIPLES
    "${arch} ${mod} ${triple}"
  )
endforeach()

if(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB)
  set(triples)
  foreach(entry ${EMBEDDED_STDLIB_TARGET_TRIPLES})
    string(REGEX REPLACE "[ \t]+" ";" list "${entry}")
    list(GET list 2 triple)
    list(APPEND triples ${triple})
  endforeach()

  message(STATUS "Building Embedded Codira standard libraries for targets: ${triples}")
  message(STATUS "")
endif()

if(LANGUAGE_BUILD_STDLIB)
  # These must be kept in dependency order so that any referenced targets
  # exist at the time we look for them in add_language_*.
  add_subdirectory(runtime)
  add_subdirectory(stubs)
  add_subdirectory(core)
  add_subdirectory(CodiraOnoneSupport)

  if(LANGUAGE_BUILD_CLANG_OVERLAYS OR LANGUAGE_BUILD_TEST_SUPPORT_MODULES)
    add_subdirectory(ClangOverlays)
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY)
    add_subdirectory(Concurrency)
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_STRING_PROCESSING)
    add_subdirectory(RegexParser)
    add_subdirectory(StringProcessing)
    add_subdirectory(RegexBuilder)
  endif()
endif()

if(LANGUAGE_BUILD_STDLIB AND NOT LANGUAGE_STDLIB_BUILD_ONLY_CORE_MODULES)
  # In some internal Apple configurations we have the need
  # to build Core and Onone separately from the rest
  # of the stdlib
  if(LANGUAGE_ENABLE_EXPERIMENTAL_DIFFERENTIABLE_PROGRAMMING)
    add_subdirectory(Differentiation)
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_DISTRIBUTED)
    add_subdirectory(Distributed)
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_OBSERVATION)
    add_subdirectory(Observation)
  endif()

  if(LANGUAGE_ENABLE_RUNTIME_MODULE)
    add_subdirectory(RuntimeModule)
  endif()

  if(LANGUAGE_ENABLE_SYNCHRONIZATION)
    add_subdirectory(Synchronization)
  endif()

  if(LANGUAGE_ENABLE_VOLATILE)
    add_subdirectory(Volatile)
  endif()
endif()

if(LANGUAGE_BUILD_REMOTE_MIRROR)
  add_subdirectory(RemoteInspection)
  add_subdirectory(CodiraRemoteMirror)
endif()

if(LANGUAGE_BUILD_SDK_OVERLAY OR (LANGUAGE_BUILD_TEST_SUPPORT_MODULES AND LANGUAGE_BUILD_DYNAMIC_SDK_OVERLAY_default))
  add_subdirectory(Platform)
endif()

if(LANGUAGE_BUILD_SDK_OVERLAY)
  # On Apple platforms, we aren't building any overlays. Instead, we're picking them up from the SDK.

  if(WINDOWS IN_LIST LANGUAGE_SDKS)
    add_subdirectory(Windows)
  endif()
endif()

if(LANGUAGE_BUILD_LIBEXEC)
add_subdirectory(libexec)
endif()

