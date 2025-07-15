
gyb_expand(tgmath.code.gyb tgmath.code)

add_library(languageAndroid
  tgmath.code
  Android.code
  Platform.code
  POSIXError.code
  TiocConstants.code)
set_target_properties(languageAndroid PROPERTIES
  Codira_MODULE_NAME Android)
target_compile_definitions(languageAndroid PRIVATE
  $<$<BOOL:${CodiraOverlay_ENABLE_REFLECTION}>:LANGUAGE_ENABLE_REFLECTION>)
target_link_libraries(languageAndroid PRIVATE
  CodiraAndroid
  languageCore)

install(TARGETS languageAndroid
  ARCHIVE DESTINATION "${CodiraOverlay_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CodiraOverlay_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
emit_language_interface(languageAndroid)
install_language_interface(languageAndroid)

embed_manifest(languageAndroid)
