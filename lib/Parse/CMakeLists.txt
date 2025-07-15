

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  set(LANGUAGE_GYB_FLAGS --line-directive "^\"#line %(line)d \\\"%(file)s\\\"^\"")
else()
  set(LANGUAGE_GYB_FLAGS --line-directive "\'#line" "%(line)d" "\"%(file)s\"\'")
endif()

add_language_host_library(languageParse STATIC
  Confusables.cpp
  Lexer.cpp
  ParseDecl.cpp
  ParseDeclName.cpp
  ParseExpr.cpp
  ParseGeneric.cpp
  ParseIfConfig.cpp
  ParsePattern.cpp
  Parser.cpp
  ParseRegex.cpp
  ParseRequests.cpp
  ParseStmt.cpp
  ParseType.cpp
  ParseVersion.cpp
  PersistentParserState.cpp)
target_link_libraries(languageParse PRIVATE
  languageAST
)

if (LANGUAGE_BUILD_LANGUAGE_SYNTAX)
  target_link_libraries(languageParse
    PRIVATE
    languageASTGen
  )

  add_dependencies(languageParse
    languageASTGen
  )
endif()

if(LANGUAGE_BUILD_REGEX_PARSER_IN_COMPILER)
  target_compile_definitions(languageParse
    PUBLIC
    LANGUAGE_BUILD_REGEX_PARSER_IN_COMPILER
  )
endif()

if(LANGUAGE_COMPILER_VERSION)
  set_property(SOURCE ParseVersion.cpp APPEND_STRING PROPERTY COMPILE_FLAGS
    " -DLANGUAGE_COMPILER_VERSION=\"\\\"${LANGUAGE_COMPILER_VERSION}\\\"\"")
endif()

set_language_toolchain_is_available(languageParse)
