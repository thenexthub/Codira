
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
  Swift/AdjustPositionToStartOfIdentifier.code
  Swift/ClosureCompletionFormat.code
  Swift/CodeActions/AddDocumentation.code
  Swift/CodeActions/ConvertIntegerLiteral.code
  Swift/CodeActions/ConvertJSONToCodableStruct.code
  Swift/CodeActions/ConvertStringConcatenationToStringInterpolation.code
  Swift/CodeActions/PackageManifestEdits.code
  Swift/CodeActions/SyntaxCodeActionProvider.code
  Swift/CodeActions/SyntaxCodeActions.code
  Swift/CodeActions/SyntaxRefactoringCodeActionProvider.code
  Swift/CodeCompletion.code
  Swift/CodeCompletionSession.code
  Swift/CommentXML.code
  Swift/CursorInfo.code
  Swift/Diagnostic.code
  Swift/DiagnosticReportManager.code
  Swift/DocumentFormatting.code
  Swift/DocumentSymbols.code
  Swift/ExpandMacroCommand.code
  Swift/FoldingRange.code
  Swift/GeneratedInterfaceDocumentURLData.code
  Swift/GeneratedInterfaceManager.code
  Swift/GeneratedInterfaceManager.code
  Swift/MacroExpansion.code
  Swift/MacroExpansionReferenceDocumentURLData.code
  Swift/OpenInterface.code
  Swift/RefactoringEdit.code
  Swift/RefactoringResponse.code
  Swift/ReferenceDocumentURL.code
  Swift/RelatedIdentifiers.code
  Swift/RewriteSourceKitPlaceholders.code
  Swift/SemanticRefactorCommand.code
  Swift/SemanticRefactoring.code
  Swift/SemanticTokens.code
  Swift/SwiftCodeLensScanner.code
  Swift/SwiftCommand.code
  Swift/SwiftLanguageService.code
  Swift/SwiftTestingScanner.code
  Swift/SymbolInfo.code
  Swift/SyntacticSwiftXCTestScanner.code
  Swift/SyntacticTestIndex.code
  Swift/SyntaxHighlightingToken.code
  Swift/SyntaxHighlightingTokenParser.code
  Swift/SyntaxHighlightingTokens.code
  Swift/SyntaxTreeManager.code
  Swift/VariableTypeInfo.code
  Swift/WithSnapshotFromDiskOpenedInSourcekitd.code
)
set_target_properties(SourceKitLSP PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Swift_MODULE_DIRECTORY})
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
  SwiftExtensions
  ToolchainRegistry
  IndexStoreDB
  SwiftSyntax::SwiftBasicFormat
  SwiftSyntax::SwiftDiagnostics
  SwiftSyntax::SwiftIDEUtils
  SwiftSyntax::SwiftParser
  SwiftSyntax::SwiftParserDiagnostics
  SwiftSyntax::SwiftRefactor
  SwiftSyntax::SwiftSyntax)
target_link_libraries(SourceKitLSP PRIVATE
  PackageModelSyntax
  TSCExtensions
  $<$<NOT:$<PLATFORM_ID:Darwin>>:FoundationXML>)

