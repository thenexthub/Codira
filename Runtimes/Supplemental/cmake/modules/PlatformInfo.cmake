include_guard(GLOBAL)

if(NOT ${PROJECT_NAME}_SIZEOF_POINTER)
  set(${PROJECT_NAME}_SIZEOF_POINTER "${CMAKE_SIZEOF_VOID_P}" CACHE STRING "Size of a pointer in bytes")
  message(CONFIGURE_LOG "Stdlib Pointer size: ${CMAKE_SIZEOF_VOID_P}")
  mark_as_advanced(${PROJECT_NAME}_SIZEOF_POINTER)
endif()

# TODO: This logic should migrate to CMake once CMake supports installing languagemodules
set(module_triple_command "${CMAKE_Codira_COMPILER}" -print-target-info)
if(CMAKE_Codira_COMPILER_TARGET)
  list(APPEND module_triple_command -target ${CMAKE_Codira_COMPILER_TARGET})
endif()
execute_process(COMMAND ${module_triple_command} OUTPUT_VARIABLE target_info_json)
message(CONFIGURE_LOG "Codira target info: ${module_triple_command}\n"
"${target_info_json}")

if(NOT ${PROJECT_NAME}_MODULE_TRIPLE)
  string(JSON module_triple GET "${target_info_json}" "target" "moduleTriple")
  set(${PROJECT_NAME}_MODULE_TRIPLE "${module_triple}" CACHE STRING "Triple used for installed language{doc,module,interface} files")
  mark_as_advanced(${PROJECT_NAME}_MODULE_TRIPLE)

  message(CONFIGURE_LOG "Codira module triple: ${module_triple}")
endif()

if(NOT ${PROJECT_NAME}_PLATFORM_SUBDIR)
  string(JSON platform GET "${target_info_json}" "target" "platform")
  set(${PROJECT_NAME}_PLATFORM_SUBDIR "${platform}" CACHE STRING "Platform name used for installed language{doc,module,interface} files")
  mark_as_advanced(${PROJECT_NAME}_PLATFORM_SUBDIR)

  message(CONFIGURE_LOG "Codira platform: ${platform}")
endif()

if(NOT ${PROJECT_NAME}_ARCH_SUBDIR)
  string(JSON arch GET "${target_info_json}" "target" "arch")
  set(${PROJECT_NAME}_ARCH_SUBDIR "${arch}" CACHE STRING "Architecture used for setting the architecture subdirectory")
  mark_as_advanced(${PROJECT_NAME}_ARCH_SUBDIR)

  message(CONFIGURE_LOG "Codira Arch: ${arch}")
endif()
