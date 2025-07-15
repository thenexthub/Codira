
# Install the generated language interface files for the target.
function(install_language_interface target)
  # Codiramodules are already in the directory structure
  get_target_property(module_name ${target} Codira_MODULE_NAME)
  if(NOT module_name)
    set(module_name ${target})
  endif()

  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${module_name}.codemodule"
    DESTINATION "${CodiraOverlay_INSTALL_LANGUAGEMODULEDIR}"
    COMPONENT CodiraOverlay_development)
endfunction()
