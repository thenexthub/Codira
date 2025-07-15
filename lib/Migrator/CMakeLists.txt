set(datafiles
  macos4.json
  ios4.json
  tvos4.json
  watchos4.json
  overlay4.json
  macos42.json
  ios42.json
  tvos42.json
  watchos42.json
  overlay42.json
)
set(LANGUAGELIB_DIR
    "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib/language")
set(output_dir "${LANGUAGELIB_DIR}/migrator")

add_custom_command(
    OUTPUT "${output_dir}"
    COMMAND ${CMAKE_COMMAND} "-E" "make_directory" "${output_dir}")

set(outputs)

foreach(input ${datafiles})
  set(source "${CMAKE_CURRENT_SOURCE_DIR}/${input}")
  set(dest "${output_dir}/${input}")

  add_custom_command(OUTPUT
                       "${output_dir}/${input}"
                     DEPENDS
                       "${CMAKE_CURRENT_SOURCE_DIR}/${input}"
                     COMMAND
                       "${CMAKE_COMMAND}" "-E" "${LANGUAGE_COPY_OR_SYMLINK}" "${source}" "${dest}")
  list(APPEND outputs "${output_dir}/${input}")
endforeach()
list(APPEND outputs "${output_dir}")

add_custom_target("symlink_migrator_data"
    DEPENDS "${output_dir}" "${outputs}"
    COMMENT "Symlinking migrator data to ${output_dir}")

language_install_in_component(FILES ${datafiles}
                           DESTINATION "lib/language/migrator"
                           COMPONENT compiler)

add_language_host_library(languageMigrator STATIC
  APIDiffMigratorPass.cpp
  EditorAdapter.cpp
  FixitApplyDiagnosticConsumer.cpp
  Migrator.cpp
  MigrationState.cpp
  OptionalTryMigratorPass.cpp
  RewriteBufferEditsReceiver.cpp)
target_link_libraries(languageMigrator PRIVATE
  languageIDE)

add_dependencies(languageMigrator
  "symlink_migrator_data")

set_language_toolchain_is_available(languageMigrator)
