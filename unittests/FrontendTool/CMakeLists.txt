add_language_unittest(CodiraFrontendToolTests
  FrontendToolTests.cpp
  ModuleLoadingTests.cpp)

target_link_libraries(CodiraFrontendToolTests
  PRIVATE
    languageFrontend
    languageFrontendTool)
