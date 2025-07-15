
find_package(dispatch QUIET REQUIRED)

check_symbol_exists("dispatch_async_language_job" "dispatch/private.h"
  CodiraConcurrency_HAS_DISPATCH_ASYNC_LANGUAGE_JOB)

target_sources(language_Concurrency PRIVATE
  DispatchGlobalExecutor.cpp
  DispatchExecutor.code
  CFExecutor.code
  ExecutorImpl.code)
target_compile_definitions(language_Concurrency PRIVATE
  $<$<BOOL:${CodiraConcurrency_HAS_DISPATCH_ASYNC_LANGUAGE_JOB}>:-DCodiraConcurrency_HAS_DISPATCH_ASYNC_LANGUAGE_JOB=1>
  $<$<COMPILE_LANGUAGE:C,CXX>:-DLANGUAGE_CONCURRENCY_USES_DISPATCH=1>)
target_compile_options(language_Concurrency PRIVATE
  $<$<COMPILE_LANGUAGE:Codira>:-DLANGUAGE_CONCURRENCY_USES_DISPATCH>
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xcc -DLANGUAGE_CONCURRENCY_USES_DISPATCH>")
target_link_libraries(language_Concurrency PRIVATE
  dispatch)
