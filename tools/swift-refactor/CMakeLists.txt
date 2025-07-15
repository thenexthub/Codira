add_language_host_tool(language-refactor
  language-refactor.cpp
  LANGUAGE_COMPONENT testsuite-tools
  TOOLCHAIN_LINK_COMPONENTS support
)
target_link_libraries(language-refactor
                      PRIVATE
                        languageDriver
                        languageFrontend
                        languageIDE
                        languageRefactoring)
