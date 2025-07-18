add_subdirectory(Sources)

# If requested, build the regular expression parser into the compiler itself.
if(LANGUAGE_BUILD_REGEX_PARSER_IN_COMPILER)
  file(GLOB_RECURSE _COMPILER_REGEX_PARSER_SOURCES
    "${LANGUAGE_PATH_TO_STRING_PROCESSING_SOURCE}/Sources/_RegexParser/*.code")
  set(COMPILER_REGEX_PARSER_SOURCES)
  foreach(source ${_COMPILER_REGEX_PARSER_SOURCES})
    file(TO_CMAKE_PATH "${source}" source)
    list(APPEND COMPILER_REGEX_PARSER_SOURCES ${source})
  endforeach()
  message(STATUS "Using Experimental String Processing library for _CompilerRegexParser (${LANGUAGE_PATH_TO_STRING_PROCESSING_SOURCE}).")

  add_pure_language_host_library(_CompilerRegexParser STATIC
    "${COMPILER_REGEX_PARSER_SOURCES}"
  )
else()
  # Dummy target for dependencies
  add_custom_target(_CompilerRegexParser)
endif()
