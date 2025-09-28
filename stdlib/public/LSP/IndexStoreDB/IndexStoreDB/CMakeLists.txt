if(CMAKE_VERSION VERSION_LESS 3.16)
  if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    set(CMAKE_LINK_LIBRARY_FLAG "-l")
    set(CMAKE_LINK_LIBRARY_SUFFIX "")
  endif()
endif()

add_library(IndexStoreDB
  IndexDelegate.code
  IndexStoreDB.code
  IndexStoreDBError.code
  SymbolLocation.code
  SymbolOccurrence.code
  SymbolProperty.code
  SymbolRole.code
  Symbol.code)
set_target_properties(IndexStoreDB PROPERTIES
  Codira_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/language
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/language")
target_link_libraries(IndexStoreDB PRIVATE
  CIndexStoreDB)
if(NOT CMAKE_SYSTEM_NAME STREQUAL Darwin)
  target_link_libraries(IndexStoreDB PRIVATE
    Foundation)
endif()

install(TARGETS IndexStoreDB
  ARCHIVE DESTINATION lib/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>/$<LOWER_CASE:${CMAKE_SYSTEM_NAME}>
  LIBRARY DESTINATION lib/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>/$<LOWER_CASE:${CMAKE_SYSTEM_NAME}>
  RUNTIME DESTINATION bin)
install_languagemodule(IndexStoreDB)
set_property(GLOBAL APPEND PROPERTY IndexStoreDB_EXPORTS IndexStoreDB)
