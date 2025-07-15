# Use an 'internal' name, this is primarily intended for CodiraDriver to import.
set(LANGUAGE_SCAN_LIB_NAME "_InternalCodiraScan")

set(TOOLCHAIN_EXPORTED_SYMBOL_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/libCodiraScan.exports)

add_language_host_library(libCodiraScan SHARED
  libCodiraScan.cpp
  c-include-check.c
  CodiraCaching.cpp
  HAS_LANGUAGE_MODULES)

if(LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS)
  # Workaround for a linker crash related to autolinking: rdar://77839981
  set_property(TARGET libCodiraScan APPEND_STRING PROPERTY
               LINK_FLAGS " -lobjc ")
endif()

add_dependencies(libCodiraScan
  clang
  languageDependencyScan)

target_link_libraries(libCodiraScan PRIVATE
    languageDependencyScan
    languageDriverTool
    languageStaticMirror
    languageRemoteInspection
    languageCompilerModules)

set_target_properties(libCodiraScan
    PROPERTIES
    OUTPUT_NAME ${LANGUAGE_SCAN_LIB_NAME})

if(LANGUAGE_HOST_VARIANT_SDK MATCHES "LINUX|ANDROID|OPENBSD|FREEBSD" AND BOOTSTRAPPING_MODE STREQUAL "HOSTTOOLS")
  # Runtime INSTALL_RPATH are set by 'add_language_host_library', but that expects
  # libCodiraScan be installed in 'lib'. But since it's actually installed in 'lib/language/host',
  # we need to have correct language runtime path to 'lib/language/{platform}'.
  # FIXME: BUILD_RPATH and INSTALL_PATH should be different
  # FIXME: add_language_host_library should accept 'DESTINATION' and handle installation
  # FIXME: Build this library into 'lib/language/host/' instead of 'lib/'
  set_property(
    TARGET libCodiraScan
    APPEND PROPERTY INSTALL_RPATH "$ORIGIN/../${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}"
  )
endif()

if(LANGUAGE_BUILD_LANGUAGE_SYNTAX)
  # Ensure that we can find the shared language-syntax libraries.
  if(LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS)
    set_property(
      TARGET libCodiraScan
      APPEND PROPERTY INSTALL_RPATH "@loader_path/language/host/compiler")
    set_property(
      TARGET libCodiraScan
      APPEND PROPERTY INSTALL_RPATH "@loader_path")
  elseif(LANGUAGE_HOST_VARIANT_SDK MATCHES "LINUX|ANDROID|OPENBSD|FREEBSD")
    set_property(
      TARGET libCodiraScan
      APPEND PROPERTY INSTALL_RPATH "$ORIGIN/language/host/compiler")
    set_property(
      TARGET libCodiraScan
      APPEND PROPERTY INSTALL_RPATH "$ORIGIN")
    # $ORIGIN can be the parent of the symbolic link inside language/host on Linux
    set_property(
      TARGET libCodiraScan
      APPEND PROPERTY INSTALL_RPATH "$ORIGIN/compiler")
  endif()
endif()

add_toolchain_symbol_exports(libCodiraScan ${TOOLCHAIN_EXPORTED_SYMBOL_FILE})

# Adds -dead_strip option
add_link_opts(libCodiraScan)

add_dependencies(compiler libCodiraScan)


if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
language_install_in_component(TARGETS libCodiraScan
  ARCHIVE DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}" COMPONENT compiler
  LIBRARY DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}" COMPONENT compiler
  RUNTIME DESTINATION "bin" COMPONENT compiler)
else()
  # On other platforms, instead install the library into 'lib/language/host/compiler' and symlink to it from its legacy location 'lib/language/host'
  language_install_in_component(TARGETS libCodiraScan
    ARCHIVE DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/host/compiler" COMPONENT compiler
    LIBRARY DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/host/compiler" COMPONENT compiler
    RUNTIME DESTINATION "bin" COMPONENT compiler)

  if(EXISTS ${TOOLCHAIN_CMAKE_DIR}/LLVMInstallSymlink.cmake)
    set(INSTALL_SYMLINK ${TOOLCHAIN_CMAKE_DIR}/LLVMInstallSymlink.cmake)
  endif()
  precondition(INSTALL_SYMLINK
               MESSAGE "LLVMInstallSymlink script must be available.")

  # 'lib/language/host' -> 'lib/language/host/compiler'
  set(FULL_LIB_NAME "lib${LANGUAGE_SCAN_LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  message(STATUS "Installing symlink (compiler/${FULL_LIB_NAME}) to lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/host/${FULL_LIB_NAME}")
  install(SCRIPT ${INSTALL_SYMLINK}
          CODE "install_symlink(${FULL_LIB_NAME}
                                compiler/${FULL_LIB_NAME}
                                lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/host
                                ${LANGUAGE_COPY_OR_SYMLINK})"
          COMPONENT compiler)
endif()

language_install_in_component(DIRECTORY "${LANGUAGE_MAIN_INCLUDE_DIR}/language-c/DependencyScan/"
                           DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/${LANGUAGE_SCAN_LIB_NAME}"
                           COMPONENT compiler)
