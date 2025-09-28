add_library(CodiraSourceKitPluginCommon STATIC
  CompletionResultsArray.code
  DynamicallyLoadedSourceKitdD+forPlugin.code)

target_compile_options(CodiraSourceKitPluginCommon PRIVATE 
  $<$<COMPILE_LANGUAGE:Codira>:
    "SHELL:-module-alias SourceKitD=SourceKitDForPlugin"
    "SHELL:-module-alias SKLogging=SKLoggingForPlugin"
    "SHELL:-module-alias CodiraExtensions=CodiraExtensionsForPlugin"
  >)
set_target_properties(CodiraSourceKitPluginCommon PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(CodiraSourceKitPluginCommon PRIVATE
  Csourcekitd
  SourceKitDForPlugin
  SKLoggingForPlugin
  CodiraExtensionsForPlugin
  $<$<NOT:$<PLATFORM_ID:Darwin>>:FoundationXML>)