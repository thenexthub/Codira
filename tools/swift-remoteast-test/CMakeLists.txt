add_language_host_tool(language-remoteast-test
  language-remoteast-test.cpp
  LANGUAGE_COMPONENT testsuite-tools
  HAS_LANGUAGE_MODULES
  THINLTO_LD64_ADD_FLTO_CODEGEN_ONLY
)
target_link_libraries(language-remoteast-test
                      PRIVATE
                        languageFrontendTool
                        languageRemoteAST
                        languageCompilerModules)
set_target_properties(language-remoteast-test PROPERTIES ENABLE_EXPORTS 1)

# If building as part of clang, make sure the headers are installed.
if(NOT LANGUAGE_BUILT_STANDALONE)
  add_dependencies(language-frontend clang-resource-headers)
endif()

