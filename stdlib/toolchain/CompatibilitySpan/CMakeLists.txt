if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND DEFINED LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES)

  set(library_name "languageCompatibilitySpan")

  add_language_target_library("${library_name}" ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
    FakeStdlib.code
    ../../public/core/Span/MutableRawSpan.code
    ../../public/core/Span/MutableSpan.code
    ../../public/core/Span/RawSpan.code
    ../../public/core/Span/Span.code

    TARGET_SDKS ${LANGUAGE_APPLE_PLATFORMS}

    LINK_FLAGS "${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}"

    LANGUAGE_COMPILE_FLAGS
      ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
      -parse-stdlib
      -module-abi-name Codira
      -DSPAN_COMPATIBILITY_STUB

    NO_LANGUAGEMODULE

    DEPLOYMENT_VERSION_OSX "10.14.4"
    DEPLOYMENT_VERSION_IOS "12.2"
    DEPLOYMENT_VERSION_TVOS "12.2"
    DEPLOYMENT_VERSION_WATCHOS "5.2"
    DEPLOYMENT_VERSION_XROS "1.0"
    DEPLOYMENT_VERSION_MACCATALYST "13.1"

    MACCATALYST_BUILD_FLAVOR "zippered"

    INSTALL_IN_COMPONENT compiler

    LINK_FLAGS "${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}" -Xlinker -not_for_dyld_shared_cache

    BACK_DEPLOYMENT_LIBRARY 6.2)
endif()
