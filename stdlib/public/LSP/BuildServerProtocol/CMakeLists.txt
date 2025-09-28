add_library(BuildServerProtocol STATIC
  Messages.code

  Messages/BuildShutdownRequest.code
  Messages/BuildTargetPrepareRequest.code
  Messages/BuildTargetSourcesRequest.code
  Messages/InitializeBuildRequest.code
  Messages/OnBuildExitNotification.code
  Messages/OnBuildInitializedNotification.code
  Messages/OnBuildLogMessageNotification.code
  Messages/OnBuildTargetDidChangeNotification.code
  Messages/OnWatchedFilesDidChangeNotification.code
  Messages/RegisterForChangeNotifications.code
  Messages/TaskFinishNotification.code
  Messages/TaskProgressNotification.code
  Messages/TaskStartNotification.code
  Messages/TextDocumentSourceKitOptionsRequest.code
  Messages/WorkspaceBuildTargetsRequest.code
  Messages/WorkspaceWaitForBuildSystemUpdates.code

  SupportTypes/BuildTarget.code
  SupportTypes/MessageType.code
  SupportTypes/MillisecondsSince1970Date.code
  SupportTypes/StatusCode.code
  SupportTypes/TaskId.code
  SupportTypes/TextDocumentIdentifier.code)
set_target_properties(BuildServerProtocol PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(BuildServerProtocol PRIVATE
  LanguageServerProtocol)
