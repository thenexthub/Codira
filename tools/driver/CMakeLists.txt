# The language-frontend tool


# Add additional libraries to which we need to link when the Codira Codira
# parser is built in.
function(add_language_parser_link_libraries target)
  if(LANGUAGE_BUILD_LANGUAGE_SYNTAX)
    target_link_libraries(${target}
                          PRIVATE languageCore)

    get_property(LANGUAGE_MACRO_PLUGINS GLOBAL PROPERTY LANGUAGE_MACRO_PLUGINS)
    foreach(macrolib ${LANGUAGE_MACRO_PLUGINS})
      add_dependencies(${target} ${macrolib})
    endforeach()
    add_dependencies(${target} CodiraInProcPluginServer)
  endif()
endfunction()

if(BOOTSTRAPPING_MODE MATCHES "BOOTSTRAPPING.*")

  # Bootstrapping - level 0
  # For more information on how bootstrapping works, see docs/CodiraInTheCompiler.md

  add_language_host_tool(language-frontend-bootstrapping0
    driver.cpp
    LANGUAGE_COMPONENT no_component
    HAS_LANGUAGE_MODULES
    BOOTSTRAPPING 0
    THINLTO_LD64_ADD_FLTO_CODEGEN_ONLY
  )
  target_link_libraries(language-frontend-bootstrapping0
                        PRIVATE
                          languageDriverTool
                          languageCompilerStub)

  add_language_parser_link_libraries(language-frontend-bootstrapping0)

  language_create_post_build_symlink(language-frontend-bootstrapping0
    SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
    DESTINATION "languagec${CMAKE_EXECUTABLE_SUFFIX}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bootstrapping0/${CMAKE_CFG_INTDIR}/bin")

  # Bootstrapping - level 1

  add_language_host_tool(language-frontend-bootstrapping1
    driver.cpp
    LANGUAGE_COMPONENT no_component
    HAS_LANGUAGE_MODULES
    BOOTSTRAPPING 1
    THINLTO_LD64_ADD_FLTO_CODEGEN_ONLY
  )
  target_link_libraries(language-frontend-bootstrapping1
                        PRIVATE
                          languageDriverTool
                          languageCompilerModules-bootstrapping1)

  add_language_parser_link_libraries(language-frontend-bootstrapping1)

  language_create_post_build_symlink(language-frontend-bootstrapping1
    SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
    DESTINATION "languagec${CMAKE_EXECUTABLE_SUFFIX}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bootstrapping1/${CMAKE_CFG_INTDIR}/bin")
endif()

add_language_host_tool(language-frontend
  driver.cpp
  LANGUAGE_COMPONENT compiler
  HAS_LANGUAGE_MODULES
)
target_link_libraries(language-frontend
                      PUBLIC
                        languageDriverTool
                        languageCompilerModules)

add_language_parser_link_libraries(language-frontend)

# Create a `language-driver` executable adjacent to the `language-frontend` executable
# to ensure that `languagec` forwards to the standalone driver when invoked.
language_create_early_driver_copies(language-frontend)

# If a `language-driver` executable adjacent to the `language-frontend` executable exists
# then the `language` and `languagec` symlinks should point to it by-default
if(EXISTS "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language-driver${CMAKE_EXECUTABLE_SUFFIX}" AND EXISTS "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language-help${CMAKE_EXECUTABLE_SUFFIX}")
  message(STATUS "Pointing 'language' and 'languagec' symlinks at 'language-driver'.")
  language_create_post_build_symlink(language-frontend
    SOURCE "language-driver${CMAKE_EXECUTABLE_SUFFIX}"
    DESTINATION "language${CMAKE_EXECUTABLE_SUFFIX}"
    WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")
  language_create_post_build_symlink(language-frontend
    SOURCE "language-driver${CMAKE_EXECUTABLE_SUFFIX}"
    DESTINATION "languagec${CMAKE_EXECUTABLE_SUFFIX}"
    WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")

  message(STATUS "Pointing 'language-legacy-driver' and 'languagec-legacy-driver' symlinks at 'language-frontend'.")
  language_create_post_build_symlink(language-frontend
    SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
    DESTINATION "language-legacy-driver${CMAKE_EXECUTABLE_SUFFIX}"
    WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")
  language_create_post_build_symlink(language-frontend
    SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
    DESTINATION "languagec-legacy-driver${CMAKE_EXECUTABLE_SUFFIX}"
    WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")
else()
  message(STATUS "Pointing 'language' and 'languagec' symlinks at 'language-frontend' - no early CodiraDriver build found.")
  language_create_post_build_symlink(language-frontend
    SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
    DESTINATION "language${CMAKE_EXECUTABLE_SUFFIX}"
    WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")

  language_create_post_build_symlink(language-frontend
    SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
    DESTINATION "languagec${CMAKE_EXECUTABLE_SUFFIX}"
    WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")
endif()

language_create_post_build_symlink(language-frontend
  SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
  DESTINATION "sil-opt${CMAKE_EXECUTABLE_SUFFIX}"
  WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")

language_create_post_build_symlink(language-frontend
  SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
  DESTINATION "sil-fn-extractor${CMAKE_EXECUTABLE_SUFFIX}"
  WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")

language_create_post_build_symlink(language-frontend
  SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
  DESTINATION "sil-nm${CMAKE_EXECUTABLE_SUFFIX}"
  WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")

language_create_post_build_symlink(language-frontend
  SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
  DESTINATION "sil-toolchain-gen${CMAKE_EXECUTABLE_SUFFIX}"
  WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")

language_create_post_build_symlink(language-frontend
  SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
  DESTINATION "sil-passpipeline-dumper${CMAKE_EXECUTABLE_SUFFIX}"
  WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")

language_create_post_build_symlink(language-frontend
  SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
  DESTINATION "language-dependency-tool${CMAKE_EXECUTABLE_SUFFIX}"
  WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")

language_create_post_build_symlink(language-frontend
  SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
  DESTINATION "language-toolchain-opt${CMAKE_EXECUTABLE_SUFFIX}"
  WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")

language_create_post_build_symlink(language-frontend
  SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
  DESTINATION "language-symbolgraph-extract${CMAKE_EXECUTABLE_SUFFIX}"
  WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")

language_create_post_build_symlink(language-frontend
  SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
  DESTINATION "language-autolink-extract${CMAKE_EXECUTABLE_SUFFIX}"
  WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")

language_create_post_build_symlink(language-frontend
  SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
  DESTINATION "language-api-digester${CMAKE_EXECUTABLE_SUFFIX}"
  WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")

language_create_post_build_symlink(language-frontend
  SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
  DESTINATION "language-cache-tool${CMAKE_EXECUTABLE_SUFFIX}"
  WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")

language_create_post_build_symlink(language-frontend
  SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
  DESTINATION "language-parse-test${CMAKE_EXECUTABLE_SUFFIX}"
  WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")

language_create_post_build_symlink(language-frontend
  SOURCE "language-frontend${CMAKE_EXECUTABLE_SUFFIX}"
  DESTINATION "language-synthesize-interface${CMAKE_EXECUTABLE_SUFFIX}"
  WORKING_DIRECTORY "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")

add_dependencies(compiler language-frontend)
language_install_in_component(FILES "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language${CMAKE_EXECUTABLE_SUFFIX}"
                           DESTINATION "bin"
                           COMPONENT compiler)
language_install_in_component(FILES "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/languagec${CMAKE_EXECUTABLE_SUFFIX}"
                           DESTINATION "bin"
                           COMPONENT compiler)
language_install_in_component(FILES "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language-symbolgraph-extract${CMAKE_EXECUTABLE_SUFFIX}"
                           DESTINATION "bin"
                           COMPONENT compiler)
language_install_in_component(FILES "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language-api-digester${CMAKE_EXECUTABLE_SUFFIX}"
                           DESTINATION "bin"
                           COMPONENT compiler)
language_install_in_component(FILES "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language-cache-tool${CMAKE_EXECUTABLE_SUFFIX}"
                           DESTINATION "bin"
                           COMPONENT compiler)
language_install_in_component(FILES "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language-synthesize-interface${CMAKE_EXECUTABLE_SUFFIX}"
                           DESTINATION "bin"
                           COMPONENT compiler)
add_dependencies(autolink-driver language-frontend)
language_install_in_component(FILES "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language-autolink-extract${CMAKE_EXECUTABLE_SUFFIX}"
                           DESTINATION "bin"
                           COMPONENT autolink-driver)

if(EXISTS "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language-driver${CMAKE_EXECUTABLE_SUFFIX}" AND EXISTS "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language-help${CMAKE_EXECUTABLE_SUFFIX}")
  language_install_in_component(PROGRAMS "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language-driver${CMAKE_EXECUTABLE_SUFFIX}"
                             DESTINATION "bin"
                             COMPONENT compiler)
  language_install_in_component(PROGRAMS "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language-help${CMAKE_EXECUTABLE_SUFFIX}"
                             DESTINATION "bin"
                             COMPONENT compiler)
  language_install_in_component(FILES "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/languagec-legacy-driver${CMAKE_EXECUTABLE_SUFFIX}"
                             DESTINATION "bin"
                             COMPONENT compiler)
  language_install_in_component(FILES "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language-legacy-driver${CMAKE_EXECUTABLE_SUFFIX}"
                             DESTINATION "bin"
                             COMPONENT compiler)
endif()
