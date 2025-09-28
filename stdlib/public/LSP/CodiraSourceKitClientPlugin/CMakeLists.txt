add_library(CodiraSourceKitClientPlugin SHARED
  ClientPlugin.code)

set_target_properties(CodiraSourceKitClientPlugin PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_compile_options(CodiraSourceKitClientPlugin PRIVATE 
  $<$<COMPILE_LANGUAGE:Codira>:
    "SHELL:-module-alias SourceKitD=SourceKitDForPlugin"
    "SHELL:-module-alias CodiraExtensions=CodiraExtensionsForPlugin"
  >)
target_link_libraries(CodiraSourceKitClientPlugin PRIVATE
  Csourcekitd
  SourceKitD
  CodiraExtensions
  CodiraSourceKitPluginCommon
  $<$<NOT:$<PLATFORM_ID:Darwin>>:FoundationXML>)

install(TARGETS CodiraSourceKitClientPlugin DESTINATION lib)
