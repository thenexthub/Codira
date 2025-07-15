set(LANGUAGE_GYB_FLAGS
    "-DCFDatabaseFile=${LANGUAGE_SOURCE_DIR}/lib/ClangImporter/CFDatabase.def")

add_gyb_target(generated_sorted_cf_database
    SortedCFDatabase.def.gyb
    DEPENDS "${LANGUAGE_SOURCE_DIR}/lib/ClangImporter/CFDatabase.def")

add_language_host_library(languageClangImporter STATIC
  CFTypeInfo.cpp
  ClangAdapter.cpp
  ClangClassTemplateNamePrinter.cpp
  ClangDerivedConformances.cpp
  ClangDiagnosticConsumer.cpp
  ClangImporter.cpp
  ClangImporterRequests.cpp
  ClangIncludePaths.cpp
  ClangModuleDependencyScanner.cpp
  ClangSourceBufferImporter.cpp
  CodiraDeclSynthesizer.cpp
  DWARFImporter.cpp
  ImportDecl.cpp
  ImportEnumInfo.cpp
  ImportMacro.cpp
  ImportName.cpp
  ImportType.cpp
  Serializability.cpp
  CodiraLookupTable.cpp
)
target_link_libraries(languageClangImporter PRIVATE
  languageAST
  languageParse
  clangTooling
  LLVMBitstreamReader)

target_link_libraries(languageClangImporter INTERFACE
  clangDependencyScanning)

# This property is only set by calls to clang_tablegen. It will not be set on
# standalone builds, so it can always be safely passed.
get_property(CLANG_TABLEGEN_TARGETS GLOBAL PROPERTY CLANG_TABLEGEN_TARGETS)
add_dependencies(languageClangImporter
  generated_sorted_cf_database
  ${CLANG_TABLEGEN_TARGETS})

set_language_toolchain_is_available(languageClangImporter)

if(LANGUAGE_ENABLE_CXX_INTEROP_LANGUAGE_BRIDGING_HEADER)
  add_subdirectory(CodiraBridging)
endif()
