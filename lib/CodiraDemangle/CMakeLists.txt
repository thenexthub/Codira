add_language_host_library(languageDemangle SHARED
  CodiraDemangle.cpp
  MangleHack.cpp)
target_compile_definitions(languageDemangle PRIVATE
  TOOLCHAIN_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1)
target_link_libraries(languageDemangle PRIVATE
  languageDemangling)

add_dependencies(compiler languageDemangle)
language_install_in_component(TARGETS languageDemangle
  RUNTIME
    DESTINATION "bin"
    COMPONENT compiler
  LIBRARY
    DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}"
    COMPONENT compiler
  ARCHIVE
    DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}"
    COMPONENT compiler)
