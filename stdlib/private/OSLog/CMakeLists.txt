set(language_oslog_darwin_dependencies "")
if (LANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY)
    list(APPEND language_oslog_darwin_dependencies "_Concurrency")
endif()
if (LANGUAGE_ENABLE_EXPERIMENTAL_STRING_PROCESSING)
  list(APPEND language_oslog_darwin_dependencies "_StringProcessing")
endif()
if((LANGUAGE_BUILD_CLANG_OVERLAYS
    OR LANGUAGE_BUILD_TEST_SUPPORT_MODULES)
   AND (NOT DEFINED LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT
        OR NOT LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT))
  # This library imports Darwin from the SDK, which re-exports
  # _Builtin_float which is part of this build.
  list(APPEND language_oslog_darwin_dependencies _Builtin_float)
endif()

add_language_target_library(languageOSLogTestHelper
  IS_SDK_OVERLAY
  SHARED

  OSLogTestHelper.code
  OSLogMessage.code
  OSLogIntegerFormatting.code
  OSLogStringAlignment.code
  OSLogIntegerTypes.code
  OSLogStringTypes.code
  OSLogNSObjectType.code
  OSLogFloatingPointTypes.code
  OSLogCodiraProtocols.code
  OSLogPrivacy.code
  OSLogFloatFormatting.code

  LANGUAGE_MODULE_DEPENDS_IOS ${language_oslog_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_OSX ${language_oslog_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_TVOS ${language_oslog_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_WATCHOS ${language_oslog_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_XROS ${language_oslog_darwin_dependencies}
  LANGUAGE_MODULE_DEPENDS_MACCATALYST ${language_oslog_darwin_dependencies}
  TARGET_SDKS ALL_APPLE_PLATFORMS
  LANGUAGE_COMPILE_FLAGS ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
  INSTALL_IN_COMPONENT never_install
  DARWIN_INSTALL_NAME_DIR "${LANGUAGE_DARWIN_STDLIB_PRIVATE_INSTALL_NAME_DIR}"
  MACCATALYST_BUILD_FLAVOR "zippered")
