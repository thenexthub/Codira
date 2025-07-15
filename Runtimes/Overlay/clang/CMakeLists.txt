
gyb_expand(float.code.gyb float.code)

add_library(language_Builtin_float
  float.code
  "${PROJECT_SOURCE_DIR}/linker-support/magic-symbols-for-install-name.c")
set_target_properties(language_Builtin_float PROPERTIES
  Codira_MODULE_NAME _Builtin_float)
target_compile_options(language_Builtin_float PRIVATE
  "$<$<PLATFORM_ID:Darwin>:SHELL:-Xfrontend -module-abi-name -Xfrontend Darwin>")
target_link_libraries(language_Builtin_float PRIVATE
  $<$<PLATFORM_ID:Windows>:ClangModules>
  languageCore)

install(TARGETS language_Builtin_float
  ARCHIVE DESTINATION "${CodiraOverlay_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CodiraOverlay_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
emit_language_interface(language_Builtin_float)
install_language_interface(language_Builtin_float)

embed_manifest(language_Builtin_float)
