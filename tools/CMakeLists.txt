include(CheckIncludeFiles)
check_include_files("xpc/xpc.h" HAVE_XPC_H)

if(HAVE_XPC_H AND LANGUAGE_BUILD_SOURCEKIT)
  set(BUILD_SOURCEKIT_XPC_SERVICE_default TRUE)
else()
  set(BUILD_SOURCEKIT_XPC_SERVICE_default FALSE)
endif()

option(BUILD_SOURCEKIT_XPC_SERVICE
  "Whether or not the SourceKit XPC service should be built"
  ${BUILD_SOURCEKIT_XPC_SERVICE_default})

# Add generated SourceKit UID header to global dependencies.
if(LANGUAGE_BUILD_SOURCEKIT)
  list(APPEND TOOLCHAIN_COMMON_DEPENDS generated_sourcekit_uids)
endif()

add_language_tool_subdirectory(driver)
add_language_tool_subdirectory(language-compatibility-symbols)
add_language_tool_subdirectory(language-demangle)
add_language_tool_subdirectory(language-demangle-yamldump)
add_language_tool_subdirectory(language-def-to-strings-converter)
add_language_tool_subdirectory(language-serialize-diagnostics)
add_language_tool_subdirectory(language-ast-script)
add_language_tool_subdirectory(language-refactor)
add_language_tool_subdirectory(libStaticMirror)
add_language_tool_subdirectory(libMockPlugin)
add_language_tool_subdirectory(language-plugin-server)
add_language_tool_subdirectory(language-scan-test)

if(LANGUAGE_INCLUDE_TESTS OR LANGUAGE_INCLUDE_TEST_BINARIES)
  add_language_tool_subdirectory(language-ide-test)
  add_language_tool_subdirectory(language-remoteast-test)
  add_language_tool_subdirectory(lldb-moduleimport-test)
endif()

if(TOOLCHAIN_USE_SANITIZE_COVERAGE)
add_language_tool_subdirectory(language-demangle-fuzzer)
add_language_tool_subdirectory(language-reflection-fuzzer)
endif()

if(LANGUAGE_BUILD_SOURCEKIT)
  add_language_tool_subdirectory(SourceKit)
endif()

if(LANGUAGE_HOST_VARIANT STREQUAL "macosx")
  # Only build Darwin-specific tools when deploying to OS X.
  add_language_tool_subdirectory(language-stdlib-tool)
endif()

add_language_tool_subdirectory(language-reflection-dump)
