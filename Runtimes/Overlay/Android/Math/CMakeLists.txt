
add_library(language_math
  Math.code)
set_target_properties(language_math PROPERTIES
  Codira_MODULE_NAME math)
target_link_libraries(language_math PRIVATE
  CodiraAndroid
  languageCore)

install(TARGETS language_math
  ARCHIVE DESTINATION "${CodiraOverlay_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CodiraOverlay_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
emit_language_interface(language_math)
install_language_interface(language_math)

embed_manifest(language_math)
