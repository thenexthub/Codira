add_language_fuzzer_host_tool(language-reflection-fuzzer
  language-reflection-fuzzer.cpp
  TOOLCHAIN_LINK_COMPONENTS support
  LANGUAGE_COMPONENT testsuite-tools
  )
target_link_libraries(language-reflection-fuzzer
                      PRIVATE
                        languageRemoteInspection)
