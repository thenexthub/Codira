set(public_headers
  ${SOURCEKITD_SOURCE_DIR}/include/sourcekitd/sourcekitd.h
)

set(EXPORTED_SYMBOL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/sourcekitd.exports")

# FIXME: If you delete sourcekitd.framework, the soft links inside it need
# a CMake reconfigure in order for them to be created.

add_sourcekit_framework(sourcekitd
  ${public_headers}
  sourcekitd.cpp
  TOOLCHAIN_LINK_COMPONENTS support
  MODULEMAP module.modulemap
  INSTALL_IN_COMPONENT sourcekit-xpc-service
)
target_link_libraries(sourcekitd PRIVATE sourcekitdAPI)

if (SOURCEKIT_BUILT_STANDALONE)
  # Create the symlink necessary to find the language stdlib.
  language_create_post_build_symlink(sourcekitd
    IS_DIRECTORY
    SOURCE "${LANGUAGELIB_DIR}"
    DESTINATION "${SOURCEKIT_LIBRARY_OUTPUT_INTDIR}/language"
    COMMENT "Creating symlink necessary to find the language stdlib.")
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  if(LANGUAGE_COMPILER_VERSION)
    set_property(TARGET sourcekitd APPEND_STRING PROPERTY
                 LINK_FLAGS " -Wl,-current_version -Wl,${LANGUAGE_COMPILER_VERSION}")
  endif()
endif()

