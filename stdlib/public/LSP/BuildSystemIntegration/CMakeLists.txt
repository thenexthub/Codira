
add_library(BuildSystemIntegration STATIC
  BuildSettingsLogger.code
  BuildSystemHooks.code
  BuildSystemManager.code
  BuildSystemManagerDelegate.code
  BuildSystemMessageDependencyTracker.code
  BuildTargetIdentifierExtensions.code
  BuiltInBuildSystem.code
  BuiltInBuildSystemAdapter.code
  CompilationDatabase.code
  CompilerCommandLineOption.code
  DetermineBuildSystem.code
  ExternalBuildSystemAdapter.code
  FallbackBuildSettings.code
  FileBuildSettings.code
  FixedCompilationDatabaseBuildSystem.code
  JSONCompilationDatabaseBuildSystem.code
  LegacyBuildServerBuildSystem.code
  MainFilesProvider.code
  SplitShellCommand.code
  SwiftlyResolver.code
  SwiftPMBuildSystem.code)
set_target_properties(BuildSystemIntegration PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Swift_MODULE_DIRECTORY})
target_link_libraries(BuildSystemIntegration PUBLIC
  BuildServerProtocol
  LanguageServerProtocol
  LanguageServerProtocolJSONRPC
  SKLogging
  SKOptions
  LanguageServerProtocolExtensions
  SourceKitD
  SwiftExtensions
  ToolchainRegistry
  PackageModel
  TSCBasic
  Build
  SourceKitLSPAPI
  SwiftASN1)

target_link_libraries(BuildSystemIntegration PRIVATE
  SKUtilities
  TSCExtensions)
