
add_language_host_library(languageIDETool STATIC
  CompileInstance.cpp
  CompilerInvocation.cpp
  IDEInspectionInstance.cpp
  DependencyChecking.cpp
  SyntacticMacroExpansion.cpp
  )

target_link_libraries(languageIDETool PRIVATE
  languageAST
  languageDriver
  languageFrontend
  languageFrontendTool
  clangAST
  clangFrontend)

set_language_toolchain_is_available(languageIDETool)
