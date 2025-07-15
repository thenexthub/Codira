# If we're building overlays, make sure we mention the ones we need.
# Otherwise, assume they already exist.
set(darwin)
set(wincrt_sdk)
set(glibc)
set(musl)

if(LANGUAGE_BUILD_SDK_OVERLAY)
  set(wincrt_sdk CRT WinSDK)
  set(glibc Glibc)
  set(musl Musl)
endif()

if((LANGUAGE_BUILD_CLANG_OVERLAYS
    OR LANGUAGE_BUILD_TEST_SUPPORT_MODULES)
   AND (NOT DEFINED LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT
        OR NOT LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT))
  # This library imports Darwin from the SDK, which re-exports
  # _Builtin_float which is part of this build.
  list(APPEND darwin _Builtin_float)
endif()

# Similarly, we only want the Runtime dependency if we're building
# with the stdlib.
set(runtime)
if(LANGUAGE_BUILD_STDLIB AND LANGUAGE_ENABLE_RUNTIME_MODULE)
  set(runtime Runtime)
endif()

if(NOT LANGUAGE_BUILD_STDLIB)
  set(BUILD_STANDALONE TRUE)
endif()

set(BACKTRACING_COMPILE_FLAGS
  "-cxx-interoperability-mode=default"
  "-I${LANGUAGE_STDLIB_SOURCE_DIR}/public/RuntimeModule/modules"
  "-Xcc;-I${LANGUAGE_SOURCE_DIR}/include"
  "-Xcc;-I${CMAKE_BINARY_DIR}/include"
  "-disable-upcoming-feature;MemberImportVisibility")

set(BACKTRACING_SOURCES
  main.code
  AnsiColor.code
  JSON.code
  TargetMacOS.code
  TargetLinux.code
  Themes.code
  Utils.code
  )

# We have to build with a deployment target of at least 10.15, otherwise
# the tests will all fail because dyld will get confused at the use of
# @rpath (from magic-symbols-for-install-name.c) and the `some Sequence<Frame>`
# in Backtrace won't be accessible.
if(LANGUAGE_DARWIN_DEPLOYMENT_VERSION_OSX VERSION_LESS "10.15")
  set(osx_deployment_target "10.15")
else()
  set(osx_deployment_target "${LANGUAGE_DARWIN_DEPLOYMENT_VERSION_OSX}")
endif()

add_language_target_executable(language-backtrace BUILD_WITH_LIBEXEC
  ${BACKTRACING_SOURCES}

  LANGUAGE_MODULE_DEPENDS         ${runtime}

  LANGUAGE_MODULE_DEPENDS_OSX          ${darwin}
  LANGUAGE_MODULE_DEPENDS_WINDOWS      ${wincrt_sdk}
  LANGUAGE_MODULE_DEPENDS_LINUX        ${glibc}
  LANGUAGE_MODULE_DEPENDS_LINUX_STATIC ${musl}

  DEPLOYMENT_VERSION_OSX ${osx_deployment_target}

  INSTALL_IN_COMPONENT libexec
  COMPILE_FLAGS
    ${BACKTRACING_COMPILE_FLAGS}
    -parse-as-library

  TARGET_SDKS OSX LINUX)

set(static_target_sdks)
if(LANGUAGE_BUILD_STATIC_STDLIB)
  list(APPEND static_target_sdks "LINUX")
endif()
if("LINUX_STATIC" IN_LIST LANGUAGE_SDKS)
  list(APPEND static_target_sdks "LINUX_STATIC")
endif()
if(static_target_sdks)
  add_language_target_executable(language-backtrace-static BUILD_WITH_LIBEXEC
    PREFER_STATIC

    ${BACKTRACING_SOURCES}

    LANGUAGE_MODULE_DEPENDS         ${runtime}

    LANGUAGE_MODULE_DEPENDS_OSX          ${darwin}
    LANGUAGE_MODULE_DEPENDS_WINDOWS      ${wincrt_sdk}
    LANGUAGE_MODULE_DEPENDS_LINUX        ${glibc}
    LANGUAGE_MODULE_DEPENDS_LINUX_STATIC ${musl}

    DEPLOYMENT_VERSION_OSX ${osx_deployment_target}

    INSTALL_IN_COMPONENT libexec
    COMPILE_FLAGS
      ${BACKTRACING_COMPILE_FLAGS}
      -parse-as-library

    TARGET_SDKS ${static_target_sdks})
endif()
