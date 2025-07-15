# Add flags for generating the zippered target variant in the build

# Initialize `CodiraCore_VARIANT_MODULE_TRIPLE` if the driver is able to emit
# modules for the target variant.

if(CodiraCore_COMPILER_VARIANT_TARGET)
  add_compile_options(
    "$<$<COMPILE_LANGUAGE:C,CXX>:SHELL:-darwin-target-variant ${CodiraCore_COMPILER_VARIANT_TARGET}>"
    "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-target-variant ${CodiraCore_COMPILER_VARIANT_TARGET}>"

    # TODO: Remove me once we have a driver with
    #   https://github.com/languagelang/language-driver/pull/1803
    "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xclang-linker -darwin-target-variant -Xclang-linker ${CodiraCore_COMPILER_VARIANT_TARGET}>")

  add_link_options(
    "$<$<LINK_LANGUAGE:C,CXX>:SHELL:-darwin-target-variant ${CodiraCore_COMPILER_VARIANT_TARGET}>"
    "$<$<LINK_LANGUAGE:Codira>:SHELL:-target-variant ${CodiraCore_COMPILER_VARIANT_TARGET}>"

    # TODO: Remove me once we have a driver with
    #   https://github.com/languagelang/language-driver/pull/1803
    "$<$<LINK_LANGUAGE:Codira>:SHELL:-Xclang-linker -darwin-target-variant -Xclang-linker ${CodiraCore_COMPILER_VARIANT_TARGET}>")

  # TODO: Once we are guaranteed to have a driver with the variant module path
  #       support everywhere, we should integrate this into PlatformInfo.cmake
  check_compiler_flag(Codira "-emit-variant-module-path ${CMAKE_CURRENT_BINARY_DIR}/CompilerID/variant.codemodule" HAVE_Codira_VARIANT_MODULE_PATH_FLAG)
  if(HAVE_Codira_VARIANT_MODULE_PATH_FLAG)
    # Get variant module triple
    set(module_triple_command "${CMAKE_Codira_COMPILER}" -print-target-info -target ${CodiraCore_COMPILER_VARIANT_TARGET})
    execute_process(COMMAND ${module_triple_command} OUTPUT_VARIABLE target_info_json)
    message(CONFIGURE_LOG "Codira target variant info: ${target_info_json}")


    string(JSON module_triple GET "${target_info_json}" "target" "moduleTriple")
    set(CodiraCore_VARIANT_MODULE_TRIPLE "${module_triple}" CACHE STRING "Triple used for installed language{module,interface} files for the target variant")
    mark_as_advanced(CodiraCore_VARIANT_MODULE_TRIPLE)
    message(CONFIGURE_LOG "Codira target variant module triple: ${module_triple}")

    string(JSON triple GET "${target_info_json}" "target" "triple")
    if(triple MATCHES "apple-([a-zA-Z]+)([0-9]+[.0-9]*)-macabi")
      set(CodiraCore_VARIANT_DEPLOYMENT_VERSION "${CMAKE_MATCH_2}")
      mark_as_advanced(CodiraCore_VARIANT_DEPLOYMENT_VERSION)
      message(CONFIGURE_LOG "Codira target variant deployment version: ${CodiraCore_VARIANT_DEPLOYMENT_VERSION}")
    endif()
  endif()
endif()
