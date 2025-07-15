if(NOT LANGUAGE_HOST_VARIANT MATCHES "${LANGUAGE_DARWIN_EMBEDDED_VARIANTS}")
  add_language_unittest(SourceKitCodiraLangTests
    CursorInfoTest.cpp
    CloseTest.cpp
    EditingTest.cpp
    )
  target_link_libraries(SourceKitCodiraLangTests PRIVATE SourceKitCodiraLang)
  target_compile_definitions(SourceKitCodiraLangTests PRIVATE
    LANGUAGELIB_DIR=\"${LANGUAGELIB_DIR}\")
endif()
