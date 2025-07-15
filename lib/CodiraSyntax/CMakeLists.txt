if(NOT LANGUAGE_BUILD_LANGUAGE_SYNTAX)
  return()
endif()
if(NOT EXISTS "${LANGUAGE_PATH_TO_LANGUAGE_SYNTAX_SOURCE}")
  message(SEND_ERROR "language-syntax is required to build the Codira compiler. Please run update-checkout or specify LANGUAGE_PATH_TO_LANGUAGE_SYNTAX_SOURCE")
  return()
endif()

# Build language-syntax libraries with FetchContent.
set(CMAKE_SYSTEM_PROCESSOR ${LANGUAGE_HOST_VARIANT_ARCH})
set(CMAKE_OSX_ARCHITECTURES ${LANGUAGE_HOST_VARIANT_ARCH})
set(CMAKE_C_COMPILER_TARGET ${LANGUAGE_HOST_TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${LANGUAGE_HOST_TRIPLE})
set(CMAKE_Codira_COMPILER_TARGET ${LANGUAGE_HOST_TRIPLE})
set(BUILD_SHARED_LIBS ON)
# All libraries in 'language-syntax' goes to 'lib/language/host'.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LANGUAGE_HOST_LIBRARIES_DEST_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${LANGUAGE_HOST_LIBRARIES_DEST_DIR}")
if(LANGUAGE_HOST_VARIANT_SDK MATCHES "LINUX|ANDROID|OPENBSD|FREEBSD")
  set(LANGUAGE_HOST_LIBRARIES_RPATH "$ORIGIN;$ORIGIN/../${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}")
endif()
# Add unique ABI prefix to language-syntax libraries so that compiler libraries (e.g. sourcekitdInProc)
# can be used from tools that has its own language-syntax libraries as CodiraPM dependencies.
set(LANGUAGE_MODULE_ABI_NAME_PREFIX "Compiler")
set(LANGUAGESYNTAX_PACKAGE_NAME "Toolchain")
set(LANGUAGESYNTAX_EMIT_MODULE ON)

file(TO_CMAKE_PATH "${LANGUAGE_PATH_TO_LANGUAGE_SYNTAX_SOURCE}" language_syntax_path)
FetchContent_Declare(CodiraSyntax SOURCE_DIR "${language_syntax_path}")
FetchContent_MakeAvailable(CodiraSyntax)

# FIXME: Use FetchContent_Declare's EXCLUDE_FROM_ALL after CMake 3.28
FetchContent_GetProperties(CodiraSyntax BINARY_DIR binary_dir)
set_property(DIRECTORY "${binary_dir}" PROPERTY EXCLUDE_FROM_ALL TRUE)

# Install language-syntax libraries.
set(LANGUAGE_SYNTAX_MODULES
  CodiraBasicFormat
  CodiraIfConfig
  CodiraParser
  CodiraParserDiagnostics
  CodiraDiagnostics
  CodiraSyntax
  CodiraOperators
  CodiraSyntaxBuilder
  CodiraSyntaxMacros
  CodiraSyntaxMacroExpansion
  CodiraCompilerPluginMessageHandling
  # Support for LSP
  CodiraIDEUtils
  CodiraRefactor
  # For language-plugin-server
  CodiraLibraryPluginProvider
)

# Install shared runtime libraries
if(CMAKE_SYSTEM_NAME MATCHES Windows)
  language_install_in_component(TARGETS ${LANGUAGE_SYNTAX_MODULES}
                             RUNTIME
                             DESTINATION "bin"
                             COMPONENT language-syntax-lib)
else()
  language_install_in_component(TARGETS ${LANGUAGE_SYNTAX_MODULES}
                             LIBRARY
                             DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/host"
                             COMPONENT language-syntax-lib)
endif()

# Install import libraries in Windows.
if(CMAKE_SYSTEM_NAME MATCHES Windows)
  list(TRANSFORM LANGUAGE_SYNTAX_MODULES
       PREPEND "${CMAKE_IMPORT_LIBRARY_PREFIX}"
       OUTPUT_VARIABLE LANGUAGE_SYNTAX_IMPORT_LIBRARIES)
  list(TRANSFORM LANGUAGE_SYNTAX_IMPORT_LIBRARIES
       APPEND "${CMAKE_IMPORT_LIBRARY_SUFFIX}"
       OUTPUT_VARIABLE LANGUAGE_SYNTAX_IMPORT_LIBRARIES)
  list(TRANSFORM LANGUAGE_SYNTAX_IMPORT_LIBRARIES
       PREPEND "${LANGUAGE_HOST_LIBRARIES_DEST_DIR}/"
       OUTPUT_VARIABLE LANGUAGE_SYNTAX_IMPORT_LIBRARIES)
  language_install_in_component(PROGRAMS "${LANGUAGE_SYNTAX_IMPORT_LIBRARIES}"
                             DESTINATION lib
                             COMPONENT language-syntax-lib)
endif()

add_dependencies(language-syntax-lib
  ${LANGUAGE_SYNTAX_MODULES})

if(BOOTSTRAPPING_MODE STREQUAL "CROSSCOMPILE")
    add_dependencies(language-syntax-lib language-stdlib-${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}-${LANGUAGE_HOST_VARIANT_ARCH})
endif()

# Install Codira module interface files.
foreach(module ${LANGUAGE_SYNTAX_MODULES})
  set(module_dir "${module}.codemodule")
  set(module_file "${LANGUAGE_HOST_LIBRARIES_DEST_DIR}/${module_dir}/${LANGUAGE_HOST_MODULE_TRIPLE}")
  language_install_in_component(FILES "${module_file}.codeinterface" "${module_file}.private.codeinterface"
                             DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/host/${module_dir}"
                             COMPONENT language-syntax-lib)
endforeach()

export(TARGETS ${LANGUAGE_SYNTAX_MODULES}
       NAMESPACE CodiraSyntax::
       FILE ${CMAKE_BINARY_DIR}/cmake/modules/CodiraSyntaxConfig.cmake
       EXPORT_LINK_INTERFACE_LIBRARIES)
