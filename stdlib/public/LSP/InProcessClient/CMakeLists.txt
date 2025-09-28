add_library(InProcessClient STATIC
  InProcessSourceKitLSPClient.code)

set_target_properties(InProcessClient PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})

target_link_libraries(InProcessClient PUBLIC
  BuildSystemIntegration
  LanguageServerProtocol
  SKLogging
  SKOptions
  SourceKitLSP
  ToolchainRegistry
)

target_link_libraries(InProcessClient PRIVATE
  TSCExtensions
)
