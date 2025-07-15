# In the absence of fine grained tablegen dependencies we need to ensure that
# Codira's libraries all build after the LLVM & Clang tablegen-generated headers
# are generated. When building out-of-tree (as with build-script) LLVM & Clang's
# CMake configuration files create these targets as dummies so we can safely
# depend on them directly here.
# See https://github.com/apple/language/issues/48583.
#
# TOOLCHAIN_COMMON_DEPENDS is a construct from the LLVM build system. It is a special
# purpose variable that provides common dependencies for all libraries, and
# executables generated when it is set. CMake's scoping rules enforce that these
# new dependencies will only be added to targets created under Codira's lib
# directory.
list(APPEND TOOLCHAIN_COMMON_DEPENDS intrinsics_gen clang-tablegen-targets)

# Create a library that depends on all tool headers defined in include/module.modulemap
#
# TODO: generate this dynamically through the modulemap; this cannot use `sed`
# as that is not available on all platforms (e.g. Windows).
file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/HeaderDependencies.cpp"
  CONTENT "
#ifndef COMPILED_WITH_LANGUAGE
#define COMPILED_WITH_LANGUAGE
#endif

#include \"language/Basic/BasicBridging.h\"
#include \"language/AST/ASTBridging.h\"
#include \"language/IDE/IDEBridging.h\"
#include \"language/SIL/SILBridging.h\"
#include \"language/SILOptimizer/OptimizerBridging.h\"
")
add_library(importedHeaderDependencies "${CMAKE_CURRENT_BINARY_DIR}/HeaderDependencies.cpp")
# Respect TOOLCHAIN_COMMON_DEPENDS as well as `add_language_X` does to ensure
# all prerequisites like TableGen'ed headers are generated before.
add_dependencies(importedHeaderDependencies ${TOOLCHAIN_COMMON_DEPENDS})

add_subdirectory(APIDigester)
add_subdirectory(AST)
add_subdirectory(ASTGen)
add_subdirectory(ASTSectionImporter)
add_subdirectory(Basic)
add_subdirectory(CompilerCodiraSyntax)
add_subdirectory(ConstExtract)
add_subdirectory(ClangImporter)
add_subdirectory(Demangling)
add_subdirectory(DependencyScan)
add_subdirectory(Driver)
add_subdirectory(DriverTool)
add_subdirectory(Frontend)
add_subdirectory(FrontendTool)
add_subdirectory(Index)
add_subdirectory(IDE)
add_subdirectory(IDETool)
add_subdirectory(Immediate)
add_subdirectory(IRGen)
add_subdirectory(LLVMPasses)
add_subdirectory(Localization)
add_subdirectory(Macros)
add_subdirectory(Markup)
add_subdirectory(Migrator)
add_subdirectory(Option)
add_subdirectory(Parse)
add_subdirectory(PrintAsClang)
add_subdirectory(Refactoring)
add_subdirectory(RemoteAST)
add_subdirectory(Sema)
add_subdirectory(Serialization)
add_subdirectory(CodiraDemangle)
add_subdirectory(CodiraRemoteInspection)
add_subdirectory(CodiraRemoteMirror)
add_subdirectory(SIL)
add_subdirectory(SILGen)
add_subdirectory(SILOptimizer)
add_subdirectory(StaticMirror)
add_subdirectory(SymbolGraphGen)
add_subdirectory(CodiraSyntax)
add_subdirectory(Threading)
add_subdirectory(Tooling)
