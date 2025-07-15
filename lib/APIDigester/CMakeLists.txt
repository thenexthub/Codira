

add_language_host_library(languageAPIDigester STATIC
  ModuleAnalyzerNodes.cpp
  ModuleDiagsConsumer.cpp)

target_link_libraries(languageAPIDigester PRIVATE
  languageIDE)

set_language_toolchain_is_available(languageAPIDigester)
