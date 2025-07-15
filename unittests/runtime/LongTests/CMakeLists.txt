if(("${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "${LANGUAGE_PRIMARY_VARIANT_SDK}") AND
   ("${LANGUAGE_HOST_VARIANT_ARCH}" STREQUAL "${LANGUAGE_PRIMARY_VARIANT_ARCH}"))

  set(PLATFORM_SOURCES)
  set(PLATFORM_TARGET_LINK_LIBRARIES)

  if(LANGUAGE_HOST_VARIANT MATCHES "${LANGUAGE_DARWIN_VARIANTS}")
#   Placeholder for Darwin Foundation tests (currently none)
#    find_library(FOUNDATION_LIBRARY Foundation)
#    list(APPEND PLATFORM_SOURCES
#      YourTestNameHere.mm
#      )
#    # We need to link languageCore on Darwin because the runtime still relies on
#    # some stdlib hooks to implement CodiraObject.
#    list(APPEND PLATFORM_TARGET_LINK_LIBRARIES
#      ${FOUNDATION_LIBRARY}
#      languageStdlibUnittest${LANGUAGE_PRIMARY_VARIANT_SUFFIX}
#      )

    # Link the Objective-C runtime.
    list(APPEND PLATFORM_TARGET_LINK_LIBRARIES "objc")
  elseif(LANGUAGE_HOST_VARIANT STREQUAL "linux")
    if(LANGUAGE_HOST_VARIANT_ARCH MATCHES "armv6|armv7|i686")
      list(APPEND PLATFORM_TARGET_LINK_LIBRARIES
        "atomic"
        )
    endif()
  elseif(LANGUAGE_HOST_VARIANT STREQUAL "freebsd")
    find_library(EXECINFO_LIBRARY execinfo)
    list(APPEND PLATFORM_TARGET_LINK_LIBRARIES
      ${EXECINFO_LIBRARY}
      )
  elseif(LANGUAGE_HOST_VARIANT STREQUAL "windows")
    list(APPEND PLATFORM_TARGET_LINK_LIBRARIES DbgHelp;Synchronization)
  endif()

  add_language_unittest(CodiraRuntimeLongTests IS_TARGET_TEST
    LongRefcounting.cpp
    ../Stdlib.cpp
    ${PLATFORM_SOURCES}

    # The runtime tests link to internal runtime symbols, which aren't exported
    # from the languageCore dylib, so we need to link to both the runtime archive
    # and the stdlib.
    $<TARGET_OBJECTS:languageRuntimeCore${LANGUAGE_PRIMARY_VARIANT_SUFFIX}>
    $<TARGET_OBJECTS:languageLLVMSupport${LANGUAGE_PRIMARY_VARIANT_SUFFIX}>
    $<TARGET_OBJECTS:languageDemangling${LANGUAGE_PRIMARY_VARIANT_SUFFIX}>
    )

  # The local stdlib implementation provides definitions of the languageCore
  # interfaces to avoid pulling in languageCore itself.  Build the
  # CodiraRuntimeLongTests with languageCore_EXPORTS to permit exporting the stdlib
  # interfaces.
  target_compile_definitions(CodiraRuntimeLongTests
                             PRIVATE
                               languageCore_EXPORTS
                               LANGUAGE_INLINE_NAMESPACE=__runtime)

  # FIXME: cross-compile for all variants.
  target_link_libraries(CodiraRuntimeLongTests
    PRIVATE
    languageCore${LANGUAGE_PRIMARY_VARIANT_SUFFIX}
    languageThreading${LANGUAGE_PRIMARY_VARIANT_SUFFIX}
    ${PLATFORM_TARGET_LINK_LIBRARIES}
    ${language_runtime_test_extra_libraries}
    )
endif()

