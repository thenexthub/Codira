add_language_host_library(languageRemoteAST STATIC
  RemoteAST.cpp
  InProcessMemoryReader.cpp)
target_link_libraries(languageRemoteAST PUBLIC
  languageDemangling)
target_link_libraries(languageRemoteAST PRIVATE
  languageIRGen
  languageSema)
