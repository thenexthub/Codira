add_language_host_tool(language-def-to-strings-converter
  language-def-to-strings-converter.cpp
  LANGUAGE_COMPONENT tools
  DOES_NOT_USE_LANGUAGE
)

target_link_libraries(language-def-to-strings-converter PRIVATE
  languageLocalization)
