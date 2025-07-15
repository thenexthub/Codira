add_language_host_library(languageImmediate STATIC
  CodiraMaterializationUnit.cpp
  Immediate.cpp
  TOOLCHAIN_LINK_COMPONENTS
    executionengine
    linker
    mcjit
    orcjit
    orctargetprocess
    jitlink
    transformutils)
target_link_libraries(languageImmediate PRIVATE
  languageFrontend
  languageIDE
  languageIRGen
  languageSILGen
  languageSILOptimizer)
