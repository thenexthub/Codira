

set(languageDriver_sources
  Action.cpp
  Compilation.cpp
  DarwinToolChains.cpp
  Driver.cpp
  FrontendUtil.cpp
  Job.cpp
  PrettyStackTrace.cpp
  ToolChain.cpp
  ToolChains.cpp
  UnixToolChains.cpp
  WebAssemblyToolChains.cpp
  WindowsToolChains.cpp
  )

set(languageDriver_targetDefines)

add_language_host_library(languageDriver STATIC
  ${languageDriver_sources})
add_dependencies(languageDriver
  CodiraOptions)
target_link_libraries(languageDriver PRIVATE
  languageAST
  languageBasic
  languageOption)

set_language_toolchain_is_available(languageDriver)
