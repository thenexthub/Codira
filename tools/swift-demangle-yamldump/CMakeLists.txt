add_language_host_tool(language-demangle-yamldump
  language-demangle-yamldump.cpp
  TOOLCHAIN_LINK_COMPONENTS support
  LANGUAGE_COMPONENT testsuite-tools
  )
target_link_libraries(language-demangle-yamldump
                      PRIVATE
                        languageDemangling)
