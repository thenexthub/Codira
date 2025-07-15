add_language_host_library(languageSIL STATIC
  SIL.cpp)
target_link_libraries(languageSIL PUBLIC
  languageDemangling)
target_link_libraries(languageSIL PRIVATE
  languageAST
  languageClangImporter
  languageSema
  languageSerialization
  languageSILOptimizer)

add_subdirectory(IR)
add_subdirectory(Utils)
add_subdirectory(Verifier)
add_subdirectory(Parser)

# intrinsics_gen is the LLVM tablegen target that generates the include files
# where intrinsics and attributes are declared. languageSIL depends on these
# headers.
# For more information see the comment at the top of lib/CMakeLists.txt.
add_dependencies(languageSIL intrinsics_gen clang-tablegen-targets)

set_language_toolchain_is_available(languageSIL)
