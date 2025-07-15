add_library(language_RegexParser
  Regex/TreeProtocols.code
  Regex/Parse/DelimiterLexing.code
  Regex/Parse/Source.code
  Regex/Parse/CharacterPropertyClassification.code
  Regex/Parse/LexicalAnalysis.code
  Regex/Parse/SyntaxOptions.code
  Regex/Parse/CaptureList.code
  Regex/Parse/CompilerInterface.code
  Regex/Parse/CaptureStructure.code
  Regex/Parse/Parse.code
  Regex/Parse/SourceLocation.code
  Regex/Parse/Diagnostics.code
  Regex/Parse/Sema.code
  Regex/Printing/RenderRanges.code
  Regex/Printing/PrettyPrinter.code
  Regex/Printing/PrintAsCanonical.code
  Regex/Printing/DumpAST.code
  Regex/AST/ASTAction.code
  Regex/AST/AST.code
  Regex/AST/CustomCharClass.code
  Regex/AST/Quantification.code
  Regex/AST/Conditional.code
  Regex/AST/MatchingOptions.code
  Regex/AST/ASTProtocols.code
  Regex/AST/Group.code
  Regex/AST/Atom.code
  Utility/TypeConstruction.code
  Utility/Misc.code
  Utility/AllScalars.code
  Utility/Errors.code
  Utility/MissingUnicode.code)

target_link_libraries(language_RegexParser PRIVATE
  languageCore)

set_target_properties(language_RegexParser PROPERTIES
  Codira_MODULE_NAME _RegexParser)

install(TARGETS language_RegexParser
  ARCHIVE DESTINATION "${CodiraStringProcessing_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CodiraStringProcessing_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
emit_language_interface(language_RegexParser)
install_language_interface(language_RegexParser)

embed_manifest(language_RegexParser)
