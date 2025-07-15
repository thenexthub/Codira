add_language_host_tool(lldb-moduleimport-test
  lldb-moduleimport-test.cpp
  LANGUAGE_COMPONENT testsuite-tools
  HAS_LANGUAGE_MODULES
)
target_link_libraries(lldb-moduleimport-test
                      PRIVATE
                        languageAST
                        languageASTSectionImporter
                        languageClangImporter
                        languageFrontend
                        languageCompilerModules)

