configure_file(${LANGUAGE_SOURCE_DIR}/stdlib/public/Cxx/libstdcxx/libstdcxx.modulemap
  ${CMAKE_CURRENT_BINARY_DIR}/libstdcxx.modulemap COPYONLY)

add_language_unittest(CodiraClangImporterTests
  ClangImporterTests.cpp
)

target_link_libraries(CodiraClangImporterTests
    PRIVATE
    languageClangImporter
    languageSema
    languageParse
    languageAST
)
