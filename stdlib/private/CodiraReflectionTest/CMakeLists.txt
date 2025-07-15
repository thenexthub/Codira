set(language_reflection_test_darwin_dependencies)

if((LANGUAGE_BUILD_CLANG_OVERLAYS
    OR LANGUAGE_BUILD_TEST_SUPPORT_MODULES)
   AND (NOT DEFINED LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT
        OR NOT LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT))
  # This library imports Darwin from the SDK, which re-exports
  # _Builtin_float which is part of this build.
  list(APPEND language_reflection_test_darwin_dependencies _Builtin_float)
endif()

if (LANGUAGE_INCLUDE_TESTS AND LANGUAGE_BUILD_DYNAMIC_STDLIB)
  add_language_target_library(languageCodiraReflectionTest ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
    CodiraReflectionTest.code
    LANGUAGE_COMPILE_FLAGS ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
    LANGUAGE_COMPILE_FLAGS_LINUX -Xcc -D_GNU_SOURCE
    LANGUAGE_MODULE_DEPENDS_OSX ${language_reflection_test_darwin_dependencies}
    LANGUAGE_MODULE_DEPENDS_IOS ${language_reflection_test_darwin_dependencies}
    LANGUAGE_MODULE_DEPENDS_TVOS ${language_reflection_test_darwin_dependencies}
    LANGUAGE_MODULE_DEPENDS_WATCHOS ${language_reflection_test_darwin_dependencies}
    LANGUAGE_MODULE_DEPENDS_XROS ${language_reflection_test_darwin_dependencies}
    LANGUAGE_MODULE_DEPENDS_ANDROID Android
    LANGUAGE_MODULE_DEPENDS_LINUX Glibc
    LANGUAGE_MODULE_DEPENDS_LINUX_STATIC Musl
    LANGUAGE_MODULE_DEPENDS_FREEBSD Glibc
    LANGUAGE_MODULE_DEPENDS_OPENBSD Glibc
    LANGUAGE_MODULE_DEPENDS_WINDOWS CRT
    INSTALL_IN_COMPONENT stdlib-experimental
    DARWIN_INSTALL_NAME_DIR "${LANGUAGE_DARWIN_STDLIB_PRIVATE_INSTALL_NAME_DIR}"

    TARGET_SDKS NOT LINUX_STATIC)

  foreach(SDK ${LANGUAGE_SDKS})
    if ("${SDK}" STREQUAL "LINUX_STATIC")
      continue()
    endif()

    foreach(ARCH ${LANGUAGE_SDK_${SDK}_ARCHITECTURES})
      set(VARIANT_SUFFIX "-${LANGUAGE_SDK_${SDK}_LIB_SUBDIR}-${ARCH}")
      add_dependencies("languageCodiraReflectionTest${VARIANT_SUFFIX}"
        "language-reflection-test${VARIANT_SUFFIX}")
    endforeach()
  endforeach()
endif()
