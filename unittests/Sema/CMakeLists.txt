
add_language_unittest(languageSemaTests
  SemaFixture.cpp
  BindingInferenceTests.cpp
  ConstraintGenerationTests.cpp
  ConstraintSimplificationTests.cpp
  ConstraintSystemDumpTests.cpp
  UnresolvedMemberLookupTests.cpp
  PlaceholderTypeInferenceTests.cpp
  SolutionFilteringTests.cpp
  KeypathFunctionConversionTests.cpp)

target_link_libraries(languageSemaTests
  PRIVATE
    languageAST
    languageSema
    languageSerialization)

target_compile_definitions(languageSemaTests PRIVATE
  LANGUAGELIB_DIR=\"${LANGUAGELIB_DIR}\")
