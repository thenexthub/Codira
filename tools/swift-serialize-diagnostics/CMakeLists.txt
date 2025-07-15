add_language_host_tool(language-serialize-diagnostics
  language-serialize-diagnostics.cpp
  LANGUAGE_COMPONENT tools
  DOES_NOT_USE_LANGUAGE
)
target_link_libraries(language-serialize-diagnostics PRIVATE
  languageLocalization)
