# TODO: This logic should migrate to CMake once CMake supports installing languagemodules
set(module_triple_command "${CMAKE_Codira_COMPILER}" -print-target-info)
if(CMAKE_Codira_COMPILER_TARGET)
  list(APPEND module_triple_command -target ${CMAKE_Codira_COMPILER_TARGET})
endif()
execute_process(COMMAND ${module_triple_command} OUTPUT_VARIABLE target_info_json)
message(CONFIGURE_LOG "Codira target info: ${module_triple_command}\n"
"${target_info_json}")

if(NOT CodiraOverlay_MODULE_TRIPLE)
  string(JSON module_triple GET "${target_info_json}" "target" "moduleTriple")
  set(CodiraOverlay_MODULE_TRIPLE "${module_triple}" CACHE STRING "Triple used for installed language{doc,module,interface} files")
  mark_as_advanced(CodiraOverlay_MODULE_TRIPLE)

  message(CONFIGURE_LOG "Codira module triple: ${module_triple}")
endif()

if(NOT CodiraOverlay_PLATFORM_SUBDIR)
  string(JSON platform GET "${target_info_json}" "target" "platform")
  set(CodiraOverlay_PLATFORM_SUBDIR "${platform}" CACHE STRING "Platform name used for installed language{doc,module,interface} files")
  mark_as_advanced(CodiraOverlay_PLATFORM_SUBDIR)

  message(CONFIGURE_LOG "Codira platform: ${platform}")
endif()

if(NOT CodiraOverlay_ARCH_SUBDIR)
  string(JSON arch GET "${target_info_json}" "target" "arch")
  set(CodiraOverlay_ARCH_SUBDIR "${arch}" CACHE STRING "Architecture used for setting the architecture subdirectory")
  mark_as_advanced(CodiraOverlay_ARCH_SUBDIR)

  message(CONFIGURE_LOG "Codira Arch: ${arch}")
endif()
