add_language_host_library(languageIndex STATIC
  Index.cpp
  IndexDataConsumer.cpp
  IndexRecord.cpp
  IndexSymbol.cpp)
target_link_libraries(languageIndex PRIVATE
  languageAST
  languageIDE)
