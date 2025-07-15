add_language_host_library(languageRemoteMirror STATIC
  ${LANGUAGE_SOURCE_DIR}/stdlib/public/CodiraRemoteMirror/CodiraRemoteMirror.cpp)
target_link_libraries(languageRemoteMirror PRIVATE
  languageDemangling)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_definitions(languageRemoteMirror PRIVATE _LIB)
endif()
