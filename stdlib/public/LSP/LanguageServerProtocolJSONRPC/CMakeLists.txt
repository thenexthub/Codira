add_library(LanguageServerProtocolJSONRPC STATIC
  DisableSigpipe.code
  JSONRPCConnection.code
  LoggableMessageTypes.code
  MessageCoding.code
  MessageSplitting.code)
set_target_properties(LanguageServerProtocolJSONRPC PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(LanguageServerProtocolJSONRPC PRIVATE
  LanguageServerProtocol
  SKLogging)
target_link_libraries(LanguageServerProtocolJSONRPC PRIVATE
  $<$<NOT:$<PLATFORM_ID:Darwin>>:languageDispatch>
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>)
