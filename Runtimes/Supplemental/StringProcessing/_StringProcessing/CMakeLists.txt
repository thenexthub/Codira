add_library(language_StringProcessing
  _CUnicode/UnicodeData.c
  _CUnicode/UnicodeScalarProps.c
  _CharacterClassModel.code
  Executor.code
  Capture.code
  Unicode/NumberParsing.code
  Unicode/ASCII.code
  Unicode/CharacterProps.code
  Unicode/Transcoding.code
  Unicode/ScalarProps.code
  Unicode/Comparison.code
  Unicode/NecessaryEvils.code
  Unicode/Validation.code
  Unicode/UCD.code
  Unicode/NFC.code
  Unicode/Encodings.code
  Unicode/Formatting.code
  Unicode/WordBreaking.code
  Unicode/Decoding.code
  Unicode/CaseConversion.code
  Algorithms/Matching/FirstMatch.code
  Algorithms/Matching/MatchingCollectionSearcher.code
  Algorithms/Matching/MatchResult.code
  Algorithms/Matching/MatchReplace.code
  Algorithms/Matching/Matches.code
  Algorithms/Algorithms/FirstRange.code
  Algorithms/Algorithms/Contains.code
  Algorithms/Algorithms/Split.code
  Algorithms/Algorithms/Trim.code
  Algorithms/Algorithms/Ranges.code
  Algorithms/Algorithms/StartsWith.code
  Algorithms/Algorithms/Replace.code
  Algorithms/Algorithms/SubstringSearcher.code
  Algorithms/Searchers/CollectionSearcher.code
  Algorithms/Searchers/ZSearcher.code
  Algorithms/Consumers/CollectionConsumer.code
  Algorithms/Consumers/FixedPatternConsumer.code
  Regex/Options.code
  Regex/Match.code
  Regex/CustomComponents.code
  Regex/Core.code
  Regex/ASTConversion.code
  Regex/DSLTree.code
  Regex/AnyRegexOutput.code
  LiteralPrinter.code
  MatchingOptions.code
  Compiler.code
  ConsumerInterface.code
  PrintAsPattern.code
  Engine/MEBuiltins.code
  Engine/Processor.code
  Engine/MEProgram.code
  Engine/MEQuantify.code
  Engine/InstPayload.code
  Engine/Backtracking.code
  Engine/Instruction.code
  Engine/Registers.code
  Engine/MEBuilder.code
  Engine/Structuralize.code
  Engine/Tracing.code
  Engine/Metrics.code
  Engine/MECapture.code
  ByteCodeGen.code
  Utility/AsciiBitset.code
  Utility/Traced.code
  Utility/TypedIndex.code
  Utility/Misc.code
  Utility/TypedInt.code
  Utility/Protocols.code
  Utility/TypeVerification.code
  Utility/RegexFactory.code
  Utility/ASTBuilder.code)

set_target_properties(language_StringProcessing PROPERTIES
  Codira_MODULE_NAME _StringProcessing)

target_link_libraries(language_StringProcessing PRIVATE
    language_RegexParser
    languageCore)

install(TARGETS language_StringProcessing
  ARCHIVE DESTINATION "${CodiraStringProcessing_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CodiraStringProcessing_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
emit_language_interface(language_StringProcessing)
install_language_interface(language_StringProcessing)

embed_manifest(language_StringProcessing)
