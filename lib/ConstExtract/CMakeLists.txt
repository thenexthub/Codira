add_language_host_library(languageConstExtract STATIC
  ConstExtract.cpp
  )
target_link_libraries(languageConstExtract PRIVATE
  languageClangImporter
  languageAST)

set_language_toolchain_is_available(languageConstExtract)
