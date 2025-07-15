add_language_host_tool(language-scan-test
  language-scan-test.cpp
  TOOLCHAIN_LINK_COMPONENTS support
  LANGUAGE_COMPONENT tools
  THINLTO_LD64_ADD_FLTO_CODEGEN_ONLY
  DOES_NOT_USE_LANGUAGE
)

target_link_libraries(language-scan-test
                      PRIVATE
                      languageBasic
                      languageDependencyScanImpl
                      libCodiraScan)
