set(TOOLCHAIN_ENABLE_PROJECTS
      clang
      clang-tools-extra
      lld
      lldb
    CACHE STRING "")

set(TOOLCHAIN_EXTERNAL_PROJECTS
      cmark
      language
    CACHE STRING "")

set(TOOLCHAIN_ENABLE_RUNTIMES
      compiler-rt
    CACHE STRING "")

# NOTE(compnerd) always enable assertions, the toolchain will not provide enough
# context to resolve issues otherwise and may silently generate invalid output.
set(TOOLCHAIN_ENABLE_ASSERTIONS YES CACHE BOOL "")

set(ENABLE_X86_RELAX_RELOCATIONS YES CACHE BOOL "")

# NOTE(compnerd) we can hardcode the default target triple since the cache files
# are target dependent.
set(TOOLCHAIN_DEFAULT_TARGET_TRIPLE x86_64-unknown-linux-gnu CACHE STRING "")

set(TOOLCHAIN_APPEND_VC_REV NO CACHE BOOL "")
set(TOOLCHAIN_ENABLE_PER_TARGET_RUNTIME_DIR YES CACHE BOOL "")
set(TOOLCHAIN_ENABLE_PYTHON YES CACHE BOOL "")
set(TOOLCHAIN_BUILTIN_TARGETS
      x86_64-unknown-linux-gnu
    CACHE STRING "")
foreach(target ${TOOLCHAIN_RUNTIME_TARGETS})
  set(BUILTINS_${target}_TOOLCHAIN_ENABLE_RUNTIMES
        compiler-rt
      CACHE STRING "")
  set(BUILTINS_${target}_CMAKE_SYSTEM_NAME Linux CACHE STRING "")
  set(BUILTINS_${target}_CMAKE_BUILD_TYPE Release CACHE STRING "")
  set(BUILTINS_${target}_COMPILER_RT_BUILD_CRT NO CACHE BOOL "")
  set(BUILTINS_${target}_COMPILER_RT_BUILD_LIBFUZZER NO CACHE BOOL "")
  set(BUILTINS_${target}_COMPILER_RT_BUILD_PROFILE YES CACHE BOOL "")
  set(BUILTINS_${target}_COMPILER_RT_BUILD_SANITIZERS NO CACHE BOOL "")
  set(BUILTINS_${target}_COMPILER_RT_BUILD_XRAY NO CACHE BOOL "")
  set(BUILTINS_${target}_COMPILER_RT_DEFAULT_TARGET_ONLY YES CACHE BOOL "")
  set(BUILTINS_${target}_COMPILER_RT_INCLUDE_TESTS NO CACHE BOOL "")
endforeach()

set(TOOLCHAIN_TARGETS_TO_BUILD AArch64 ARM WebAssembly X86 CACHE STRING "")

# Disable certain targets to reduce the configure time or to avoid configuration
# differences (and in some cases weird build errors on a complete build).
set(TOOLCHAIN_BUILD_TOOLCHAIN_DYLIB NO CACHE BOOL "")
set(TOOLCHAIN_BUILD_TOOLCHAIN_C_DYLIB NO CACHE BOOL "")
set(TOOLCHAIN_ENABLE_LIBEDIT NO CACHE BOOL "")
set(TOOLCHAIN_ENABLE_LIBXML2 NO CACHE BOOL "")
set(TOOLCHAIN_ENABLE_OCAMLDOC NO CACHE BOOL "")
set(TOOLCHAIN_ENABLE_TERMINFO NO CACHE BOOL "")
set(TOOLCHAIN_ENABLE_Z3_SOLVER NO CACHE BOOL "")
set(TOOLCHAIN_ENABLE_ZLIB NO CACHE BOOL "")
set(TOOLCHAIN_INCLUDE_BENCHMARKS NO CACHE BOOL "")
set(TOOLCHAIN_INCLUDE_DOCS NO CACHE BOOL "")
set(TOOLCHAIN_INCLUDE_EXAMPLES NO CACHE BOOL "")
set(TOOLCHAIN_INCLUDE_GO_TESTS NO CACHE BOOL "")
set(TOOLCHAIN_TOOL_GOLD_BUILD NO CACHE BOOL "")
set(TOOLCHAIN_TOOL_TOOLCHAIN_SHLIB_BUILD NO CACHE BOOL "")

# Avoid swig dependency for lldb
set(LLDB_ALLOW_STATIC_BINDINGS YES CACHE BOOL "")
set(LLDB_USE_STATIC_BINDINGS YES CACHE BOOL "")

# This requires perl which may not be available on Windows
set(LANGUAGE_INCLUDE_DOCS NO CACHE BOOL "")
set(LANGUAGE_BUILD_ENABLE_PARSER_LIB YES CACHE BOOL "")
# static linking is not supported on Windows yet
set(LANGUAGE_BUILD_STATIC_STDLIB NO CACHE BOOL "")
set(LANGUAGE_BUILD_STATIC_SDK_OVERLAY NO CACHE BOOL "")

set(TOOLCHAIN_INSTALL_BINUTILS_SYMLINKS YES CACHE BOOL "")
set(TOOLCHAIN_INSTALL_TOOLCHAIN_ONLY YES CACHE BOOL "")
set(TOOLCHAIN_TOOLCHAIN_TOOLS
      addr2line
      ar
      c++filt
      dsymutil
      dwp
      # lipo
      toolchain-ar
      toolchain-cov
      toolchain-cvtres
      toolchain-cxxfilt
      toolchain-dlltool
      toolchain-dwarfdump
      toolchain-dwp
      toolchain-lib
      toolchain-lipo
      toolchain-mt
      toolchain-nm
      toolchain-objcopy
      toolchain-objdump
      toolchain-pdbutil
      toolchain-profdata
      toolchain-ranlib
      toolchain-rc
      toolchain-readelf
      toolchain-readobj
      toolchain-size
      toolchain-strings
      toolchain-strip
      toolchain-symbolizer
      toolchain-undname
      nm
      objcopy
      objdump
      ranlib
      readelf
      size
      strings
    CACHE STRING "")

set(CLANG_TOOLS
      clang
      clangd
      clang-format
      clang-resource-headers
      clang-tidy
    CACHE STRING "")

set(LLD_TOOLS
      lld
    CACHE STRING "")

set(LLDB_TOOLS
      liblldb
      lldb
      lldb-argdumper
      lldb-python-scripts
      lldb-server
      lldb-dap
      repl_language
    CACHE STRING "")

set(LANGUAGE_INSTALL_COMPONENTS
      autolink-driver
      compiler
      compiler-language-syntax-lib
      clang-builtin-headers
      editor-integration
      tools
      sourcekit-inproc
      language-remote-mirror
      language-remote-mirror-headers
      language-syntax-lib
    CACHE STRING "")

set(TOOLCHAIN_DISTRIBUTION_COMPONENTS
      IndexStore
      libclang
      libclang-headers
      LTO
      runtimes
      ${TOOLCHAIN_TOOLCHAIN_TOOLS}
      ${CLANG_TOOLS}
      ${LLD_TOOLS}
      ${LLDB_TOOLS}
      ${LANGUAGE_INSTALL_COMPONENTS}
    CACHE STRING "")
