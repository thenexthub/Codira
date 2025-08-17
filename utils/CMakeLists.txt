add_subdirectory(vim)
add_subdirectory(lldb)
add_subdirectory(api_checker)

language_install_in_component(FILES language-mode.el
                           DESTINATION "share/emacs/site-lisp"
                           COMPONENT editor-integration)

language_install_in_component(PROGRAMS language-api-dump.py
                           DESTINATION bin
                           COMPONENT tools)

# We install LLVM's FileCheck, if requested.
file(TO_CMAKE_PATH "${LLVM_BUILD_BINARY_DIR}/bin/FileCheck${CMAKE_EXECUTABLE_SUFFIX}" _SWIFT_UTILS_FILECHECK)
language_install_in_component(PROGRAMS "${_SWIFT_UTILS_FILECHECK}"
                           DESTINATION bin
                           COMPONENT toolchain-toolchain-dev-tools)
