# In some configurations (e.g. back deploy concurrency) we
# configure the build from the root of the Codira repo but we skip
# stdlib/CMakeLists.txt, with the risk of missing important parameters.
# To account for this scenario, we include the stdlib options
# before the guard
include(${CMAKE_CURRENT_LIST_DIR}/../../stdlib/cmake/modules/StdlibOptions.cmake)

# CMAKE_SOURCE_DIR is the directory that cmake got initially invoked on.
# CMAKE_CURRENT_SOURCE_DIR is the current directory. If these are equal, it's
# a top-level build of the CMAKE_SOURCE_DIR. Otherwise, define a guard variable
# and return.
if(DEFINED LANGUAGE_MASTER_LOADED
    OR NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(LANGUAGE_MASTER_LOADED TRUE)
  return()
endif()


list(APPEND CMAKE_MODULE_PATH
  "${LANGUAGE_SOURCE_ROOT}/toolchain-project/toolchain/cmake/modules"
  "${PROJECT_SOURCE_DIR}/../../../../cmake/modules"
  "${PROJECT_SOURCE_DIR}/../../../cmake/modules")


# -----------------------------------------------------------------------------
# Preconditions

include(CodiraUtils)

precondition(CMAKE_INSTALL_PREFIX)
precondition(LANGUAGE_DEST_ROOT)
precondition(LANGUAGE_HOST_VARIANT_SDK)
precondition(LANGUAGE_SOURCE_ROOT)
precondition(TOOLCHAIN_DIR)


# -----------------------------------------------------------------------------
# Cache Variables and Options

set(LANGUAGE_SOURCE_DIR "${LANGUAGE_SOURCE_ROOT}/language" CACHE PATH
  "Path to the directory containing the Codira sources.")

set(LANGUAGE_DARWIN_XCRUN_TOOLCHAIN "XcodeDefault" CACHE STRING
  "The name of the toolchain to pass to 'xcrun'.")

set(LANGUAGE_DARWIN_DEPLOYMENT_VERSION_OSX "10.9" CACHE STRING
    "Minimum deployment target version for macOS.")
set(LANGUAGE_DARWIN_DEPLOYMENT_VERSION_IOS "7.0" CACHE STRING
    "Minimum deployment target version for iOS.")
set(LANGUAGE_DARWIN_DEPLOYMENT_VERSION_TVOS "9.0" CACHE STRING
    "Minimum deployment target version for tvOS.")
set(LANGUAGE_DARWIN_DEPLOYMENT_VERSION_WATCHOS "2.0" CACHE STRING
    "Minimum deployment target version for watchOS.")

set(LANGUAGE_INSTALL_COMPONENTS "sdk-overlay" CACHE STRING
  "A semicolon-separated list of install components.")

set(LANGUAGE_SDKS "${LANGUAGE_HOST_VARIANT_SDK}" CACHE STRING
  "List of Codira SDKs to build.")

set(LANGUAGE_NATIVE_TOOLCHAIN_TOOLS_PATH "${TOOLCHAIN_DIR}/usr/bin" CACHE STRING
  "Path to LLVM tools that are executable on the build machine.")
set(LANGUAGE_NATIVE_CLANG_TOOLS_PATH "${TOOLCHAIN_DIR}/usr/bin" CACHE STRING
  "Path to Clang tools that are executable on the build machine.")
set(LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH "${TOOLCHAIN_DIR}/usr/bin" CACHE STRING
  "Path to Codira tools that are executable on the build machine.")

# NOTE: The initialization in stdlib/CMakeLists.txt will be bypassed if we
# directly invoke CMake for this directory, so we initialize the variables
# related to library evolution here as well.

option(LANGUAGE_STDLIB_STABLE_ABI
  "Should stdlib be built with stable ABI (library evolution, resilience)."
  TRUE)

option(LANGUAGE_ENABLE_MODULE_INTERFACES
  "Generate .codeinterface files alongside .codemodule files."
  "${LANGUAGE_STDLIB_STABLE_ABI}")

set(LANGUAGE_STDLIB_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
  "Build type for the Codira standard library and SDK overlays.")

file(STRINGS "../../utils/availability-macros.def" LANGUAGE_STDLIB_AVAILABILITY_DEFINITIONS)
list(FILTER LANGUAGE_STDLIB_AVAILABILITY_DEFINITIONS EXCLUDE REGEX "^\\s*(#.*)?$")

set(LANGUAGE_DARWIN_SUPPORTED_ARCHS "" CACHE STRING
  "Semicolon-separated list of architectures to configure on Darwin platforms. \
If left empty all default architectures are configured.")

set(LANGUAGE_DARWIN_MODULE_ARCHS "" CACHE STRING
  "Semicolon-separated list of architectures to configure Codira module-only \
targets on Darwin platforms. These targets are in addition to the full \
library targets.")

# -----------------------------------------------------------------------------
# Constants

set(CMAKE_INSTALL_PREFIX
  "${LANGUAGE_DEST_ROOT}${TOOLCHAIN_DIR}/usr")


set(LANGUAGE_DARWIN_PLATFORMS
  OSX IOS IOS_SIMULATOR TVOS TVOS_SIMULATOR WATCHOS WATCHOS_SIMULATOR XROS XROS_SIMULATOR)

# Flags used to indicate we are building a standalone overlay.
# FIXME: We should cut this down to a single flag.
set(BUILD_STANDALONE TRUE)
set(LANGUAGE_BUILD_STANDALONE_OVERLAY TRUE)

set(LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES "SHARED")
set(LANGUAGE_SDK_OVERLAY_LIBRARY_BUILD_TYPES "SHARED")

option(LANGUAGE_ENABLE_MACCATALYST
  "Build the overlays with macCatalyst support"
  FALSE)

set(LANGUAGE_DARWIN_DEPLOYMENT_VERSION_MACCATALYST "13.0" CACHE STRING
  "Minimum deployment target version for macCatalyst")

# -----------------------------------------------------------------------------

include(CodiraToolchainUtils)
if(NOT LANGUAGE_LIPO)
  find_toolchain_tool(LANGUAGE_LIPO "${LANGUAGE_DARWIN_XCRUN_TOOLCHAIN}" lipo)
endif()

include(AddLLVM)
include(CodiraSharedCMakeConfig)
include(AddCodira)
include(CodiraHandleGybSources)
include(CodiraConfigureSDK)
include(CodiraComponents)
include(DarwinSDKs)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Without this line, installing components is broken. This needs refactoring.
language_configure_components()


list_subtract(
  "${LANGUAGE_SDKS}"
  "${LANGUAGE_CONFIGURED_SDKS}"
  unknown_sdks)

precondition(unknown_sdks NEGATE
  MESSAGE
    "Unknown SDKs: ${unknown_sdks}")


# Some overlays include the runtime's headers, and some of those headers are
# generated at build time.
add_subdirectory("${LANGUAGE_SOURCE_DIR}/include" "${LANGUAGE_SOURCE_DIR}/include")
add_subdirectory("${LANGUAGE_SOURCE_DIR}/apinotes" "${LANGUAGE_SOURCE_DIR}/apinotes")
