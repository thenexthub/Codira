add_language_host_tool(language-compatibility-symbols
  language-compatibility-symbols.cpp
  TOOLCHAIN_LINK_COMPONENTS support
  LANGUAGE_COMPONENT tools
  DOES_NOT_USE_LANGUAGE
)

if(LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH)
  set(syms_file "${CMAKE_BINARY_DIR}/share/language/compatibility-symbols")

  add_custom_command_target(copy_compat_target
    COMMAND
      "${LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH}/language-compatibility-symbols"
      --output-filename ${syms_file}
    OUTPUT
      ${syms_file}
    DEPENDS
      language-compatibility-symbols
  )

  add_dependencies(language-frontend "${copy_compat_target}")

  language_install_in_component(
    FILES
      ${syms_file}
    DESTINATION
      "share/language"
    COMPONENT
      compiler
  )
endif()
