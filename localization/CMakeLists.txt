set(diagnostic_witness "${CMAKE_BINARY_DIR}/share/language/diagnostics/generated")

add_custom_command(
  OUTPUT
    ${diagnostic_witness}
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/diagnostics/ ${CMAKE_BINARY_DIR}/share/language/diagnostics/
  COMMAND
    "${LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH}/language-def-to-strings-converter"
      --output-directory ${CMAKE_BINARY_DIR}/share/language/diagnostics/
  COMMAND
    "${LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH}/language-serialize-diagnostics"
      --input-file-path ${CMAKE_BINARY_DIR}/share/language/diagnostics/en.strings
      --output-directory ${CMAKE_BINARY_DIR}/share/language/diagnostics/
  COMMAND
    ${CMAKE_COMMAND} -E touch ${diagnostic_witness}
  DEPENDS
    language-def-to-strings-converter
    language-serialize-diagnostics
    # Add files in diagnostics subdirectory when they're created
)

add_custom_target(diagnostic-database DEPENDS ${diagnostic_witness})

add_dependencies(language-frontend diagnostic-database)

language_install_in_component(
  DIRECTORY ${CMAKE_BINARY_DIR}/share/language/diagnostics/
  DESTINATION "share/language/diagnostics"
  COMPONENT compiler
  FILES_MATCHING
  PATTERN "*.db"
  PATTERN "*.yaml"
  PATTERN "*.strings"
)
