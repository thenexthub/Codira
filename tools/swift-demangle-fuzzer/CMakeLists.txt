add_language_fuzzer_host_tool(language-demangle-fuzzer
  language-demangle-fuzzer.cpp
  TOOLCHAIN_LINK_COMPONENTS support
  LANGUAGE_COMPONENT compiler
  )
target_link_libraries(language-demangle-fuzzer
                      PRIVATE
                        languageDemangling)
