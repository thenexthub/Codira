
add_library(TSCExtensions STATIC
  AbsolutePath+Init.code
  ByteString.code
  Process+Run.code
  SwitchableProcessResultExitStatus.code
  URL+appendingRelativePath.code
)
set_target_properties(TSCExtensions PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(TSCExtensions PRIVATE
  SKLogging
  CodiraExtensions
  TSCBasic
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>)
