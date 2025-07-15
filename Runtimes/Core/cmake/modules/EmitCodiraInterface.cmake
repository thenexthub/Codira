# Generate and install language interface files

# TODO: CMake should learn how to model library evolution and generate this
#       stuff automatically.


# Generate a language interface file for the target if library evolution is enabled
function(emit_language_interface target)
  # Generate the target-variant binary language module when performing zippered
  # build
  # Clean this up once CMake has nested languagemodules in the build directory:
  # https://gitlab.kitware.com/cmake/cmake/-/merge_requests/10664
  # https://cmake.org/cmake/help/git-stage/policy/CMP0195.html

  # We can't expand the Codira_MODULE_NAME target property in a generator
  # expression or it will fail saying that the target doesn't exist.
  get_target_property(module_name ${target} Codira_MODULE_NAME)
  if(NOT module_name)
    set(module_name ${target})
  endif()
  # Account for an existing languagemodule file
  # generated with the previous logic
  if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${module_name}.codemodule"
     AND NOT IS_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${module_name}.codemodule")
    message(STATUS "Removing regular file ${CMAKE_CURRENT_BINARY_DIR}/${module_name}.codemodule to support nested languagemodule generation")
    file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/${module_name}.codemodule")
  endif()
  target_compile_options(${target} PRIVATE
    "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-emit-module-path ${CMAKE_CURRENT_BINARY_DIR}/${module_name}.codemodule/${CodiraCore_MODULE_TRIPLE}.codemodule>")
  if(CodiraCore_VARIANT_MODULE_TRIPLE)
    target_compile_options(${target} PRIVATE
      "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-emit-variant-module-path ${CMAKE_CURRENT_BINARY_DIR}/${module_name}.codemodule/${CodiraCore_VARIANT_MODULE_TRIPLE}.codemodule>")
  endif()
  add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${module_name}.codemodule/${CodiraCore_MODULE_TRIPLE}.codemodule"
    DEPENDS ${target})
  target_sources(${target}
    INTERFACE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/${module_name}.codemodule/${CodiraCore_MODULE_TRIPLE}.codemodule>)

  # Generate textual language interfaces is library-evolution is enabled
  if(CodiraCore_ENABLE_LIBRARY_EVOLUTION)
    target_compile_options(${target} PRIVATE
      $<$<COMPILE_LANGUAGE:Codira>:-emit-module-interface-path$<SEMICOLON>${CMAKE_CURRENT_BINARY_DIR}/${module_name}.codemodule/${CodiraCore_MODULE_TRIPLE}.codeinterface>
      $<$<COMPILE_LANGUAGE:Codira>:-emit-private-module-interface-path$<SEMICOLON>${CMAKE_CURRENT_BINARY_DIR}/${module_name}.codemodule/${CodiraCore_MODULE_TRIPLE}.private.codeinterface>)
    if(CodiraCore_VARIANT_MODULE_TRIPLE)
      target_compile_options(${target} PRIVATE
        "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-emit-variant-module-interface-path ${CMAKE_CURRENT_BINARY_DIR}/${module_name}.codemodule/${CodiraCore_VARIANT_MODULE_TRIPLE}.codeinterface>"
        "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-emit-variant-private-module-interface-path ${CMAKE_CURRENT_BINARY_DIR}/${module_name}.codemodule/${CodiraCore_VARIANT_MODULE_TRIPLE}.private.codeinterface>")
    endif()
    target_compile_options(${target} PRIVATE
      $<$<COMPILE_LANGUAGE:Codira>:-library-level$<SEMICOLON>api>
      $<$<COMPILE_LANGUAGE:Codira>:-Xfrontend$<SEMICOLON>-require-explicit-availability=ignore>)
  endif()
endfunction()
