# Variable that tracks the set of configured SDKs.
#
# Each element in this list is an SDK for which the various
# LANGUAGE_SDK_${name}_* variables are defined. Codira libraries will be
# built for each variant.
set(LANGUAGE_CONFIGURED_SDKS)

include(CodiraWindowsSupport)
include(CodiraAndroidSupport)

# Report the given SDK to the user.
function(_report_sdk prefix)
  message(STATUS "${LANGUAGE_SDK_${prefix}_NAME} SDK:")
  message(STATUS "  Object File Format: ${LANGUAGE_SDK_${prefix}_OBJECT_FORMAT}")
  message(STATUS "  Codira Standard Library Path: ${LANGUAGE_SDK_${prefix}_LIB_SUBDIR}")
  message(STATUS "  Threading Package: ${LANGUAGE_SDK_${prefix}_THREADING_PACKAGE}")

  message(STATUS "  Static linking supported: ${LANGUAGE_SDK_${prefix}_STATIC_LINKING_SUPPORTED}")
  message(STATUS "  Static link only: ${LANGUAGE_SDK_${prefix}_STATIC_ONLY}")

  if("${prefix}" STREQUAL "WINDOWS")
    message(STATUS "  UCRT Version: ${UCRTVersion}")
    message(STATUS "  UCRT SDK Path: ${UniversalCRTSdkDir}")
    message(STATUS "  VC Path: ${VCToolsInstallDir}")
    if("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
      message(STATUS "  ${CMAKE_BUILD_TYPE} VC++ CRT: MDd")
    else()
      message(STATUS "  ${CMAKE_BUILD_TYPE} VC++ CRT: MD")
    endif()
  endif()
  if(prefix IN_LIST LANGUAGE_DARWIN_PLATFORMS)
    message(STATUS "  Version: ${LANGUAGE_SDK_${prefix}_VERSION}")
    message(STATUS "  Build number: ${LANGUAGE_SDK_${prefix}_BUILD_NUMBER}")
    message(STATUS "  Deployment version: ${LANGUAGE_SDK_${prefix}_DEPLOYMENT_VERSION}")
    message(STATUS "  Triple name: ${LANGUAGE_SDK_${prefix}_TRIPLE_NAME}")
    message(STATUS "  Simulator: ${LANGUAGE_SDK_${prefix}_IS_SIMULATOR}")
  endif()
  if(LANGUAGE_SDK_${prefix}_MODULE_ARCHITECTURES)
    message(STATUS "  Module Architectures: ${LANGUAGE_SDK_${prefix}_MODULE_ARCHITECTURES}")
  endif()

  message(STATUS "  Architectures: ${LANGUAGE_SDK_${prefix}_ARCHITECTURES}")
  foreach(arch ${LANGUAGE_SDK_${prefix}_ARCHITECTURES})
    message(STATUS "  ${arch} triple: ${LANGUAGE_SDK_${prefix}_ARCH_${arch}_TRIPLE}")
    message(STATUS "  Module triple: ${LANGUAGE_SDK_${prefix}_ARCH_${arch}_MODULE}")
  endforeach()
  if("${prefix}" STREQUAL "WINDOWS")
    foreach(arch ${LANGUAGE_SDK_${prefix}_ARCHITECTURES})
      language_windows_include_for_arch(${arch} ${arch}_INCLUDE)
      language_windows_lib_for_arch(${arch} ${arch}_LIB)
      message(STATUS "  ${arch} INCLUDE: ${${arch}_INCLUDE}")
      message(STATUS "  ${arch} LIB: ${${arch}_LIB}")
    endforeach()
  elseif("${prefix}" STREQUAL "ANDROID")
    if(NOT "${LANGUAGE_ANDROID_NDK_PATH}" STREQUAL "")
      message(STATUS " NDK: ${LANGUAGE_ANDROID_NDK_PATH}")
    endif()
    if(NOT "${LANGUAGE_ANDROID_NATIVE_SYSROOT}" STREQUAL "")
      message(STATUS " Sysroot: ${LANGUAGE_ANDROID_NATIVE_SYSROOT}")
    endif()
  else()
    foreach(arch ${LANGUAGE_SDK_${prefix}_ARCHITECTURES})
      message(STATUS "  ${arch} Path: ${LANGUAGE_SDK_${prefix}_ARCH_${arch}_PATH}")
    endforeach()
  endif()

  message(STATUS "")
endfunction()

# Remove architectures not supported by the SDK from the given list.
function(remove_sdk_unsupported_archs name os sdk_path deployment_version architectures_var)
  execute_process(COMMAND
      /usr/libexec/PlistBuddy -c "Print :SupportedTargets:${os}:Archs" ${sdk_path}/SDKSettings.plist
    OUTPUT_VARIABLE sdk_supported_archs
    RESULT_VARIABLE plist_error)

  if (NOT plist_error EQUAL 0)
    message(STATUS "${os} SDK at ${sdk_path} does not publish its supported architectures")
    return()
  endif()

  set(architectures)
  foreach(arch ${${architectures_var}})
    if(sdk_supported_archs MATCHES "${arch}\n")
      list(APPEND architectures ${arch})
    elseif(arch STREQUAL "i386" AND os STREQUAL "iphonesimulator")
      # 32-bit iOS simulator is not listed explicitly in SDK settings.
      message(STATUS "Assuming ${name} SDK at ${sdk_path} supports architecture ${arch}")
      list(APPEND architectures ${arch})
    elseif(arch STREQUAL "armv7k" AND os STREQUAL "watchos")
      # 32-bit watchOS is not listed explicitly in SDK settings.
      message(STATUS "Assuming ${name} SDK at ${sdk_path} supports architecture ${arch}")
      list(APPEND architectures ${arch})
    else()
      message(STATUS "${name} SDK at ${sdk_path} does not support architecture ${arch}")
    endif()
  endforeach()

  set("${architectures_var}" ${architectures} PARENT_SCOPE)
endfunction()

# Work out which threading package to use by consulting LANGUAGE_THREADING_PACKAGE
function(get_threading_package sdk default package_var)
  set(global_override)
  foreach(elt ${LANGUAGE_THREADING_PACKAGE})
    string(REPLACE ":" ";" elt_list "${elt}")
    list(LENGTH elt_list elt_list_len)
    if(elt_list_len EQUAL 1)
      list(GET elt_list 0 global_override)
      string(TOLOWER "${global_override}" global_override)
    else()
      list(GET elt_list 0 elt_sdk)
      list(GET elt_list 1 elt_package)
      string(TOUPPER "${elt_sdk}" elt_sdk)
      string(TOLOWER "${elt_package}" elt_package)

      if("${elt_sdk}" STREQUAL "${sdk}")
        set("${package_var}" "${elt_package}" PARENT_SCOPE)
        return()
      endif()
    endif()
  endforeach()
  if(global_override)
    set("${package_var}" "${global_override}" PARENT_SCOPE)
  else()
    set("${package_var}" "${default}" PARENT_SCOPE)
  endif()
endfunction()

# Configure an SDK
#
# Usage:
#   configure_sdk_darwin(
#     prefix             # Prefix to use for SDK variables (e.g., OSX)
#     name               # Display name for this SDK
#     deployment_version # Deployment version
#     xcrun_name         # SDK name to use with xcrun
#     triple_name        # The name used in Codira's -triple
#     availability_name  # The name used in Codira's @availability
#     architectures      # A list of architectures this SDK supports
#   )
#
# Sadly there are three OS naming conventions.
# xcrun SDK name:   macosx iphoneos iphonesimulator (+ version)
# -mOS-version-min: macosx ios      ios-simulator
# language -triple:    macosx ios      ios
#
# This macro attempts to configure a given SDK. When successful, it
# defines a number of variables:
#
#   LANGUAGE_SDK_${prefix}_NAME                      Display name for the SDK
#   LANGUAGE_SDK_${prefix}_VERSION                   SDK version number (e.g., 10.9, 7.0)
#   LANGUAGE_SDK_${prefix}_BUILD_NUMBER              SDK build number (e.g., 14A389a)
#   LANGUAGE_SDK_${prefix}_DEPLOYMENT_VERSION        Deployment version (e.g., 10.9, 7.0)
#   LANGUAGE_SDK_${prefix}_LIB_SUBDIR                Library subdir for this SDK
#   LANGUAGE_SDK_${prefix}_TRIPLE_NAME               Triple name for this SDK
#   LANGUAGE_SDK_${prefix}_OBJECT_FORMAT             The object file format (e.g. MACHO)
#   LANGUAGE_SDK_${prefix}_USE_ISYSROOT              Whether to use -isysroot
#   LANGUAGE_SDK_${prefix}_SHARED_LIBRARY_PREFIX     Shared library prefix for this SDK (e.g. 'lib')
#   LANGUAGE_SDK_${prefix}_SHARED_LIBRARY_SUFFIX     Shared library suffix for this SDK (e.g. 'dylib')
#   LANGUAGE_SDK_${prefix}_STATIC_LINKING_SUPPORTED  Whether static linking is supported for this SDK
#   LANGUAGE_SDK_${prefix}_STATIC_ONLY               Whether to build *only* static libraries

#   LANGUAGE_SDK_${prefix}_ARCHITECTURES             Architectures (as a list)
#   LANGUAGE_SDK_${prefix}_IS_SIMULATOR              Whether this is a simulator target.
#   LANGUAGE_SDK_${prefix}_ARCH_${ARCH}_TRIPLE       Triple name
#   LANGUAGE_SDK_${prefix}_ARCH_${ARCH}_MODULE       Module triple name for this SDK
#   LANGUAGE_SDK_${prefix}_USE_BUILD_ID              Whether to pass --build-id to the linker
#   LANGUAGE_SDK_${prefix}_AVAILABILITY_NAME       Name to use in @availability
#
macro(configure_sdk_darwin
    prefix name deployment_version xcrun_name
    triple_name module_name availability_name architectures)
  # Note: this has to be implemented as a macro because it sets global
  # variables.

  # Find the SDK
  set(LANGUAGE_SDK_${prefix}_PATH "" CACHE PATH "Path to the ${name} SDK")

  if(NOT LANGUAGE_SDK_${prefix}_PATH)
    execute_process(
        COMMAND "xcrun" "--sdk" "${xcrun_name}" "--show-sdk-path"
        OUTPUT_VARIABLE LANGUAGE_SDK_${prefix}_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()

  if(NOT EXISTS "${LANGUAGE_SDK_${prefix}_PATH}/SDKSettings.plist")
    message(FATAL_ERROR "${name} SDK not found at LANGUAGE_SDK_${prefix}_PATH.")
  endif()

  # Determine the SDK version we found.
  execute_process(
    COMMAND "defaults" "read" "${LANGUAGE_SDK_${prefix}_PATH}/SDKSettings.plist" "Version"
      OUTPUT_VARIABLE LANGUAGE_SDK_${prefix}_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(
    COMMAND "xcodebuild" "-sdk" "${LANGUAGE_SDK_${prefix}_PATH}" "-version" "ProductBuildVersion"
      OUTPUT_VARIABLE LANGUAGE_SDK_${prefix}_BUILD_NUMBER
      OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Set other variables.
  set(LANGUAGE_SDK_${prefix}_NAME "${name}")
  set(LANGUAGE_SDK_${prefix}_DEPLOYMENT_VERSION "${deployment_version}")
  set(LANGUAGE_SDK_${prefix}_LIB_SUBDIR "${xcrun_name}")
  set(LANGUAGE_SDK_${prefix}_TRIPLE_NAME "${triple_name}")
  set(LANGUAGE_SDK_${prefix}_AVAILABILITY_NAME "${availability_name}")
  set(LANGUAGE_SDK_${prefix}_OBJECT_FORMAT "MACHO")
  set(LANGUAGE_SDK_${prefix}_USE_ISYSROOT TRUE)
  set(LANGUAGE_SDK_${prefix}_SHARED_LIBRARY_PREFIX "lib")
  set(LANGUAGE_SDK_${prefix}_SHARED_LIBRARY_SUFFIX ".dylib")
  set(LANGUAGE_SDK_${prefix}_STATIC_LIBRARY_PREFIX "lib")
  set(LANGUAGE_SDK_${prefix}_STATIC_LIBRARY_SUFFIX ".a")
  set(LANGUAGE_SDK_${prefix}_IMPORT_LIBRARY_PREFIX "")
  set(LANGUAGE_SDK_${prefix}_IMPORT_LIBRARY_SUFFIX "")
  set(LANGUAGE_SDK_${prefix}_STATIC_LINKING_SUPPORTED FALSE)
  set(LANGUAGE_SDK_${prefix}_STATIC_ONLY FALSE)
  get_threading_package(${prefix} "darwin" LANGUAGE_SDK_${prefix}_THREADING_PACKAGE)

  # On Darwin we get UUIDs automatically, without the --build-id flag
  set(LANGUAGE_SDK_${prefix}_USE_BUILD_ID FALSE)

  set(LANGUAGE_SDK_${prefix}_ARCHITECTURES ${architectures})
  if(LANGUAGE_DARWIN_SUPPORTED_ARCHS)
    list_intersect(
      "${architectures}"                  # lhs
      "${LANGUAGE_DARWIN_SUPPORTED_ARCHS}"   # rhs
      LANGUAGE_SDK_${prefix}_ARCHITECTURES)  # result
  endif()

  # Remove any architectures not supported by the SDK.
  remove_sdk_unsupported_archs(${name} ${xcrun_name} ${LANGUAGE_SDK_${prefix}_PATH} "${LANGUAGE_SDK_${prefix}_DEPLOYMENT_VERSION}" LANGUAGE_SDK_${prefix}_ARCHITECTURES)

  list_intersect(
    "${LANGUAGE_DARWIN_MODULE_ARCHS}"            # lhs
    "${architectures}"                        # rhs
    LANGUAGE_SDK_${prefix}_MODULE_ARCHITECTURES) # result

  # Ensure the architectures and module-only architectures lists are mutually
  # exclusive.
  list_subtract(
    "${LANGUAGE_SDK_${prefix}_MODULE_ARCHITECTURES}" # lhs
    "${LANGUAGE_SDK_${prefix}_ARCHITECTURES}"        # rhs
    LANGUAGE_SDK_${prefix}_MODULE_ARCHITECTURES)     # result

  # Determine whether this is a simulator SDK.
  if("${xcrun_name}" MATCHES "simulator")
    set(LANGUAGE_SDK_${prefix}_IS_SIMULATOR TRUE)
  else()
    set(LANGUAGE_SDK_${prefix}_IS_SIMULATOR FALSE)
  endif()

  # Configure variables for _all_ architectures even if we aren't "building"
  # them because they aren't supported.
  foreach(arch ${architectures})
    # On Darwin, all archs share the same SDK path.
    set(LANGUAGE_SDK_${prefix}_ARCH_${arch}_PATH "${LANGUAGE_SDK_${prefix}_PATH}")

    set(LANGUAGE_SDK_${prefix}_ARCH_${arch}_TRIPLE
        "${arch}-apple-${LANGUAGE_SDK_${prefix}_TRIPLE_NAME}")

    set(LANGUAGE_SDK_${prefix}_ARCH_${arch}_MODULE
        "${arch}-apple-${module_name}")

    # If this is a simulator target, append -simulator.
    if (LANGUAGE_SDK_${prefix}_IS_SIMULATOR)
      set(LANGUAGE_SDK_${prefix}_ARCH_${arch}_TRIPLE
          "${LANGUAGE_SDK_${prefix}_ARCH_${arch}_TRIPLE}-simulator")
    endif ()

    if(LANGUAGE_ENABLE_MACCATALYST AND "${prefix}" STREQUAL "OSX")
      # For macCatalyst append the '-macabi' environment to the target triple.
      set(LANGUAGE_SDK_MACCATALYST_ARCH_${arch}_TRIPLE "${arch}-apple-ios-macabi")
      set(LANGUAGE_SDK_MACCATALYST_ARCH_${arch}_MODULE "${arch}-apple-ios-macabi")

      # For macCatalyst, the xcrun_name is "macosx" since it uses that sdk.
      # Hard code the library subdirectory to "maccatalyst" in that case.
      set(LANGUAGE_SDK_MACCATALYST_LIB_SUBDIR "maccatalyst")
    endif()
  endforeach()

  # Add this to the list of known SDKs.
  list(APPEND LANGUAGE_CONFIGURED_SDKS "${prefix}")

  _report_sdk("${prefix}")
endmacro()

macro(configure_sdk_unix name architectures)
  # Note: this has to be implemented as a macro because it sets global
  # variables.

  string(TOUPPER ${name} prefix)
  string(TOLOWER ${name} platform)
  string(REPLACE "_" "-" platform "${platform}")

  set(LANGUAGE_SDK_${prefix}_NAME "${name}")
  set(LANGUAGE_SDK_${prefix}_LIB_SUBDIR "${platform}")
  set(LANGUAGE_SDK_${prefix}_ARCHITECTURES "${architectures}")
  if("${prefix}" STREQUAL "CYGWIN")
    set(LANGUAGE_SDK_${prefix}_OBJECT_FORMAT "COFF")
    set(LANGUAGE_SDK_${prefix}_SHARED_LIBRARY_PREFIX "")
    set(LANGUAGE_SDK_${prefix}_SHARED_LIBRARY_SUFFIX ".dll")
    set(LANGUAGE_SDK_${prefix}_STATIC_LIBRARY_PREFIX "")
    set(LANGUAGE_SDK_${prefix}_STATIC_LIBRARY_SUFFIX ".lib")
    set(LANGUAGE_SDK_${prefix}_IMPORT_LIBRARY_PREFIX "")
    set(LANGUAGE_SDK_${prefix}_IMPORT_LIBRARY_SUFFIX ".lib")
  elseif("${prefix}" STREQUAL "WASI" OR "${prefix}" STREQUAL "EMSCRIPTEN")
    set(LANGUAGE_SDK_${prefix}_OBJECT_FORMAT "WASM")
    set(LANGUAGE_SDK_${prefix}_SHARED_LIBRARY_PREFIX "lib")
    set(LANGUAGE_SDK_${prefix}_SHARED_LIBRARY_SUFFIX ".so")
    set(LANGUAGE_SDK_${prefix}_STATIC_LIBRARY_PREFIX "lib")
    set(LANGUAGE_SDK_${prefix}_STATIC_LIBRARY_SUFFIX ".a")
    set(LANGUAGE_SDK_${prefix}_IMPORT_LIBRARY_PREFIX "")
    set(LANGUAGE_SDK_${prefix}_IMPORT_LIBRARY_SUFFIX "")
  else()
    set(LANGUAGE_SDK_${prefix}_OBJECT_FORMAT "ELF")
    set(LANGUAGE_SDK_${prefix}_SHARED_LIBRARY_PREFIX "lib")
    set(LANGUAGE_SDK_${prefix}_SHARED_LIBRARY_SUFFIX ".so")
    set(LANGUAGE_SDK_${prefix}_STATIC_LIBRARY_PREFIX "lib")
    set(LANGUAGE_SDK_${prefix}_STATIC_LIBRARY_SUFFIX ".a")
    set(LANGUAGE_SDK_${prefix}_IMPORT_LIBRARY_PREFIX "")
    set(LANGUAGE_SDK_${prefix}_IMPORT_LIBRARY_SUFFIX "")
  endif()
  set(LANGUAGE_SDK_${prefix}_USE_ISYSROOT FALSE)

  # Static linking is supported on Linux and WASI
  if("${prefix}" STREQUAL "LINUX"
      OR "${prefix}" STREQUAL "LINUX_STATIC"
      OR "${prefix}" STREQUAL "EMSCRIPTEN"
      OR "${prefix}" STREQUAL "WASI")
    set(LANGUAGE_SDK_${prefix}_STATIC_LINKING_SUPPORTED TRUE)
  else()
    set(LANGUAGE_SDK_${prefix}_STATIC_LINKING_SUPPORTED FALSE)
  endif()

  # For LINUX_STATIC, build static only
  if("${prefix}" STREQUAL "LINUX_STATIC")
    set(LANGUAGE_SDK_${prefix}_STATIC_ONLY TRUE)
  else()
    set(LANGUAGE_SDK_${prefix}_STATIC_ONLY FALSE)
  endif()

  if("${prefix}" STREQUAL "LINUX"
      OR "${prefix}" STREQUAL "ANDROID"
      OR "${prefix}" STREQUAL "FREEBSD"
      OR "${prefix}" STREQUAL "OPENBSD")
    set(LANGUAGE_SDK_${prefix}_USE_BUILD_ID TRUE)
  else()
    set(LANGUAGE_SDK_${prefix}_USE_BUILD_ID FALSE)
  endif()

  # GCC on Linux is usually located under `/usr`.
  # However, Ubuntu 20.04 ships with another GCC installation under `/`, which
  # does not include libstdc++. Codira build scripts pass `--sysroot=/` to
  # Clang. By default, Clang tries to find GCC installation under sysroot, and
  # if it doesn't exist, under `{sysroot}/usr`. On Ubuntu 20.04 and newer, it
  # attempts to use the GCC without the C++ stdlib, which causes a build
  # failure. To fix that, we tell Clang explicitly to use GCC from `/usr`.
  # FIXME: This is a compromise. The value might depend on the architecture
  # but add_language_target_library does not allow passing different values
  # depending on the architecture, so having a single value is the only
  # possibility right now.
  set(LANGUAGE_SDK_${prefix}_CXX_OVERLAY_LANGUAGE_COMPILE_FLAGS
      -Xcc --gcc-toolchain=/usr
    CACHE STRING "Extra flags for compiling the C++ overlay")

  set(_default_threading_package "pthreads")
  if("${prefix}" STREQUAL "LINUX" OR "${prefix}" STREQUAL "LINUX_STATIC")
    set(_default_threading_package "linux")
  elseif("${prefix}" STREQUAL "WASI")
    if(LANGUAGE_ENABLE_WASI_THREADS)
      set(_default_threading_package "pthreads")
    else()
      set(_default_threading_package "none")
    endif()
  endif()
  get_threading_package(${prefix} ${_default_threading_package}
    LANGUAGE_SDK_${prefix}_THREADING_PACKAGE)

  foreach(arch ${architectures})
    if("${prefix}" STREQUAL "ANDROID")
      language_android_sysroot(android_sysroot)
      set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_PATH "${android_sysroot}")

      if("${arch}" STREQUAL "armv7")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_NDK_TRIPLE "arm-linux-androideabi")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_ALT_SPELLING "arm")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_TRIPLE "armv7-unknown-linux-androideabi")
        # The Android ABI isn't part of the module triple.
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_MODULE "armv7-unknown-linux-android")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_ABI "armeabi-v7a")
      elseif("${arch}" STREQUAL "aarch64")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_NDK_TRIPLE "aarch64-linux-android")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_ALT_SPELLING "aarch64")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_TRIPLE "aarch64-unknown-linux-android")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_ABI "arm64-v8a")
      elseif("${arch}" STREQUAL "i686")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_NDK_TRIPLE "i686-linux-android")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_ALT_SPELLING "i686")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_TRIPLE "i686-unknown-linux-android")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_ABI "x86")
      elseif("${arch}" STREQUAL "x86_64")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_NDK_TRIPLE "x86_64-linux-android")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_ALT_SPELLING "x86_64")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_TRIPLE "x86_64-unknown-linux-android")
        set(LANGUAGE_SDK_ANDROID_ARCH_${arch}_ABI "x86_64")
      else()
        message(FATAL_ERROR "unknown arch for android SDK: ${arch}")
      endif()
    else()
      set(LANGUAGE_SDK_${prefix}_ARCH_${arch}_PATH "/" CACHE STRING "CMAKE_SYSROOT for ${prefix} ${arch}")

      if("${prefix}" STREQUAL "LINUX")
        if(arch MATCHES "(armv5)")
          set(LANGUAGE_SDK_LINUX_ARCH_${arch}_TRIPLE "${arch}-unknown-linux-gnueabi")
        elseif(arch MATCHES "(armv6|armv7)")
          set(LANGUAGE_SDK_LINUX_ARCH_${arch}_TRIPLE "${arch}-unknown-linux-gnueabihf")
        elseif(arch MATCHES "(aarch64|i686|powerpc|powerpc64|powerpc64le|s390x|x86_64|riscv64)")
          set(LANGUAGE_SDK_LINUX_ARCH_${arch}_TRIPLE "${arch}-unknown-linux-gnu")
        else()
          message(FATAL_ERROR "unknown arch for ${prefix}: ${arch}")
        endif()
      elseif("${prefix}" STREQUAL "FREEBSD")
        if(NOT arch MATCHES "(aarch64|x86_64)")
          message(FATAL_ERROR "unsupported arch for FreeBSD: ${arch}")
        endif()

        if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "FreeBSD")
          message(WARNING "CMAKE_SYSTEM_VERSION will not match target")
        endif()

        string(REGEX REPLACE "[-].*" "" freebsd_system_version ${CMAKE_SYSTEM_VERSION})
        message(STATUS "FreeBSD Version: ${freebsd_system_version}")

        set(LANGUAGE_SDK_FREEBSD_ARCH_${arch}_TRIPLE "${arch}-unknown-freebsd")
      elseif("${prefix}" STREQUAL "OPENBSD")
        if(NOT arch STREQUAL "x86_64" AND NOT arch STREQUAL "aarch64")
          message(FATAL_ERROR "unsupported arch for OpenBSD: ${arch}")
        endif()

        set(openbsd_system_version ${CMAKE_SYSTEM_VERSION})
        message(STATUS "OpenBSD Version: ${openbsd_system_version}")

        set(LANGUAGE_SDK_OPENBSD_ARCH_${arch}_TRIPLE "${arch}-unknown-openbsd${openbsd_system_version}")

        add_link_options("LINKER:-z,origin")

        if(CMAKE_SYSROOT)
          set(LANGUAGE_SDK_OPENBSD_ARCH_${arch}_PATH "${CMAKE_SYSROOT}${LANGUAGE_SDK_OPENBSD_ARCH_${arch}_PATH}" CACHE INTERNAL "sysroot path" FORCE)
        endif()
      elseif("${prefix}" STREQUAL "CYGWIN")
        if(NOT arch STREQUAL "x86_64")
          message(FATAL_ERROR "unsupported arch for cygwin: ${arch}")
        endif()
        set(LANGUAGE_SDK_CYGWIN_ARCH_x86_64_TRIPLE "x86_64-unknown-windows-cygnus")
      elseif("${prefix}" STREQUAL "HAIKU")
        if(NOT arch STREQUAL "x86_64")
          message(FATAL_ERROR "unsupported arch for Haiku: ${arch}")
        endif()
        set(LANGUAGE_SDK_HAIKU_ARCH_x86_64_TRIPLE "x86_64-unknown-haiku")
      elseif("${prefix}" STREQUAL "WASI")
        if(NOT arch STREQUAL "wasm32")
          message(FATAL_ERROR "unsupported arch for WebAssembly: ${arch}")
        endif()
        set(LANGUAGE_SDK_WASI_ARCH_wasm32_PATH "${LANGUAGE_WASI_SYSROOT_PATH}")
        if(LANGUAGE_ENABLE_WASI_THREADS)
          set(LANGUAGE_SDK_WASI_ARCH_wasm32_TRIPLE "wasm32-unknown-wasip1-threads")
        else()
          set(LANGUAGE_SDK_WASI_ARCH_wasm32_TRIPLE "wasm32-unknown-wasi")
        endif()
      elseif("${prefix}" STREQUAL "EMSCRIPTEN")
        set(LANGUAGE_SDK_EMSCRIPTEN_ARCH_${arch}_TRIPLE "${arch}-unknown-emscripten")
      elseif("${prefix}" STREQUAL "LINUX_STATIC")
        set(LANGUAGE_SDK_LINUX_STATIC_ARCH_${arch}_TRIPLE "${arch}-language-linux-musl")
        set(LANGUAGE_SDK_LINUX_STATIC_ARCH_${arch}_PATH "${LANGUAGE_MUSL_PATH}/${arch}")
      else()
        message(FATAL_ERROR "unknown Unix OS: ${prefix}")
      endif()
    endif()

    # If the module triple wasn't set explicitly, it's the same as the triple.
    if(NOT LANGUAGE_SDK_${prefix}_ARCH_${arch}_MODULE)
      set(LANGUAGE_SDK_${prefix}_ARCH_${arch}_MODULE "${LANGUAGE_SDK_${prefix}_ARCH_${arch}_TRIPLE}")
    endif()
  endforeach()

  # Add this to the list of known SDKs.
  list(APPEND LANGUAGE_CONFIGURED_SDKS "${prefix}")

  _report_sdk("${prefix}")
endmacro()

macro(configure_sdk_windows name environment architectures)
  # Note: this has to be implemented as a macro because it sets global
  # variables.

  language_windows_cache_VCVARS()

  string(TOUPPER ${name} prefix)
  string(TOLOWER ${name} platform)

  set(LANGUAGE_SDK_${prefix}_NAME "${name}")
  set(LANGUAGE_SDK_${prefix}_LIB_SUBDIR "windows")
  set(LANGUAGE_SDK_${prefix}_ARCHITECTURES "${architectures}")
  set(LANGUAGE_SDK_${prefix}_OBJECT_FORMAT "COFF")
  set(LANGUAGE_SDK_${prefix}_USE_ISYSROOT FALSE)
  set(LANGUAGE_SDK_${prefix}_SHARED_LIBRARY_PREFIX "")
  set(LANGUAGE_SDK_${prefix}_SHARED_LIBRARY_SUFFIX ".dll")
  set(LANGUAGE_SDK_${prefix}_STATIC_LIBRARY_PREFIX "")
  set(LANGUAGE_SDK_${prefix}_STATIC_LIBRARY_SUFFIX ".lib")
  set(LANGUAGE_SDK_${prefix}_IMPORT_LIBRARY_PREFIX "")
  set(LANGUAGE_SDK_${prefix}_IMPORT_LIBRARY_SUFFIX ".lib")
  set(LANGUAGE_SDK_${prefix}_STATIC_LINKING_SUPPORTED FALSE)
  set(LANGUAGE_SDK_${prefix}_STATIC_ONLY FALSE)
  set(LANGUAGE_SDK_${prefix}_USE_BUILD_ID FALSE)
  get_threading_package(${prefix} "win32" LANGUAGE_SDK_${prefix}_THREADING_PACKAGE)

  foreach(arch ${architectures})
    if(arch STREQUAL "armv7")
      set(LANGUAGE_SDK_${prefix}_ARCH_${arch}_TRIPLE
          "thumbv7-unknown-windows-${environment}")
    else()
      set(LANGUAGE_SDK_${prefix}_ARCH_${arch}_TRIPLE
          "${arch}-unknown-windows-${environment}")
    endif()

    set(LANGUAGE_SDK_${prefix}_ARCH_${arch}_MODULE "${LANGUAGE_SDK_${prefix}_ARCH_${arch}_TRIPLE}")

    # NOTE: set the path to / to avoid a spurious `--sysroot` from being passed
    # to the driver -- rely on the `INCLUDE` AND `LIB` environment variables
    # instead.
    set(LANGUAGE_SDK_${prefix}_ARCH_${arch}_PATH "/")

    # NOTE(compnerd) workaround incorrectly extensioned import libraries from
    # the Windows SDK on case sensitive file systems.
    language_windows_arch_spelling(${arch} WinSDKArchitecture)
    set(WinSDK${arch}UMDir "${UniversalCRTSdkDir}/Lib/${UCRTVersion}/um/${WinSDKArchitecture}")
    set(OverlayDirectory "${CMAKE_BINARY_DIR}/winsdk_lib_${arch}_symlinks")

    if(NOT EXISTS "${UniversalCRTSdkDir}/Include/${UCRTVersion}/um/WINDOWS.H")
      file(MAKE_DIRECTORY ${OverlayDirectory})

      file(GLOB libraries RELATIVE "${WinSDK${arch}UMDir}" "${WinSDK${arch}UMDir}/*")
      foreach(library ${libraries})
        get_filename_component(name_we "${library}" NAME_WE)
        get_filename_component(ext "${library}" EXT)
        string(TOLOWER "${ext}" lowercase_ext)
        set(lowercase_ext_symlink_name "${name_we}${lowercase_ext}")
        if(NOT library STREQUAL lowercase_ext_symlink_name)
          execute_process(COMMAND
                          "${CMAKE_COMMAND}" -E create_symlink "${WinSDK${arch}UMDir}/${library}" "${OverlayDirectory}/${lowercase_ext_symlink_name}")
        endif()
      endforeach()
    endif()
  endforeach()

  # Add this to the list of known SDKs.
  list(APPEND LANGUAGE_CONFIGURED_SDKS "${prefix}")

  _report_sdk("${prefix}")
endmacro()

# Configure a variant of a certain SDK
#
# In addition to the SDK and architecture, a variant determines build settings.
#
# FIXME: this is not wired up with anything yet.
function(configure_target_variant prefix name sdk build_config lib_subdir)
  set(LANGUAGE_VARIANT_${prefix}_NAME                  ${name})
  set(LANGUAGE_VARIANT_${prefix}_SDK_PATH              ${LANGUAGE_SDK_${sdk}_PATH})
  set(LANGUAGE_VARIANT_${prefix}_VERSION               ${LANGUAGE_SDK_${sdk}_VERSION})
  set(LANGUAGE_VARIANT_${prefix}_BUILD_NUMBER          ${LANGUAGE_SDK_${sdk}_BUILD_NUMBER})
  set(LANGUAGE_VARIANT_${prefix}_DEPLOYMENT_VERSION    ${LANGUAGE_SDK_${sdk}_DEPLOYMENT_VERSION})
  set(LANGUAGE_VARIANT_${prefix}_LIB_SUBDIR            "${lib_subdir}/${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}")
  set(LANGUAGE_VARIANT_${prefix}_TRIPLE_NAME           ${LANGUAGE_SDK_${sdk}_TRIPLE_NAME})
  set(LANGUAGE_VARIANT_${prefix}_ARCHITECTURES         ${LANGUAGE_SDK_${sdk}_ARCHITECTURES})
  set(LANGUAGE_VARIANT_${prefix}_SHARED_LIBRARY_PREFIX ${LANGUAGE_SDK_${sdk}_SHARED_LIBRARY_PREFIX})
  set(LANGUAGE_VARIANT_${prefix}_SHARED_LIBRARY_SUFFIX ${LANGUAGE_SDK_${sdk}_SHARED_LIBRARY_SUFFIX})
  set(LANGUAGE_VARIANT_${prefix}_STATIC_LIBRARY_PREFIX ${LANGUAGE_SDK_${sdk}_STATIC_LIBRARY_PREFIX})
  set(LANGUAGE_VARIANT_${prefix}_STATIC_LIBRARY_SUFFIX ${LANGUAGE_SDK_${sdk}_STATIC_LIBRARY_SUFFIX})
  set(LANGUAGE_VARIANT_${prefix}_IMPORT_LIBRARY_PREFIX ${LANGUAGE_SDK_${sdk}_IMPORT_LIBRARY_PREFIX})
  set(LANGUAGE_VARIANT_${prefix}_IMPORT_LIBRARY_SUFFIX ${LANGUAGE_SDK_${sdk}_IMPORT_LIBRARY_SUFFIX})
  set(LANGUAGE_VARIANT_${prefix}_STATIC_LINKING_SUPPORTED ${LANGUAGE_SDK_${sdk}_STATIC_LINKING_SUPPORTED})
  set(LANGUAGE_VARIANT_${prefix}_STATIC_ONLY ${LANGUAGE_SDK_${sdk}_STATIC_ONLY})
  get_threading_package(${prefix} ${LANGUAGE_SDK_${sdk}_THREADING_PACKAGE} LANGUAGE_VARIANT_${prefix}_THREADING_PACKAGE)
endfunction()

