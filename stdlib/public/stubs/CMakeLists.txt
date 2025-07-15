set(language_stubs_sources
    Assert.cpp
    GlobalObjects.cpp
    LibcShims.cpp
    Random.cpp
    Stubs.cpp
    ThreadLocalStorage.cpp
    MathStubs.cpp
    Unicode/UnicodeData.cpp
    Unicode/UnicodeGrapheme.cpp
    Unicode/UnicodeNormalization.cpp
    Unicode/UnicodeScalarProps.cpp
    Unicode/UnicodeWord.cpp
)
set(language_stubs_objc_sources
    Availability.mm
    FoundationHelpers.mm
    OptionalBridgingHelper.mm
    Reflection.mm
    CodiraNativeNSObject.mm
    CodiraNativeNSXXXBaseARC.m)
set(language_stubs_gyb_sources
    CodiraNativeNSXXXBase.mm.gyb)
set(TOOLCHAIN_OPTIONAL_SOURCES
    ${language_stubs_objc_sources}
    ${language_stubs_gyb_sources})

set(language_stubs_c_compile_flags ${LANGUAGE_RUNTIME_CORE_CXX_FLAGS})
list(APPEND language_stubs_c_compile_flags -DlanguageCore_EXPORTS)
list(APPEND language_stubs_c_compile_flags -I${LANGUAGE_SOURCE_DIR}/include -I${LANGUAGE_SOURCE_DIR}/stdlib/public)

add_language_target_library(languageStdlibStubs
                  OBJECT_LIBRARY
                    ${language_stubs_sources}
                    ${language_stubs_objc_sources}
                  GYB_SOURCES
                    ${language_stubs_gyb_sources}
                  C_COMPILE_FLAGS
                    ${language_stubs_c_compile_flags}
                  LINK_FLAGS
                    ${LANGUAGE_RUNTIME_CORE_LINK_FLAGS}
                  LANGUAGE_COMPILE_FLAGS ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
                  INSTALL_IN_COMPONENT
                    stdlib)

if("${LANGUAGE_PRIMARY_VARIANT_SDK}" IN_LIST LANGUAGE_DARWIN_PLATFORMS)
  set_property(SOURCE
                 CodiraNativeNSXXXBaseARC.m
               APPEND_STRING PROPERTY COMPILE_FLAGS
                 "-fobjc-arc")
endif()

add_subdirectory(Unicode)
