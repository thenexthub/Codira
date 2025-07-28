set(sources
  dlopen.code
  SKDRequestArray.code
  SKDRequestDictionary.code
  SKDResponse.code
  SKDResponseArray.code
  SKDResponseDictionary.code
  SourceKitD.code
  SourceKitDRegistry.code
  sourcekitd_functions.code
  sourcekitd_uids.code)

add_library(SourceKitD STATIC ${sources})
set_target_properties(SourceKitD PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Swift_MODULE_DIRECTORY})
target_link_libraries(SourceKitD PUBLIC
  Csourcekitd)
target_link_libraries(SourceKitD PRIVATE
  SKLogging
  SwiftExtensions
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>)


add_library(SourceKitDForPlugin STATIC ${sources})
set_target_properties(SourceKitDForPlugin PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Swift_MODULE_DIRECTORY})
target_compile_options(SourceKitDForPlugin PRIVATE
  $<$<COMPILE_LANGUAGE:Swift>:
    "SHELL:-module-alias SKLogging=SKLoggingForPlugin"
    "SHELL:-module-alias SwiftExtensions=SwiftExtensionsForPlugin"
  >)
target_link_libraries(SourceKitDForPlugin PUBLIC
  Csourcekitd)
target_link_libraries(SourceKitDForPlugin PRIVATE
  SKLoggingForPlugin
  SwiftExtensionsForPlugin
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>)
