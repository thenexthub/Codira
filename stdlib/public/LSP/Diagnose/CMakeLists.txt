add_library(Diagnose STATIC
  CommandLineArgumentsReducer.code
  DebugCommand.code
  DiagnoseCommand.code
  GenericError.code
  IndexCommand.code
  MergeSwiftFiles.code
  OSLogScraper.code
  ReduceCommand.code
  ReduceFrontendCommand.code
  ReduceSourceKitDRequest.code
  ReduceSwiftFrontend.code
  ReproducerBundle.code
  RequestInfo.code
  RunSourcekitdRequestCommand.code
  SourceKitD+RunWithYaml.code
  SourceKitDRequestExecutor.code
  SourceReducer.code
  StderrStreamConcurrencySafe.code
  SwiftFrontendCrashScraper.code
  Toolchain+SwiftFrontend.code
  Toolchain+PluginPaths.code
  TraceFromSignpostsCommand.code)

set_target_properties(Diagnose PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Swift_MODULE_DIRECTORY})

target_link_libraries(Diagnose PUBLIC
  BuildSystemIntegration
  InProcessClient
  SKLogging
  SKOptions
  SourceKitD
  SwiftExtensions
  ToolchainRegistry
  ArgumentParser
  SwiftSyntax::SwiftIDEUtils
  SwiftSyntax::SwiftSyntax
  SwiftSyntax::SwiftParser
  TSCBasic
)

target_link_libraries(Diagnose PRIVATE
  SKUtilities
  TSCExtensions
)
