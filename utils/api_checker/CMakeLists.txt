set(framework "FrameworkABIBaseline")
language_install_in_component(PROGRAMS "language-api-checker.py"
                           DESTINATION "bin"
                           COMPONENT toolchain-tools)
language_install_in_component(DIRECTORY "sdk-module-lists"
                           DESTINATION "bin"
                           COMPONENT toolchain-tools)
language_install_in_component(DIRECTORY "${framework}"
                           DESTINATION "lib/language"
                           COMPONENT toolchain-tools)

# Add symlink of FrameworkABIBaseline to the build dir. This ensures we can
# find the baseline data from the same relative path as if we are running the
# checker from the toolchain.
set(SWIFTLIB_DIR "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib/language")
set(dest "${SWIFTLIB_DIR}/${framework}")
set(source "${CMAKE_CURRENT_SOURCE_DIR}/${framework}")

add_custom_command(OUTPUT "${dest}"
                   DEPENDS "${source}"
                   COMMAND "${CMAKE_COMMAND}" "-E" "${SWIFT_COPY_OR_SYMLINK}" "${source}" "${dest}"
                   COMMENT "Symlinking ABI checker baseline data to ${dest}")
add_custom_target("symlink_abi_checker_data" ALL
                  DEPENDS "${dest}"
                  COMMENT "Symlinking ABI checker baseline data to ${dest}")
if(TARGET language-frontend)
  add_dependencies(language-frontend symlink_abi_checker_data)
endif()
