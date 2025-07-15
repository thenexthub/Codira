set(libcxxshim_modulemap_target_list)
foreach(sdk ${LANGUAGE_SDKS})
  set(module_dir "${LANGUAGELIB_DIR}/${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}")
  set(module_dir_static "${LANGUAGESTATICLIB_DIR}/${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}")

  add_custom_command(OUTPUT ${module_dir}
                     COMMAND ${CMAKE_COMMAND} "-E" "make_directory" "${module_dir}")
  if(LANGUAGE_BUILD_STATIC_STDLIB OR LANGUAGE_SDK_${sdk}_STATIC_ONLY)
    add_custom_command(OUTPUT ${module_dir_static}
                       COMMAND ${CMAKE_COMMAND} "-E" "make_directory" "${module_dir_static}")
  endif()
  if(LANGUAGE_ENABLE_MACCATALYST AND "${sdk}" STREQUAL "OSX")
    set(maccatalyst_module_dir "${LANGUAGELIB_DIR}/${LANGUAGE_SDK_MACCATALYST_LIB_SUBDIR}")
    add_custom_command(OUTPUT ${maccatalyst_module_dir}
                       COMMAND ${CMAKE_COMMAND} "-E" "make_directory" "${maccatalyst_module_dir}")
  endif()

  set(outputs)
  foreach(source libcxxshim.modulemap libcxxshim.h libcxxstdlibshim.h)
    add_custom_command(OUTPUT ${module_dir}/${source}
                       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${source}
                       COMMAND ${CMAKE_COMMAND} "-E" "copy_if_different" "${CMAKE_CURRENT_SOURCE_DIR}/${source}" "${module_dir}/${source}"
                       COMMENT "Copying ${source} to ${module_dir}")
    list(APPEND outputs "${module_dir}/${source}")

    if(LANGUAGE_BUILD_STATIC_STDLIB OR LANGUAGE_SDK_${sdk}_STATIC_ONLY)
      add_custom_command(OUTPUT ${module_dir_static}/${source}
                         DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${source}
                         COMMAND ${CMAKE_COMMAND} "-E" "copy_if_different" "${CMAKE_CURRENT_SOURCE_DIR}/${source}" "${module_dir_static}/${source}"
                         COMMENT "Copying ${source} to ${module_dir_static}")
      list(APPEND outputs "${module_dir_static}/${source}")
    endif()
    if(LANGUAGE_ENABLE_MACCATALYST AND "${sdk}" STREQUAL "OSX")
      add_custom_command(OUTPUT ${maccatalyst_module_dir}/${source}
                         DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${source}
                         COMMAND ${CMAKE_COMMAND} "-E" "copy_if_different" "${CMAKE_CURRENT_SOURCE_DIR}/${source}" "${maccatalyst_module_dir}/${source}"
                         COMMENT "Copying ${source} to ${maccatalyst_module_dir}")
      list(APPEND outputs "${maccatalyst_module_dir}/${source}")
    endif()
  endforeach()
  list(APPEND outputs ${module_dir})
  if(LANGUAGE_BUILD_STATIC_STDLIB OR LANGUAGE_SDK_${sdk}_STATIC_ONLY)
    list(APPEND outputs ${module_dir_static})
  endif()
  if(LANGUAGE_ENABLE_MACCATALYST AND "${sdk}" STREQUAL "OSX")
    list(APPEND outputs ${maccatalyst_module_dir})
  endif()

  add_custom_target(cxxshim-${sdk} ALL
                    DEPENDS ${outputs}
                    COMMENT "Copying cxxshims to ${module_dir}")
  list(APPEND libcxxshim_modulemap_target_list cxxshim-${sdk})


  language_install_in_component(FILES libcxxshim.modulemap libcxxshim.h libcxxstdlibshim.h
                             DESTINATION "lib/language/${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}"
                             COMPONENT compiler)
  if(LANGUAGE_BUILD_STATIC_STDLIB OR LANGUAGE_SDK_${sdk}_STATIC_ONLY)
    language_install_in_component(FILES libcxxshim.modulemap libcxxshim.h libcxxstdlibshim.h
                               DESTINATION "lib/language_static/${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}"
                               COMPONENT compiler)
  endif()
  if(LANGUAGE_ENABLE_MACCATALYST AND "${sdk}" STREQUAL "OSX")
    language_install_in_component(FILES libcxxshim.modulemap libcxxshim.h libcxxstdlibshim.h
                               DESTINATION "lib/language/${LANGUAGE_SDK_MACCATALYST_LIB_SUBDIR}"
                               COMPONENT compiler)
  endif()
endforeach()

if(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB)
  set(module_dir "${LANGUAGELIB_DIR}/embedded")
  add_custom_command(OUTPUT ${module_dir}
                     COMMAND ${CMAKE_COMMAND} "-E" "make_directory" "${module_dir}")
  set(outputs)
  foreach(source libcxxshim.modulemap libcxxshim.h libcxxstdlibshim.h)
    add_custom_command(OUTPUT ${module_dir}/${source}
                       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${source}
                       COMMAND ${CMAKE_COMMAND} "-E" "copy_if_different" "${CMAKE_CURRENT_SOURCE_DIR}/${source}" "${module_dir}/${source}"
                       COMMENT "Copying ${source} to ${module_dir}")
    list(APPEND outputs "${module_dir}/${source}")
  endforeach()
  add_custom_target(cxxshim-embedded ALL
                    DEPENDS ${outputs}
                    COMMENT "Copying cxxshims to ${module_dir}")
  list(APPEND libcxxshim_modulemap_target_list cxxshim-embedded)
  language_install_in_component(FILES libcxxshim.modulemap libcxxshim.h libcxxstdlibshim.h
                             DESTINATION "lib/language/embedded"
                             COMPONENT compiler)
endif()

add_custom_target(libcxxshim_modulemap DEPENDS ${libcxxshim_modulemap_target_list})
set_property(TARGET libcxxshim_modulemap PROPERTY FOLDER "Miscellaneous")
add_dependencies(sdk-overlay libcxxshim_modulemap)
add_dependencies(compiler libcxxshim_modulemap)
