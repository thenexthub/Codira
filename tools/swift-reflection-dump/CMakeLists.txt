add_language_host_tool(language-reflection-dump
  language-reflection-dump.cpp
  TOOLCHAIN_LINK_COMPONENTS object support
  LANGUAGE_COMPONENT tools
)
target_link_libraries(language-reflection-dump
                      PRIVATE
                        languageStaticMirror
                        languageRemoteInspection)
