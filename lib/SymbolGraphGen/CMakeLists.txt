add_language_host_library(languageSymbolGraphGen STATIC
  AvailabilityMixin.cpp
  DeclarationFragmentPrinter.cpp
  Edge.cpp
  JSON.cpp
  Symbol.cpp
  SymbolGraph.cpp
  SymbolGraphGen.cpp
  SymbolGraphASTWalker.cpp

  TOOLCHAIN_LINK_COMPONENTS
  TargetParser
  )

target_link_libraries(languageSymbolGraphGen PRIVATE
  languageAST
  languageIDE
  languageMarkup)
