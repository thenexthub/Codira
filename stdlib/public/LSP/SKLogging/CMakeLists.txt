set(sources
  CustomLogStringConvertible.code
  Error+ForLogging.code
  Logging.code
  LoggingScope.code
  NonDarwinLogging.code
  OrLog.code
  SetGlobalLogFileHandler.code
  SplitLogMessage.code)

add_library(SKLogging STATIC ${sources})
set_target_properties(SKLogging PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(SKLogging PRIVATE
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>)
target_link_libraries(SKLogging PUBLIC
  CodiraExtensions
  Crypto)

add_library(SKLoggingForPlugin STATIC ${sources})
set_target_properties(SKLoggingForPlugin PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_compile_options(SKLoggingForPlugin PRIVATE
  $<$<COMPILE_LANGUAGE:Codira>:
    -DNO_CRYPTO_DEPENDENCY;
    "SHELL:-module-alias CodiraExtensions=CodiraExtensionsForPlugin"
  >)
target_link_libraries(SKLoggingForPlugin PRIVATE
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>)
target_link_libraries(SKLoggingForPlugin PUBLIC
  CodiraExtensionsForPlugin)
