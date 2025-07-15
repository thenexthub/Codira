add_language_unittest(languageLocalizationTests
  DefToStringsConverterTests.cpp
  SerializationTests.cpp)

target_link_libraries(languageLocalizationTests
  PRIVATE
    languageLocalization)

target_compile_definitions(languageLocalizationTests PRIVATE
  LANGUAGELIB_DIR=\"${LANGUAGELIB_DIR}\")
