add_language_unittest(CodiraFrontendTests
  ArgParsingTest.cpp
  FeatureParsingTest.cpp
  IsFeatureEnabledTests.cpp
  StrictConcurrencyTests.cpp)

target_link_libraries(CodiraFrontendTests
  PRIVATE
    languageFrontend
    # FIXME: Circular dependencies (AST <-> Parse)
    languageAST)
