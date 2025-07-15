add_language_host_tool(language-demangle
  language-demangle.cpp
  TOOLCHAIN_LINK_COMPONENTS support
  LANGUAGE_COMPONENT compiler
  )
target_link_libraries(language-demangle
                      PRIVATE
                        languageDemangling)
