//===--- ScanningLoaders.h - Codira module scanning --------------*- C++ -*-===//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SCANNINGLOADERS_H
#define LANGUAGE_SCANNINGLOADERS_H

#include "language/AST/ASTContext.h"
#include "language/AST/ModuleDependencies.h"
#include "language/Frontend/ModuleInterfaceLoader.h"
#include "language/Serialization/SerializedModuleLoader.h"

namespace language {

/// Result of looking up a Codira module on the current filesystem
/// search paths.
struct CodiraModuleScannerQueryResult {
  struct IncompatibleCandidate {
    std::string path;
    std::string incompatibilityReason;
  };

  CodiraModuleScannerQueryResult()
      : foundDependencyInfo(std::nullopt), incompatibleCandidates() {}

  CodiraModuleScannerQueryResult(
      std::optional<ModuleDependencyInfo> &&dependencyInfo,
      std::vector<IncompatibleCandidate> &&candidates)
      : foundDependencyInfo(dependencyInfo),
        incompatibleCandidates(candidates) {}

  std::optional<ModuleDependencyInfo> foundDependencyInfo;
  std::vector<IncompatibleCandidate> incompatibleCandidates;
};

/// A module "loader" that looks for .codeinterface and .codemodule files
/// for the purpose of determining dependencies, but does not attempt to
/// load the module files.
class CodiraModuleScanner : public SerializedModuleLoaderBase {
private:
  /// Scan the given interface file to determine dependencies.
  toolchain::ErrorOr<ModuleDependencyInfo>
  scanInterfaceFile(Identifier moduleID, Twine moduleInterfacePath,
                    bool isFramework, bool isTestableImport);

  /// Scan the given serialized module file to determine dependencies.
  toolchain::ErrorOr<ModuleDependencyInfo>
  scanBinaryModuleFile(Identifier moduleID, Twine binaryModulePath,
                       bool isFramework, bool isTestableImport,
                       bool isCandidateForTextualModule);

  std::error_code findModuleFilesInDirectory(
      ImportPath::Element ModuleID, const SerializedModuleBaseName &BaseName,
      SmallVectorImpl<char> *ModuleInterfacePath,
      SmallVectorImpl<char> *ModuleInterfaceSourcePath,
      std::unique_ptr<toolchain::MemoryBuffer> *ModuleBuffer,
      std::unique_ptr<toolchain::MemoryBuffer> *ModuleDocBuffer,
      std::unique_ptr<toolchain::MemoryBuffer> *ModuleSourceInfoBuffer,
      bool SkipBuildingInterface, bool IsFramework,
      bool IsTestableDependencyLookup) override;

  virtual void collectVisibleTopLevelModuleNames(
      SmallVectorImpl<Identifier> &names) const override {
    toolchain_unreachable("Not used");
  }

  /// AST delegate to be used for textual interface scanning
  InterfaceSubContextDelegate &astDelegate;
  /// Location where pre-built modules are to be built into.
  std::string moduleOutputPath;
  /// Location where pre-built SDK modules are to be built into.
  std::string sdkModuleOutputPath;
  /// Clang-specific (-Xcc) command-line flags to include on
  /// Codira module compilation commands
  std::vector<std::string> languageModuleClangCC1CommandLineArgs;

  /// Constituents of a result of a given Codira module query,
  /// reset at the end of every query.
  std::optional<ModuleDependencyInfo> foundDependencyInfo;
  std::vector<CodiraModuleScannerQueryResult::IncompatibleCandidate>
      incompatibleCandidates;
public:
  CodiraModuleScanner(
      ASTContext &ctx, ModuleLoadingMode LoadMode,
      InterfaceSubContextDelegate &astDelegate, StringRef moduleOutputPath,
      StringRef sdkModuleOutputPath,
      std::vector<std::string> languageModuleClangCC1CommandLineArgs)
      : SerializedModuleLoaderBase(ctx, nullptr, LoadMode,
                                   /*IgnoreCodiraSourceInfoFile=*/true),
        astDelegate(astDelegate), moduleOutputPath(moduleOutputPath),
        sdkModuleOutputPath(sdkModuleOutputPath),
        languageModuleClangCC1CommandLineArgs(languageModuleClangCC1CommandLineArgs) {
  }

  /// Perform a filesystem search for a Codira module with a given name
  CodiraModuleScannerQueryResult lookupCodiraModule(Identifier moduleName,
                                                  bool isTestableImport);
};
} // namespace language

#endif // LANGUAGE_SCANNINGLOADERS_H
