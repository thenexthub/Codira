set(SOURCEKITD_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sourcekitd)

include_directories(
  ${SOURCEKITD_SOURCE_DIR}/include
)

add_language_lib_subdirectory(sourcekitd)
if(LibEdit_FOUND AND LibEdit_HAS_UNICODE)
  add_language_tool_subdirectory(sourcekitd-repl)
endif()
if(LANGUAGE_INCLUDE_TESTS OR LANGUAGE_INCLUDE_TEST_BINARIES)
  add_language_tool_subdirectory(sourcekitd-test)
  add_language_tool_subdirectory(complete-test)
endif()
