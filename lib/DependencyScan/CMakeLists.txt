add_language_host_library(languageDependencyScanImpl STATIC
  DependencyScanJSON.cpp
  StringUtils.cpp)

add_language_host_library(languageDependencyScan STATIC
  DependencyScanningTool.cpp
  ModuleDependencyCacheSerialization.cpp
  ModuleDependencyScanner.cpp
  ScanDependencies.cpp)

target_link_libraries(languageDependencyScan INTERFACE
  clangBasic)

target_link_libraries(languageDependencyScan PRIVATE
  languageClangImporter
  languageAST
  languageSerialization)

target_link_libraries(languageDependencyScan PUBLIC
  languageDependencyScanImpl)
