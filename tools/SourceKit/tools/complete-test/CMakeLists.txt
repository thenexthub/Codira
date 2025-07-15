add_sourcekit_executable(complete-test
  complete-test.cpp
  TOOLCHAIN_LINK_COMPONENTS option coverage lto
)
if(LANGUAGE_SOURCEKIT_USE_INPROC_LIBRARY)
  target_link_libraries(complete-test PRIVATE sourcekitdInProc)
else()
  target_link_libraries(complete-test PRIVATE sourcekitd)
endif()
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_libraries(complete-test PRIVATE
    dispatch
    BlocksRuntime)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set_target_properties(complete-test PROPERTIES
    INSTALL_RPATH "@executable_path/../lib")
  target_link_options(complete-test PRIVATE
    "LINKER:-exported_symbol,_main")
endif()
if(LANGUAGE_ANALYZE_CODE_COVERAGE)
  set_property(TARGET complete-test APPEND_STRING PROPERTY
    LINK_FLAGS " -fprofile-instr-generate -fcoverage-mapping")
endif()

add_dependencies(tools complete-test)
language_install_in_component(TARGETS complete-test
  RUNTIME DESTINATION bin COMPONENT tools)
