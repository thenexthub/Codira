
add_language_host_library(languageFrontendTool STATIC
  FrontendTool.cpp
  ImportedModules.cpp
  LoadedModuleTrace.cpp
  TBD.cpp)
add_dependencies(languageFrontendTool
  CodiraOptions)
target_link_libraries(languageFrontendTool INTERFACE
  clangAPINotes
  clangBasic)
target_link_libraries(languageFrontendTool PRIVATE
    languageClangImporter
    languageConstExtract
    languageDemangling
    languageDependencyScan
    languageFrontend
    languageImmediate
    languageIndex
    languageIRGen
    languageMigrator
    languageOption
    languagePrintAsClang
    languageSerialization
    languageSIL
    languageSILGen
    languageSILOptimizer
    languageThreading)

set_language_toolchain_is_available(languageFrontendTool)
