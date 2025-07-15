# Mark - copy "bridging" (support header) into the local include directory and
# install it into the compiler toolchain.

add_custom_command(
    OUTPUT "${LANGUAGE_INCLUDE_DIR}/language/bridging"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/language/bridging"
    COMMAND "${CMAKE_COMMAND}" "-E" "copy" "${CMAKE_CURRENT_SOURCE_DIR}/language/bridging" "${LANGUAGE_INCLUDE_DIR}/language")

add_custom_target("copy_cxxInterop_support_header"
    DEPENDS "${LANGUAGE_INCLUDE_DIR}/language/bridging"
    COMMENT "Copying C++ interop support header to ${LANGUAGE_INCLUDE_DIR}/language")

language_install_in_component(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/language/bridging"
    "${CMAKE_CURRENT_SOURCE_DIR}/language/bridging.modulemap"
    DESTINATION "include/language"
    COMPONENT compiler)
language_install_in_component(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/module.modulemap"
    DESTINATION "include"
    COMPONENT compiler)

add_dependencies(languageClangImporter
    "copy_cxxInterop_support_header")