add_language_unittest(CodiraParseTests
  BuildConfigTests.cpp
  LexerTests.cpp
  TokenizerTests.cpp
)

target_link_libraries(CodiraParseTests
    PRIVATE
    languageSIL
    languageClangImporter
    languageParse
    languageAST
    # FIXME: Sema must go last because of circular dependencies with AST.
    languageSema
)
