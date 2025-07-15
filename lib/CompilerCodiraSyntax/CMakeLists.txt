if(NOT LANGUAGE_BUILD_LANGUAGE_SYNTAX)
  return()
endif()
if(NOT EXISTS "${LANGUAGE_PATH_TO_LANGUAGE_SYNTAX_SOURCE}")
  message(SEND_ERROR "language-syntax is required to build the Codira compiler. Please run update-checkout or specify LANGUAGE_PATH_TO_LANGUAGE_SYNTAX_SOURCE")
  return()
endif()

# Build language-syntax libraries with FetchContent.
function(includeCodiraSyntax)
  set(CMAKE_SYSTEM_PROCESSOR ${LANGUAGE_HOST_VARIANT_ARCH})
  set(CMAKE_OSX_ARCHITECTURES ${LANGUAGE_HOST_VARIANT_ARCH})
  set(CMAKE_C_COMPILER_TARGET ${LANGUAGE_HOST_TRIPLE})
  set(CMAKE_CXX_COMPILER_TARGET ${LANGUAGE_HOST_TRIPLE})
  set(CMAKE_Codira_COMPILER_TARGET ${LANGUAGE_HOST_TRIPLE})
  set(BUILD_SHARED_LIBS ON)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LANGUAGE_HOST_LIBRARIES_DEST_DIR}/compiler")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${LANGUAGE_HOST_LIBRARIES_DEST_DIR}/compiler")
  if(LANGUAGE_HOST_VARIANT_SDK MATCHES "LINUX|ANDROID|OPENBSD|FREEBSD")
    set(LANGUAGE_HOST_LIBRARIES_RPATH "$ORIGIN;$ORIGIN/../../${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}")
  endif()

  # Add unique ABI prefix to language-syntax libraries so that compiler libraries (e.g. sourcekitdInProc)
  # can be used from tools that has its own language-syntax libraries as CodiraPM dependencies.
  set(LANGUAGE_MODULE_ABI_NAME_PREFIX "_Compiler")
  set(LANGUAGESYNTAX_TARGET_NAMESPACE "_Compiler")
  set(LANGUAGESYNTAX_EMIT_MODULE OFF)

  file(TO_CMAKE_PATH "${LANGUAGE_PATH_TO_LANGUAGE_SYNTAX_SOURCE}" language_syntax_path)
  FetchContent_Declare(CompilerCodiraSyntax SOURCE_DIR "${language_syntax_path}")
  FetchContent_MakeAvailable(CompilerCodiraSyntax)

  # FIXME: Use FetchContent_Declare's EXCLUDE_FROM_ALL after CMake 3.28
  FetchContent_GetProperties(CompilerCodiraSyntax BINARY_DIR binary_dir)
  set_property(DIRECTORY "${binary_dir}" PROPERTY EXCLUDE_FROM_ALL TRUE)
endfunction()
includeCodiraSyntax()

set(compiler_languagesyntax_libs
  _CompilerCodiraSyntax
  _CompilerCodiraIfConfig
  _CompilerCodiraLexicalLookup
  _CompilerCodiraOperators
  _CompilerCodiraSyntaxBuilder
  _CompilerCodiraParser
  _CompilerCodiraParserDiagnostics
  _CompilerCodiraCompilerPluginMessageHandling
  _CompilerCodiraSyntaxMacroExpansion
  _CompilerCodiraSyntaxMacros
  _CompilerCodiraBasicFormat
  _CompilerCodiraDiagnostics
  _CompilerCodiraIDEUtils
)

foreach(lib ${compiler_languagesyntax_libs})
  target_compile_options(${lib} PRIVATE "SHELL:-module-link-name ${lib}")
endforeach()

language_install_in_component(TARGETS ${compiler_languagesyntax_libs}
  ARCHIVE DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/host/compiler" COMPONENT compiler-language-syntax-lib
  LIBRARY DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/host/compiler" COMPONENT compiler-language-syntax-lib
  RUNTIME DESTINATION "bin" COMPONENT compiler-language-syntax-lib)
add_dependencies(compiler-language-syntax-lib ${compiler_languagesyntax_libs})
