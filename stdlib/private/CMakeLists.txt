if(LANGUAGE_BUILD_SDK_OVERLAY
   OR (LANGUAGE_BUILD_TEST_SUPPORT_MODULES
       AND NOT LANGUAGE_BUILD_DYNAMIC_SDK_OVERLAY_default
       AND (LANGUAGE_ENABLE_REFLECTION
            OR NOT LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS)))
  # CodiraPrivateThreadExtras makes use of Darwin/Glibc, which is part of the
  # SDK overlay. If the SDK overlay doesn't build by default, then it should
  # be available in the SDK and DifferentiationUnittest can still be built.
  # However, the overlay in the Apple SDKs requires the standard library to
  # have reflection enabled.
  if(LANGUAGE_ENABLE_EXPERIMENTAL_DIFFERENTIABLE_PROGRAMMING)
    add_subdirectory(DifferentiationUnittest)
  endif()
  add_subdirectory(CodiraPrivate)
  add_subdirectory(RuntimeUnittest)
  add_subdirectory(StdlibUnicodeUnittest)
  add_subdirectory(StdlibCollectionUnittest)
  add_subdirectory(CodiraPrivateLibcExtras)
  add_subdirectory(CodiraPrivateThreadExtras)

  # NOTE(compnerd) this must come after CodiraPrivateLibcExtras and
  # CodiraPrivateThreadExtras to ensure that the dependency targets are setup in
  # the correct order for Windows.
  add_subdirectory(StdlibUnittest)

  add_subdirectory(OSLog)

  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_subdirectory(StdlibUnittestFoundationExtras)
  endif()
  # Currently CodiraReflectionTest cannot be built on Windows, due to
  # dependencies on POSIX symbols
  if (LANGUAGE_INCLUDE_TESTS AND (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows"))
    add_subdirectory(CodiraReflectionTest)
  endif()
endif()

# Keep in sync with stdlib/tools/CMakeLists.txt: language-reflection-test is
# only used when testing dynamic stdlib.
if(LANGUAGE_BUILD_DYNAMIC_STDLIB AND LANGUAGE_INCLUDE_TESTS)
  # NOTE create a stub BlocksRuntime library that can be used for the
  # reflection tests
  add_subdirectory(BlocksRuntimeStubs)
endif()

