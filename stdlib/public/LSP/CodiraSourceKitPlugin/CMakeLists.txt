add_library(CodiraSourceKitPlugin SHARED
  ASTCompletion/ASTCompletionItem.code
  ASTCompletion/CompletionSession.code
  SKDResponse.code
  SKDResponseArrayBuilder.code
  SourceKitPluginError.code
  SKDRequestDictionaryReader.code
  SKDRequestArrayReader.code
  CompletionResultsArrayBuilder.code
  SKDResponseValue.code
  CompletionProvider.code
  SKDResponseDictionaryBuilder.code
  CodeCompletion/Location.code
  CodeCompletion/Connection.code
  CodeCompletion/CompletionContext.code
  CodeCompletion/Completion+ItemKind.code
  CodeCompletion/Completion+SemanticContext.code
  CodeCompletion/Completion+Identifier.code
  CodeCompletion/TextEdit.code
  CodeCompletion/Completion+TypeRelation.code
  CodeCompletion/CompletionItem.code
  CodeCompletion/WithArrayOfCStrings.code
  CodeCompletion/CompletionOptions.code
  CodeCompletion/Completion+Diagnostic.code
  CodeCompletion/CompletionSorting.code
  CodeCompletion/Completion+PriorityBucket.code
  CodeCompletion/Position.code
  SourceKitDWrappers.code
  Plugin.code)

set_target_properties(CodiraSourceKitPlugin PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_compile_options(CodiraSourceKitPlugin PRIVATE 
  $<$<COMPILE_LANGUAGE:Codira>:
    "SHELL:-module-alias CompletionScoring=CompletionScoringForPlugin"
    "SHELL:-module-alias SKUtilities=SKUtilitiesForPlugin"
    "SHELL:-module-alias SourceKitD=SourceKitDForPlugin"
    "SHELL:-module-alias SKLogging=SKLoggingForPlugin"
    "SHELL:-module-alias CodiraExtensions=CodiraExtensionsForPlugin"
  >)
target_link_libraries(CodiraSourceKitPlugin PRIVATE
  Csourcekitd
  CompletionScoringForPlugin
  SKUtilitiesForPlugin
  SKLoggingForPlugin
  SourceKitDForPlugin
  CodiraSourceKitPluginCommon
  CodiraExtensionsForPlugin
  $<$<NOT:$<PLATFORM_ID:Darwin>>:FoundationXML>)

install(TARGETS CodiraSourceKitPlugin DESTINATION lib)
