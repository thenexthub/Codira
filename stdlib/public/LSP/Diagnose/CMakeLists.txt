add_library(Diagnose STATIC
  CommandLineArgumentsReducer.code
  DebugCommand.code
  DiagnoseCommand.code
  GenericError.code
  IndexCommand.code
  MergeCodiraFiles.code
  OSLogScraper.code
  ReduceCommand.code
  ReduceFrontendCommand.code
  ReduceSourceKitDRequest.code
  ReduceCodiraFrontend.code
  ReproducerBundle.code
  RequestInfo.code
  RunSourcekitdRequestCommand.code
  SourceKitD+RunWithYaml.code
  SourceKitDRequestExecutor.code
  SourceReducer.code
  StderrStreamConcurrencySafe.code
  CodiraFrontendCrashScraper.code
  Toolchain+CodiraFrontend.code
  Toolchain+PluginPaths.code
  TraceFromSignpostsCommand.code)

set_target_properties(Diagnose PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})

target_link_libraries(Diagnose PUBLIC
  BuildSystemIntegration
  InProcessClient
  SKLogging
  SKOptions
  SourceKitD
  CodiraExtensions
  ToolchainRegistry
  ArgumentParser
  CodiraSyntax::CodiraIDEUtils
  CodiraSyntax::CodiraSyntax
  CodiraSyntax::CodiraParser
  TSCBasic
)

target_link_libraries(Diagnose PRIVATE
  SKUtilities
  TSCExtensions
)
