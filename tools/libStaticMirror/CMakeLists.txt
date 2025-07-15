# Use an 'internal' name, for now
set(LANGUAGE_SCAN_LIB_NAME "_InternalCodiraStaticMirror")

set(TOOLCHAIN_EXPORTED_SYMBOL_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/libStaticMirror.exports)

add_language_host_library(libStaticMirror SHARED
  libStaticMirror.cpp
  c-include-check.c
  TOOLCHAIN_LINK_COMPONENTS object support
)
if(NOT LANGUAGE_BUILT_STANDALONE AND NOT CMAKE_C_COMPILER_ID MATCHES Clang)
  add_dependencies(libStaticMirror clang)
endif()

add_dependencies(libStaticMirror
    languageStaticMirror)

target_link_libraries(libStaticMirror PRIVATE
    languageStaticMirror
    languageRemoteInspection)

set_target_properties(libStaticMirror
    PROPERTIES
    OUTPUT_NAME ${LANGUAGE_SCAN_LIB_NAME})

add_toolchain_symbol_exports(libStaticMirror ${TOOLCHAIN_EXPORTED_SYMBOL_FILE})

# Adds -dead_strip option
add_link_opts(libStaticMirror)

add_dependencies(static-mirror-lib libStaticMirror)
language_install_in_component(TARGETS libStaticMirror
  ARCHIVE DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}" COMPONENT static-mirror-lib
  LIBRARY DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}" COMPONENT static-mirror-lib
  RUNTIME DESTINATION "bin" COMPONENT static-mirror-lib)
language_install_in_component(DIRECTORY "${LANGUAGE_MAIN_INCLUDE_DIR}/language-c/StaticMirror/"
                           DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/${LANGUAGE_SCAN_LIB_NAME}"
                           COMPONENT static-mirror-lib)
