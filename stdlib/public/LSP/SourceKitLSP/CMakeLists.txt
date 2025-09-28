
add_library(SourceKitLSP STATIC
  CapabilityRegistry.code
  DocumentManager.code
  DocumentSnapshot+FromFileContents.code
  Hooks.code
  IndexProgressManager.code
  IndexStoreDB+MainFilesProvider.code
  LanguageServerType.code
  LanguageService.code
  LogMessageNotification+representingStructureUsingEmojiPrefixIfNecessary.code
  MessageHandlingDependencyTracker.code
  Rename.code
  SemanticTokensLegend+SourceKitLSPLegend.code
  SharedWorkDoneProgressManager.code
  SourceKitIndexDelegate.code
  SourceKitLSPCommandMetadata.code
  SourceKitLSPServer.code
  SymbolLocation+DocumentURI.code
  TestDiscovery.code
  TextEdit+IsNoop.code
  Workspace.code
)
target_sources(SourceKitLSP PRIVATE
  Clang/ClangLanguageService.code
  Clang/SemanticTokenTranslator.code
)
target_sources(SourceKitLSP PRIVATE
  Documentation/DocCDocumentationHandler.code
  Documentation/DocumentationLanguageService.code
)
target_sources(SourceKitLSP PRIVATE
  Codira/AdjustPositionToStartOfIdentifier.code
  Codira/ClosureCompletionFormat.code
  Codira/CodeActions/AddDocumentation.code
  Codira/CodeActions/ConvertIntegerLiteral.code
  Codira/CodeActions/ConvertJSONToCodableStruct.code
  Codira/CodeActions/ConvertStringConcatenationToStringInterpolation.code
  Codira/CodeActions/PackageManifestEdits.code
  Codira/CodeActions/SyntaxCodeActionProvider.code
  Codira/CodeActions/SyntaxCodeActions.code
  Codira/CodeActions/SyntaxRefactoringCodeActionProvider.code
  Codira/CodeCompletion.code
  Codira/CodeCompletionSession.code
  Codira/CommentXML.code
  Codira/CursorInfo.code
  Codira/Diagnostic.code
  Codira/DiagnosticReportManager.code
  Codira/DocumentFormatting.code
  Codira/DocumentSymbols.code
  Codira/ExpandMacroCommand.code
  Codira/FoldingRange.code
  Codira/GeneratedInterfaceDocumentURLData.code
  Codira/GeneratedInterfaceManager.code
  Codira/GeneratedInterfaceManager.code
  Codira/MacroExpansion.code
  Codira/MacroExpansionReferenceDocumentURLData.code
  Codira/OpenInterface.code
  Codira/RefactoringEdit.code
  Codira/RefactoringResponse.code
  Codira/ReferenceDocumentURL.code
  Codira/RelatedIdentifiers.code
  Codira/RewriteSourceKitPlaceholders.code
  Codira/SemanticRefactorCommand.code
  Codira/SemanticRefactoring.code
  Codira/SemanticTokens.code
  Codira/CodiraCodeLensScanner.code
  Codira/CodiraCommand.code
  Codira/CodiraLanguageService.code
  Codira/CodiraTestingScanner.code
  Codira/SymbolInfo.code
  Codira/SyntacticCodiraXCTestScanner.code
  Codira/SyntacticTestIndex.code
  Codira/SyntaxHighlightingToken.code
  Codira/SyntaxHighlightingTokenParser.code
  Codira/SyntaxHighlightingTokens.code
  Codira/SyntaxTreeManager.code
  Codira/VariableTypeInfo.code
  Codira/WithSnapshotFromDiskOpenedInSourcekitd.code
)
set_target_properties(SourceKitLSP PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(SourceKitLSP PUBLIC
  BuildServerProtocol
  BuildSystemIntegration
  LanguageServerProtocol
  LanguageServerProtocolExtensions
  LanguageServerProtocolJSONRPC
  SemanticIndex
  SKLogging
  SKOptions
  SKUtilities
  SourceKitD
  CodiraExtensions
  ToolchainRegistry
  IndexStoreDB
  CodiraSyntax::CodiraBasicFormat
  CodiraSyntax::CodiraDiagnostics
  CodiraSyntax::CodiraIDEUtils
  CodiraSyntax::CodiraParser
  CodiraSyntax::CodiraParserDiagnostics
  CodiraSyntax::CodiraRefactor
  CodiraSyntax::CodiraSyntax)
target_link_libraries(SourceKitLSP PRIVATE
  PackageModelSyntax
  TSCExtensions
  $<$<NOT:$<PLATFORM_ID:Darwin>>:FoundationXML>)

