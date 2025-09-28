
add_library(SemanticIndex STATIC
  CheckedIndex.code
  IndexTaskDescription.code
  IndexHooks.code
  PreparationTaskDescription.code
  SemanticIndexManager.code
  TaskScheduler.code
  UpdateIndexStoreTaskDescription.code
  UpToDateTracker.code
)
set_target_properties(SemanticIndex PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(SemanticIndex PRIVATE
  BuildSystemIntegration
  LanguageServerProtocol
  SKLogging
  CodiraExtensions
  ToolchainRegistry
  TSCExtensions
  IndexStoreDB
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>)
