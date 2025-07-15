add_language_host_library(languageOption STATIC
  Options.cpp
  SanitizerOptions.cpp)
add_dependencies(languageOption
  CodiraOptions)
target_link_libraries(languageOption PRIVATE
  languageBasic)

set(features_merger "${LANGUAGE_SOURCE_DIR}/utils/merge-features.py")
set(features_file_language_src "${CMAKE_CURRENT_SOURCE_DIR}/features.json")
set(features_file_clang_src "${TOOLCHAIN_BINARY_DIR}/share/clang/features.json")
set(features_file_dest "${CMAKE_BINARY_DIR}/share/language/features.json")

add_custom_command(
  OUTPUT
    ${features_file_dest}
  COMMAND
    $<TARGET_FILE:Python3::Interpreter> ${features_merger} -f ${features_file_language_src} -p \"\" -f ${features_file_clang_src} -p clang- > ${features_file_dest}
  DEPENDS
    ${features_merger}
    ${features_file_language_src}
    ${features_file_clang_src}
)

add_custom_target(language-features-file DEPENDS ${features_file_dest})

add_dependencies(languageOption language-features-file)

language_install_in_component(
  FILES
    ${features_file_dest}
  DESTINATION
    "share/language"
  COMPONENT
    compiler
)

