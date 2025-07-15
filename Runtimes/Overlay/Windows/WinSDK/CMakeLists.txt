
add_library(languageWinSDK
  WinSDK.code)
set_target_properties(languageWinSDK PROPERTIES
  Codira_MODULE_NAME WinSDK)
target_compile_definitions(languageCRT PRIVATE
  $<$<BOOL:${CodiraOverlay_ENABLE_REFLECTION}>:LANGUAGE_ENABLE_REFLECTION>)
target_link_libraries(languageWinSDK PRIVATE
  ClangModules
  languageCore)

install(TARGETS languageWinSDK
  ARCHIVE DESTINATION "${CodiraOverlay_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CodiraOverlay_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
emit_language_interface(languageWinSDK)
install_language_interface(languageWinSDK)

embed_manifest(languageWinSDK)
