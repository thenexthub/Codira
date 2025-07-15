
add_language_host_library(languageIDE STATIC
  AfterPoundExprCompletion.cpp
  APIDigesterData.cpp
  ArgumentCompletion.cpp
  CodeCompletion.cpp
  CodeCompletionCache.cpp
  CodeCompletionContext.cpp
  CodeCompletionDiagnostics.cpp
  CodeCompletionResult.cpp
  CodeCompletionResultBuilder.cpp
  CodeCompletionResultPrinter.cpp
  CodeCompletionResultType.cpp
  CodeCompletionString.cpp
  CodeCompletionStringPrinter.cpp
  CommentConversion.cpp
  CompletionLookup.cpp
  CompletionOverrideLookup.cpp
  ConformingMethodList.cpp
  CursorInfo.cpp
  ExprCompletion.cpp
  ExprContextAnalysis.cpp
  Formatting.cpp
  FuzzyStringMatcher.cpp
  IDEBridging.cpp
  IDERequests.cpp
  IDETypeChecking.cpp
  ImportDepth.cpp
  KeyPathCompletion.cpp
  ModuleInterfacePrinting.cpp
  PostfixCompletion.cpp
  REPLCodeCompletion.cpp
  SelectedOverloadInfo.cpp
  SourceEntityWalker.cpp
  CodiraSourceDocInfo.cpp
  SyntaxModel.cpp
  TypeCheckCompletionCallback.cpp
  TypeContextInfo.cpp
  UnresolvedMemberCompletion.cpp
  Utils.cpp
  )
target_link_libraries(languageIDE PRIVATE
  languageAST
  languageClangImporter
  languageDriver
  languageParse
  languageSema)

if (LANGUAGE_BUILD_LANGUAGE_SYNTAX)
  target_link_libraries(languageIDE PRIVATE
    languageIDEUtilsBridging
  )
endif()

set_language_toolchain_is_available(languageIDE)
