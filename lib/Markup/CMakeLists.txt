add_language_host_library(languageMarkup STATIC
  AST.cpp
  LineList.cpp
  Markup.cpp)
target_link_libraries(languageMarkup PRIVATE
  libcmark-gfm)
