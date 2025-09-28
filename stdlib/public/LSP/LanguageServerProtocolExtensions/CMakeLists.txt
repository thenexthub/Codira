
add_library(LanguageServerProtocolExtensions STATIC
  Connection+Send.code
  DocumentURI+CustomLogStringConvertible.code
  DocumentURI+symlinkTarget.code
  Language+Inference.code
  LocalConnection.code
  QueueBasedMessageHandler.code
  RequestAndReply.code
  ResponseError+Init.code
  WorkDoneProgressManager.code
)
set_target_properties(LanguageServerProtocolExtensions PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(LanguageServerProtocolExtensions PUBLIC
  SourceKitD)
target_link_libraries(LanguageServerProtocolExtensions PRIVATE
  LanguageServerProtocol
  LanguageServerProtocolJSONRPC
  SKLogging
  CodiraExtensions
  TSCBasic
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>)
