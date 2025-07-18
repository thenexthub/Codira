add_subdirectory(tools)

find_program(SPHINX_EXECUTABLE
  NAMES sphinx-build
  HINTS $ENV{SPHINX_DIR}
  PATH_SUFFIXES bin
  DOC "Sphinx documentation generator")

SET(LANGUAGE_SPHINX_PAPER_SIZE "letter"
  CACHE STRING "Paper size for generated documentation")

SET(SPHINX_ARGS
  -D latex_elements.papersize=${LANGUAGE_SPHINX_PAPER_SIZE}
  -d ${CMAKE_BINARY_DIR}/doctrees)

if(SPHINX_EXECUTABLE)
  add_custom_target(docs_html ALL
    ${SPHINX_EXECUTABLE} ${SPHINX_ARGS} -b html
      . ${CMAKE_BINARY_DIR}/docs/html
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building HTML documentation")
else()
  message(WARNING "Unable to find sphinx-build program. Not building docs")
endif()

if (TOOLCHAIN_ENABLE_DOXYGEN)
if (DOXYGEN_FOUND)
  set(abs_srcdir ${CMAKE_CURRENT_SOURCE_DIR})
  set(abs_builddir ${CMAKE_CURRENT_BINARY_DIR})

  if (HAVE_DOT)
    set(DOT ${TOOLCHAIN_PATH_DOT})
  endif()

  if (TOOLCHAIN_DOXYGEN_EXTERNAL_SEARCH)
    set(enable_searchengine "YES")
    set(searchengine_url "${TOOLCHAIN_DOXYGEN_SEARCHENGINE_URL}")
    set(enable_server_based_search "YES")
    set(enable_external_search "YES")
    set(extra_search_mappings "${TOOLCHAIN_DOXYGEN_SEARCH_MAPPINGS}")
  else()
    set(enable_searchengine "NO")
    set(searchengine_url "")
    set(enable_server_based_search "NO")
    set(enable_external_search "NO")
    set(extra_search_mappings "")
  endif()

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxygen.cfg.in
    ${CMAKE_CURRENT_BINARY_DIR}/doxygen.cfg @ONLY)

  set(abs_top_srcdir)
  set(abs_top_builddir)
  set(DOT)
  set(enable_searchengine)
  set(searchengine_url)
  set(enable_server_based_search)
  set(enable_external_search)
  set(extra_search_mappings)

  add_custom_target(doxygen-language
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.cfg
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating language doxygen documentation." VERBATIM)

  if(TOOLCHAIN_BUILD_DOCS)
    add_dependencies(doxygen doxygen-language)
  endif()

  add_dependencies(dev doxygen-language)
  language_install_in_component(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen/html"
                             DESTINATION "docs/html"
                             COMPONENT dev)
endif()
endif()
