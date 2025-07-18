list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../../cmake/modules)
include(StdlibOptions)

set(LANGUAGE_CXX_DEPS symlink_clang_headers)
if(LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT)
  list(APPEND LANGUAGE_CXX_DEPS copy-legacy-layouts)
endif()

add_language_target_library(languageCxx STATIC NO_LINK_NAME IS_STDLIB IS_LANGUAGE_ONLY
    CxxConvertibleToBool.code
    CxxConvertibleToCollection.code
    CxxDictionary.code
    CxxPair.code
    CxxOptional.code
    CxxSet.code
    CxxRandomAccessCollection.code
    CxxSequence.code
    CxxVector.code
    CxxSpan.code
    UnsafeCxxIterators.code

    LANGUAGE_COMPILE_FLAGS ${LANGUAGE_RUNTIME_LANGUAGE_COMPILE_FLAGS} ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
    -cxx-interoperability-mode=default
    -enable-experimental-feature Span
    -enable-experimental-feature BuiltinModule
    -enable-experimental-feature AllowUnsafeAttribute
    -strict-memory-safety
    # This module should not pull in the C++ standard library, so we disable it explicitly.
    # For functionality that depends on the C++ stdlib, use C++ stdlib overlay (`languagestd` module).
    -Xcc -nostdinc++

    DEPLOYMENT_VERSION_OSX ${COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_OSX}
    DEPLOYMENT_VERSION_IOS ${COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_IOS}
    DEPLOYMENT_VERSION_TVOS ${COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_TVOS}
    DEPLOYMENT_VERSION_WATCHOS ${COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_WATCHOS}
    DEPLOYMENT_VERSION_XROS ${COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_XROS}

    LINK_FLAGS "${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}"
    DEPENDS ${LANGUAGE_CXX_DEPS}
    MACCATALYST_BUILD_FLAVOR zippered
    INSTALL_IN_COMPONENT compiler
    INSTALL_BINARY_LANGUAGEMODULE NON_DARWIN_ONLY
    INSTALL_WITH_SHARED)

add_subdirectory(libstdcxx)
add_subdirectory(std)
add_subdirectory(cxxshim)
