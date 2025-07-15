add_language_host_tool(language-ide-test
  language-ide-test.cpp
  ModuleAPIDiff.cpp
  XMLValidator.cpp
  LANGUAGE_COMPONENT tools
  THINLTO_LD64_ADD_FLTO_CODEGEN_ONLY
  HAS_LANGUAGE_MODULES
)
target_link_libraries(language-ide-test
                      PRIVATE
                        languageAST
                        languageDriver
                        languageFrontend
                        languageIDE
                        languageIDETool
                        languageCompilerModules)

# If libxml2 is available, make it available for language-ide-test.
if(TOOLCHAIN_ENABLE_LIBXML2)
  target_link_libraries(language-ide-test PRIVATE LibXml2::LibXml2)
  target_compile_definitions(language-ide-test PRIVATE LANGUAGE_HAVE_LIBXML=1)
endif()

# Create a symlink for language-api-dump.py in the bin directory
language_create_post_build_symlink(language-ide-test
  SOURCE "${LANGUAGE_SOURCE_DIR}/utils/language-api-dump.py"
  DESTINATION "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language-api-dump.py"
  COMMENT "Creating development symlink for language-api-dump.py.")

