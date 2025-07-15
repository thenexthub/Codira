add_language_unittest(languageScanTests
  ScanFixture.cpp
  Features.cpp
  PrintTarget.cpp
  ModuleDeps.cpp)

target_link_libraries(languageScanTests
  PRIVATE
    languageFrontend
    languageDependencyScan
    libCodiraScan)

target_compile_definitions(languageScanTests PRIVATE
  LANGUAGELIB_DIR=\"${LANGUAGELIB_DIR}\")
