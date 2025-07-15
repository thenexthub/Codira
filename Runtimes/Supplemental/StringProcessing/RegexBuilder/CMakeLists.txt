add_library(languageRegexBuilder
  CharacterClass.code
  Anchor.code
  Builder.code
  Algorithms.code
  Variadics.code
  DSL.code)

set_target_properties(languageRegexBuilder PROPERTIES
  Codira_MODULE_NAME RegexBuilder)

target_link_libraries(languageRegexBuilder PRIVATE
  language_RegexParser
  language_StringProcessing
  languageCore)

install(TARGETS languageRegexBuilder
  ARCHIVE DESTINATION "${CodiraStringProcessing_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CodiraStringProcessing_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
emit_language_interface(languageRegexBuilder)
install_language_interface(languageRegexBuilder)

embed_manifest(languageRegexBuilder)
