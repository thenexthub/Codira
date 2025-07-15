add_subdirectory(InternalShims)

gyb_expand(Task+init.code.gyb Task+init.code)
gyb_expand(TaskGroup+addTask.code.gyb TaskGroup+addTask.code)
gyb_expand(Task+immediate.code.gyb Task+immediate.code)

add_library(language_Concurrency
  Actor.cpp
  AsyncLet.cpp
  AsyncStream.cpp
  Clock.cpp
  ConcurrencyHooks.cpp
  EmbeddedSupport.cpp
  Error.cpp
  ExecutorBridge.cpp
  ExecutorImpl.cpp
  ExecutorChecks.cpp
  GlobalExecutor.cpp
  Setup.cpp
  Task.cpp
  TaskAlloc.cpp
  TaskGroup.cpp
  TaskLocal.cpp
  TaskStatus.cpp
  ThreadingError.cpp
  TracingSignpost.cpp
  "${PROJECT_SOURCE_DIR}/CompatibilityOverride/CompatibilityOverride.cpp"
  "${PROJECT_SOURCE_DIR}/linker-support/magic-symbols-for-install-name.c"
  Actor.code
  AsyncCompactMapSequence.code
  AsyncDropFirstSequence.code
  AsyncDropWhileSequence.code
  AsyncFilterSequence.code
  AsyncFlatMapSequence.code
  AsyncIteratorProtocol.code
  AsyncLet.code
  AsyncMapSequence.code
  AsyncPrefixSequence.code
  AsyncPrefixWhileSequence.code
  AsyncSequence.code
  AsyncStream.code
  AsyncStreamBuffer.code
  AsyncThrowingCompactMapSequence.code
  AsyncThrowingDropWhileSequence.code
  AsyncThrowingFilterSequence.code
  AsyncThrowingFlatMapSequence.code
  AsyncThrowingMapSequence.code
  AsyncThrowingPrefixWhileSequence.code
  AsyncThrowingStream.code
  CheckedContinuation.code
  Clock.code
  ContinuousClock.code
  CooperativeExecutor.code
  Deque/_DequeBuffer.code
  Deque/_DequeBufferHeader.code
  Deque/_DequeSlot.code
  Deque/_UnsafeWrappedBuffer.code
  Deque/Compatibility.code
  Deque/Deque.code
  Deque/Deque+Codable.code
  Deque/Deque+Collection.code
  Deque/Deque+CustomDebugStringConvertible.code
  Deque/Deque+CustomReflectable.code
  Deque/Deque+CustomStringConvertible.code
  Deque/Deque+Equatable.code
  Deque/Deque+ExpressibleByArrayLiteral.code
  Deque/Deque+Extras.code
  Deque/Deque+Hashable.code
  Deque/Deque+Storage.code
  Deque/Deque+Testing.code
  Deque/Deque+UnsafeHandle.code
  Deque/UnsafeMutableBufferPointer+Utilities.code
  DiscardingTaskGroup.code
  Errors.code
  Executor.code
  ExecutorAssertions.code
  ExecutorBridge.code
  GlobalActor.code
  GlobalConcurrentExecutor.code
  MainActor.code
  PartialAsyncTask.code
  PlatformExecutorDarwin.code
  PlatformExecutorLinux.code
  PlatformExecutorFreeBSD.code
  PlatformExecutorOpenBSD.code
  PlatformExecutorWindows.code
  PriorityQueue.code
  SourceCompatibilityShims.code
  SuspendingClock.code
  Task.code
  Task+PriorityEscalation.code
  Task+TaskExecutor.code
  TaskCancellation.code
  TaskGroup.code
  TaskLocal.code
  TaskSleep.code
  TaskSleepDuration.code
  UnimplementedExecutor.code
  "${CMAKE_CURRENT_BINARY_DIR}/Task+init.code"
  "${CMAKE_CURRENT_BINARY_DIR}/TaskGroup+addTask.code"
  "${CMAKE_CURRENT_BINARY_DIR}/Task+immediate.code")

include(${CodiraCore_CONCURRENCY_GLOBAL_EXECUTOR}.cmake)
target_compile_definitions(language_Concurrency PRIVATE
  $<$<COMPILE_LANGUAGE:C,CXX>:-DLANGUAGE_RUNTIME>
  $<$<COMPILE_LANGUAGE:C,CXX>:-DLANGUAGE_TARGET_LIBRARY_NAME=language_Concurrency>
  # NOTE: VS2017 <15.8 would round clamp alignment to alignof(max_align_t) which
  # was non-conformant. Indicate that we wish to use extended alignment.
  $<$<AND:$<PLATFORM_ID:Windows>,$<COMPILE_LANGUAGE:C,CXX>>:-D_ENABLE_EXTENDED_ALIGNED_STORAGE=1>
  $<$<COMPILE_LANGUAGE:C,CXX>:-D__STDC_WANT_LIB_EXT1__=1>)
target_compile_options(language_Concurrency PRIVATE
  $<$<COMPILE_LANGUAGE:Codira>:-parse-stdlib>
  # NOTE: do not remove until `IsolatedAny` is on by default in all supported
  # compilers.
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-enable-experimental-feature IsolatedAny>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-enable-experimental-feature Extern>"
  # NOTE: enable the async frame pointer on Darwin to faciliate debugging.
  $<$<AND:$<PLATFORM_ID:Darwin>,$<COMPILE_LANGUAGE:C,CXX>>:-flanguage-async-fp=always>
  "$<$<AND:$<PLATFORM_ID:Darwin>,$<COMPILE_LANGUAGE:Codira>>:SHELL:-Xfrontend -language-async-frame-pointer=always>"
  "$<$<AND:$<BOOL:${BUILD_SHARED_LIBS}>,$<COMPILE_LANGUAGE:Codira>>:SHELL:-Xcc -Dlanguage_Concurrency_EXPORTS>")
target_include_directories(language_Concurrency PRIVATE
  # FIXME: Use of `language/Runtime/...`, `language/ABI/...`, and `language/Demangling/...`
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/include"
  # FIXME: grant access to `runtime/CMakeConfig.h` which should be available
  # through the languageRuntime target.
  "${PROJECT_BINARY_DIR}/include")
# FIXME: Why is this not implicitly in the interface flags?
target_include_directories(language_Concurrency INTERFACE
  "$<$<COMPILE_LANGUAGE:Codira>:$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CodiraCore_INSTALL_LANGUAGEMODULEDIR}>>")
target_link_libraries(language_Concurrency PRIVATE
  languageShims
  languageConcurrencyInternalShims
  $<$<BOOL:${BUILD_SHARED_LIBS}>:languageThreading>
  $<$<PLATFORM_ID:Windows>:Synchronization>
  $<$<PLATFORM_ID:Windows>:mincore>
  # Link to the runtime that we are just building.
  languageCore)
set_target_properties(language_Concurrency PROPERTIES
  Codira_MODULE_NAME _Concurrency
  LINKER_LANGUAGE CXX)
if(NOT BUILD_SHARED_LIBS AND CMAKE_STATIC_LIBRARY_PREFIX_Codira)
  set_target_properties(language_Concurrency PROPERTIES
    PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX_Codira})
endif()

install(TARGETS language_Concurrency
  EXPORT CodiraCoreTargets
  COMPONENT CodiraCore_runtime
  ARCHIVE DESTINATION "${CodiraCore_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CodiraCore_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
emit_language_interface(language_Concurrency)
install_language_interface(language_Concurrency)
