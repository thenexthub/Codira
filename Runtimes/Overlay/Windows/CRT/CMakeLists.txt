
gyb_expand(tgmath.code.gyb tgmath.code)

add_library(languageCRT
  tgmath.code
  ucrt.code
  Platform.code
  POSIXError.code
  TiocConstants.code)
set_target_properties(languageCRT PROPERTIES
  Codira_MODULE_NAME CRT)
target_compile_definitions(languageCRT PRIVATE
  $<$<BOOL:${CodiraOverlay_ENABLE_REFLECTION}>:LANGUAGE_ENABLE_REFLECTION>)
target_compile_options(languageCRT PRIVATE
  "SHELL:-Xcc -D_USE_MATH_DEFINES")
target_link_libraries(languageCRT PRIVATE
  ClangModules
  languageCore)

install(TARGETS languageCRT
  ARCHIVE DESTINATION "${CodiraOverlay_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CodiraOverlay_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
emit_language_interface(languageCRT)
install_language_interface(languageCRT)

embed_manifest(languageCRT)
