
add_library(SKOptions STATIC
  BuildConfiguration.code
  ExperimentalFeatures.code
  SourceKitLSPOptions.code
  WorkspaceType.code)
set_target_properties(SKOptions PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(SKOptions PUBLIC
  LanguageServerProtocol
  LanguageServerProtocolExtensions
  SKLogging
  TSCBasic)
