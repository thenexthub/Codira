
add_library(ToolchainRegistry STATIC
  Toolchain.code
  ToolchainRegistry.code
  XCToolchainPlist.code)
set_target_properties(ToolchainRegistry PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(ToolchainRegistry PUBLIC
LanguageServerProtocolExtensions
  SKLogging
  CodiraExtensions
  PackageModel
  TSCBasic)

target_link_libraries(ToolchainRegistry PRIVATE
  SKUtilities
  TSCExtensions)
