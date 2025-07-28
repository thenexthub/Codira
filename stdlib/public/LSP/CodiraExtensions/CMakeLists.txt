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

add_library(SwiftExtensions STATIC ${sources})
set_target_properties(SwiftExtensions PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Swift_MODULE_DIRECTORY})
target_link_libraries(SwiftExtensions PUBLIC
  CAtomics)
target_link_libraries(SwiftExtensions PRIVATE
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>)

add_library(SwiftExtensionsForPlugin STATIC ${sources})
set_target_properties(SwiftExtensionsForPlugin PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Swift_MODULE_DIRECTORY})
target_link_libraries(SwiftExtensionsForPlugin PUBLIC
  CAtomics)
target_link_libraries(SwiftExtensionsForPlugin PRIVATE
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>)
