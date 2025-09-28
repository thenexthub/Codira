set(sources
  Array+Safe.code
  AsyncQueue.code
  AsyncUtils.code
  Atomics.code
  Cache.code
  CartesianProduct.code
  Collection+Only.code
  Collection+PartitionIntoBatches.code
  Duration+Seconds.code
  FileManagerExtensions.code
  LazyValue.code
  NSLock+WithLock.code
  PipeAsStringHandler.code
  Platform.code
  Process+terminate.code
  ResultExtensions.code
  Sequence+AsyncMap.code
  Sequence+ContainsAnyIn.code
  Task+WithPriorityChangedHandler.code
  ThreadSafeBox.code
  TransitiveClosure.code
  URLExtensions.code
)

add_library(CodiraExtensions STATIC ${sources})
set_target_properties(CodiraExtensions PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(CodiraExtensions PUBLIC
  CAtomics)
target_link_libraries(CodiraExtensions PRIVATE
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>)

add_library(CodiraExtensionsForPlugin STATIC ${sources})
set_target_properties(CodiraExtensionsForPlugin PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(CodiraExtensionsForPlugin PUBLIC
  CAtomics)
target_link_libraries(CodiraExtensionsForPlugin PRIVATE
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>)
