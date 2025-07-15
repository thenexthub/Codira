
add_language_host_library(languageFrontend STATIC
  ArgsToFrontendInputsConverter.cpp
  ArgsToFrontendOptionsConverter.cpp
  ArgsToFrontendOutputsConverter.cpp
  CachedDiagnostics.cpp
  CachingUtils.cpp
  CASOutputBackends.cpp
  CompileJobCacheKey.cpp
  CompileJobCacheResult.cpp
  CompilerInvocation.cpp
  DependencyVerifier.cpp
  DiagnosticHelper.cpp
  DiagnosticVerifier.cpp
  Frontend.cpp
  FrontendInputsAndOutputs.cpp
  FrontendOptions.cpp
  MakeStyleDependencies.cpp
  ModuleInterfaceBuilder.cpp
  ModuleInterfaceLoader.cpp
  ModuleInterfaceSupport.cpp
  PrintingDiagnosticConsumer.cpp
  Serialization.cpp
  SerializedDiagnosticConsumer.cpp)
add_dependencies(languageFrontend
  CodiraOptions)
target_link_libraries(languageFrontend PRIVATE
  languageAPIDigester
  languageAST
  languageConstExtract
  languageIDE
  languageSIL
  languageOption
  languageSILGen
  languageSILOptimizer
  languageIRGen
  languageLocalization
  languageSema
  languageSerialization
  languageSymbolGraphGen)

if (LANGUAGE_BUILD_LANGUAGE_SYNTAX)
  target_link_libraries(languageFrontend
    PRIVATE
    languageASTGen
  )

  add_dependencies(languageFrontend
    languageASTGen
  )
endif()

set_language_toolchain_is_available(languageFrontend)
