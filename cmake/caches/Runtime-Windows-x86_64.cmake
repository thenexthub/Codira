
set(LANGUAGE_HOST_VARIANT_SDK WINDOWS CACHE STRING "")
set(LANGUAGE_HOST_VARIANT_ARCH x86_64 CACHE STRING "")

# NOTE(compnerd) disable the tools, we are trying to build just the standard
# library.
set(LANGUAGE_INCLUDE_TOOLS NO CACHE BOOL "")

# NOTE(compnerd) cannot build tests since the tests require the toolchain
set(LANGUAGE_INCLUDE_TESTS NO CACHE BOOL "")

# NOTE(compnerd) cannot build docs since that requires perl
set(LANGUAGE_INCLUDE_DOCS NO CACHE BOOL "")

# NOTE(compnerd) these are part of the toolchain, not the runtime.
set(LANGUAGE_BUILD_SOURCEKIT NO CACHE BOOL "")

# NOTE(compnerd) build with the compiler specified, not a just built compiler.
set(LANGUAGE_BUILD_RUNTIME_WITH_HOST_COMPILER YES CACHE BOOL "")
