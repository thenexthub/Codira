add_language_host_tool(language-ast-script
  ASTScriptConfiguration.cpp
  ASTScriptParser.cpp
  ASTScriptEvaluator.cpp
  language-ast-script.cpp
  LANGUAGE_COMPONENT tools
  THINLTO_LD64_ADD_FLTO_CODEGEN_ONLY
)
target_link_libraries(language-ast-script
                      PRIVATE
                        languageAST
                        languageFrontendTool)
