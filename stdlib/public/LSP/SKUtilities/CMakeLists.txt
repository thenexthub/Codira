set(sources
  Debouncer.code
  Dictionary+InitWithElementsKeyedBy.code
  LineTable.code
  LRUCache.code
)

add_library(SKUtilities STATIC ${sources})
set_target_properties(SKUtilities PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(SKUtilities PRIVATE
  SKLogging
  CodiraExtensions
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>)

add_library(SKUtilitiesForPlugin STATIC ${sources})
set_target_properties(SKUtilitiesForPlugin PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_compile_options(SKUtilitiesForPlugin PRIVATE
  $<$<COMPILE_LANGUAGE:Codira>:
    "SHELL:-module-alias SKLogging=SKLoggingForPlugin"
    "SHELL:-module-alias CodiraExtensions=CodiraExtensionsForPlugin"
  >)
target_link_libraries(SKUtilitiesForPlugin PRIVATE
  SKLoggingForPlugin
  CodiraExtensionsForPlugin
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>)
