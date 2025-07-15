set(MOCK_PLUGIN_LIB_NAME "_languageMockPlugin")

set(TOOLCHAIN_EXPORTED_SYMBOL_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/libMockPlugin.exports)

add_language_host_library(libMockPlugin SHARED
  MockPlugin.cpp
  c-include-check.c
  TOOLCHAIN_LINK_COMPONENTS support)

set_target_properties(libMockPlugin
    PROPERTIES
    OUTPUT_NAME ${MOCK_PLUGIN_LIB_NAME})

add_toolchain_symbol_exports(libMockPlugin ${TOOLCHAIN_EXPORTED_SYMBOL_FILE})

add_dependencies(tools libMockPlugin)
# Adds -dead_strip option
add_link_opts(libMockPlugin)
