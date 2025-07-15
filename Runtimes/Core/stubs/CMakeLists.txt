add_library(languageStdlibStubs OBJECT
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
  Unicode/UnicodeWord.cpp)

if(CodiraCore_ENABLE_OBJC_INTEROP)
  gyb_expand(CodiraNativeNSXXXBase.mm.gyb "CodiraNativeNSXXXBase.mm")
  target_sources(languageStdlibStubs PRIVATE
    # ObjC files
    Availability.mm
    FoundationHelpers.mm
    OptionalBridgingHelper.mm
    Reflection.mm
    CodiraNativeNSObject.mm
    CodiraNativeNSXXXBaseARC.m
    "${CMAKE_CURRENT_BINARY_DIR}/CodiraNativeNSXXXBase.mm")
endif()


target_compile_definitions(languageStdlibStubs PRIVATE
  $<$<BOOL:${BUILD_SHARED_LIBS}>:-DlanguageCore_EXPORTS>
  $<$<BOOL:${CodiraCore_ENABLE_UNICODE_DATA}>:-DLANGUAGE_STDLIB_ENABLE_UNICODE_DATA>
  $<$<BOOL:${CodiraCore_HAS_LOCALE}>:-DLANGUAGE_STDLIB_HAS_LOCALE>)

target_link_libraries(languageStdlibStubs PRIVATE
  languageShims)

target_include_directories(languageStdlibStubs PRIVATE
  "${PROJECT_BINARY_DIR}/include"
  # FIXME: pulls in headers from the main compiler build
  # Assert.cpp:
  #   language/Runtime/Config.h
  #   language/Runtime/Debug.h
  #   language/Runtime/Portability.h
  # Stubs.cpp:
  #   language/Runtime/Debug.h
  # GlobalObjects.cpp:
  #   language/Basic/type_traits.h
  # Random.cpp:
  #   language/Runtime/Config.h
  # UnicodeScalarProps.cpp:
  #   language/Runtime/Debug.h
  # ...
  #
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/include")

if(APPLE)
  set_property(SOURCE CodiraNativeNSXXXBaseARC.m
               APPEND_STRING
               PROPERTY COMPILE_FLAGS "-fobjc-arc")
endif()

if(NOT BUILD_SHARED_LIBS)
  install(TARGETS languageStdlibStubs
    EXPORT CodiraCoreTargets
    COMPONENT CodiraCore_runtime)
endif()
