//===--- ModuleDependencies.h - Module Dependencies -------------*- C++ -*-===//
//
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
//
// This file defines data structures for capturing all of the source files
// and modules on which a given module depends, forming a graph of all of the
// modules that need to be present for a given module to be built.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_AST_MODULE_DEPENDENCIES_H
#define LANGUAGE_AST_MODULE_DEPENDENCIES_H

#include "language/AST/Import.h"
#include "language/AST/LinkLibrary.h"
#include "language/Basic/CXXStdlibKind.h"
#include "language/Basic/Toolchain.h"
#include "language/Serialization/Validation.h"
#include "clang/CAS/CASOptions.h"
#include "clang/Tooling/DependencyScanning/DependencyScanningService.h"
#include "clang/Tooling/DependencyScanning/ModuleDepCollector.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/DenseSet.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/StringSet.h"
#include "toolchain/CAS/CachingOnDiskFileSystem.h"
#include "toolchain/Support/Mutex.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace language {

class ClangModuleDependenciesCacheImpl;
class SourceFile;
class ASTContext;
class Identifier;
class CompilerInstance;
class IRGenOptions;
class CompilerInvocation;
class DiagnosticEngine;

/// Which kind of module dependencies we are looking for.
enum class ModuleDependencyKind : int8_t {
  FirstKind,
  // Textual Codira dependency
  CodiraInterface = FirstKind,
  // Binary module Codira dependency
  CodiraBinary,
  // Clang module dependency
  Clang,
  // Used to model the translation unit's source module
  CodiraSource,
  LastKind = CodiraSource + 1
};

/// This is used to idenfity a specific macro plugin dependency.
struct MacroPluginDependency {
  std::string LibraryPath;
  std::string ExecutablePath;
};

/// This is used to identify a specific module.
struct ModuleDependencyID {
  std::string ModuleName;
  ModuleDependencyKind Kind;
  bool operator==(const ModuleDependencyID &Other) const {
    return std::tie(ModuleName, Kind) == std::tie(Other.ModuleName, Other.Kind);
  }
  bool operator<(const ModuleDependencyID &Other) const {
    return std::tie(ModuleName, Kind) < std::tie(Other.ModuleName, Other.Kind);
  }
};

struct ModuleDependencyKindHash {
  std::size_t operator()(ModuleDependencyKind k) const {
    using UnderlyingType = std::underlying_type<ModuleDependencyKind>::type;
    return std::hash<UnderlyingType>{}(static_cast<UnderlyingType>(k));
  }
};
struct ModuleDependencyIDHash {
  std::size_t operator()(ModuleDependencyID id) const {
    return toolchain::hash_combine(id.ModuleName, id.Kind);
  }
};

using ModuleDependencyIDSet =
    std::unordered_set<ModuleDependencyID, ModuleDependencyIDHash>;
using ModuleDependencyIDSetVector =
    toolchain::SetVector<ModuleDependencyID, std::vector<ModuleDependencyID>,
                    std::set<ModuleDependencyID>>;

namespace dependencies {
std::string createEncodedModuleKindAndName(ModuleDependencyID id);
bool checkImportNotTautological(const ImportPath::Module, const SourceLoc,
                                const SourceFile &, bool);
void registerBackDeployLibraries(
    const IRGenOptions &IRGenOpts,
    std::function<void(const LinkLibrary &)> RegistrationCallback);
void registerCxxInteropLibraries(
    const toolchain::Triple &Target, StringRef mainModuleName, bool hasStaticCxx,
    bool hasStaticCxxStdlib, CXXStdlibKind cxxStdlibKind,
    std::function<void(const LinkLibrary &)> RegistrationCallback);
} // namespace dependencies

struct ScannerImportStatementInfo {
  struct ImportDiagnosticLocationInfo {
    ImportDiagnosticLocationInfo() = delete;
    ImportDiagnosticLocationInfo(std::string bufferIdentifier,
                                 uint32_t lineNumber, uint32_t columnNumber)
        : bufferIdentifier(bufferIdentifier), lineNumber(lineNumber),
          columnNumber(columnNumber) {}
    std::string bufferIdentifier;
    uint32_t lineNumber;
    uint32_t columnNumber;
  };

  ScannerImportStatementInfo(std::string importIdentifier)
      : importIdentifier(importIdentifier), importLocations(),
        isExported(false), accessLevel(AccessLevel::Public) {}

  ScannerImportStatementInfo(std::string importIdentifier, bool isExported,
                             AccessLevel accessLevel)
      : importIdentifier(importIdentifier), importLocations(),
        isExported(isExported), accessLevel(accessLevel) {}

  ScannerImportStatementInfo(std::string importIdentifier, bool isExported,
                             AccessLevel accessLevel,
                             ImportDiagnosticLocationInfo location)
      : importIdentifier(importIdentifier), importLocations({location}),
        isExported(isExported), accessLevel(accessLevel) {}

  ScannerImportStatementInfo(std::string importIdentifier, bool isExported,
                             AccessLevel accessLevel,
                             SmallVector<ImportDiagnosticLocationInfo, 4> locations)
      : importIdentifier(importIdentifier), importLocations(locations),
        isExported(isExported), accessLevel(accessLevel) {}

  void addImportLocation(ImportDiagnosticLocationInfo location) {
    importLocations.push_back(location);
  }

  /// Imported module string. e.g. "Foo.Bar" in 'import Foo.Bar'
  std::string importIdentifier;
  /// Buffer, line & column number of the import statement
  SmallVector<ImportDiagnosticLocationInfo, 4> importLocations;
  /// Is this an @_exported import
  bool isExported;
  /// Access level of this dependency
  AccessLevel accessLevel;
};

/// Base class for the variant storage of ModuleDependencyInfo.
///
/// This class is mostly an implementation detail for \c ModuleDependencyInfo.
class ModuleDependencyInfoStorageBase {
public:
  const ModuleDependencyKind dependencyKind;

  ModuleDependencyInfoStorageBase(
      ModuleDependencyKind dependencyKind,
      ArrayRef<ScannerImportStatementInfo> moduleImports,
      ArrayRef<ScannerImportStatementInfo> optionalModuleImports,
      ArrayRef<LinkLibrary> linkLibraries, StringRef moduleCacheKey = "")
      : dependencyKind(dependencyKind), moduleImports(moduleImports),
        optionalModuleImports(optionalModuleImports),
        linkLibraries(linkLibraries), moduleCacheKey(moduleCacheKey.str()),
        finalized(false) {}

  virtual ModuleDependencyInfoStorageBase *clone() const = 0;

  virtual ~ModuleDependencyInfoStorageBase();

  /// The set of modules on which this module depends.
  std::vector<ScannerImportStatementInfo> moduleImports;

  /// The set of modules which constitute optional module
  /// dependencies for this module, such as `@_implementationOnly`
  /// or `internal` imports.
  std::vector<ScannerImportStatementInfo> optionalModuleImports;

  /// A collection of libraries that must be linked to
  /// use this module.
  std::vector<LinkLibrary> linkLibraries;

  /// All directly-imported Codira module dependencies.
  std::vector<ModuleDependencyID> importedCodiraModules;
  /// All directly-imported Clang module dependencies.
  std::vector<ModuleDependencyID> importedClangModules;
  /// All cross-import overlay module dependencies.
  std::vector<ModuleDependencyID> crossImportOverlayModules;
  /// All dependencies comprised of Codira overlay modules of direct and
  /// transitive Clang dependencies.
  std::vector<ModuleDependencyID> languageOverlayDependencies;

  /// The cache key for the produced module.
  std::string moduleCacheKey;

  /// Auxiliary files that help to construct other dependencies (e.g.
  /// command-line), no need to be saved to reconstruct from cache.
  std::vector<std::string> auxiliaryFiles;

  /// The macro dependencies.
  std::map<std::string, MacroPluginDependency> macroDependencies;

  /// ModuleDependencyInfo is finalized (with all transitive dependencies
  /// and inputs).
  bool finalized;
};

struct CommonCodiraTextualModuleDependencyDetails {
  CommonCodiraTextualModuleDependencyDetails(
      ArrayRef<StringRef> buildCommandLine,
      StringRef CASFileSystemRootID)
      : bridgingHeaderFile(std::nullopt),
        bridgingSourceFiles(), bridgingModuleDependencies(),
        buildCommandLine(buildCommandLine.begin(), buildCommandLine.end()),
        CASFileSystemRootID(CASFileSystemRootID) {}

  /// Bridging header file, if there is one.
  std::optional<std::string> bridgingHeaderFile;

  /// Source files on which the bridging header depends.
  std::vector<std::string> bridgingSourceFiles;

  /// (Clang) modules on which the bridging header depends.
  std::vector<ModuleDependencyID> bridgingModuleDependencies;

  /// The Codira frontend invocation arguments to build the Codira module from the
  /// interface.
  std::vector<std::string> buildCommandLine;

  /// CASID for the Root of CASFS. Empty if CAS is not used.
  std::string CASFileSystemRootID;

  /// CASID for the Root of bridgingHeaderClangIncludeTree. Empty if not used.
  std::string CASBridgingHeaderIncludeTreeRootID;
};

/// Describes the dependencies of a Codira module described by an Codira interface
/// file.
///
/// This class is mostly an implementation detail for \c ModuleDependencyInfo.
class CodiraInterfaceModuleDependenciesStorage
    : public ModuleDependencyInfoStorageBase {
public:
  /// Destination output path
  std::string moduleOutputPath;

  /// The Codira interface file to be used to generate the module file.
  const std::string languageInterfaceFile;

  /// Potentially ready-to-use compiled modules for the interface file.
  const std::vector<std::string> compiledModuleCandidates;

  /// The hash value that will be used for the generated module
  std::string contextHash;

  /// A flag that indicates this dependency is a framework
  const bool isFramework;

  /// A flag that indicates this dependency is associated with a static archive
  const bool isStatic;

  /// Details common to Codira textual (interface or source) modules
  CommonCodiraTextualModuleDependencyDetails textualModuleDetails;

  /// The user module version of this textual module interface.
  const std::string userModuleVersion;

  CodiraInterfaceModuleDependenciesStorage(
      StringRef languageInterfaceFile,
      ArrayRef<StringRef> compiledModuleCandidates,
      ArrayRef<ScannerImportStatementInfo> moduleImports,
      ArrayRef<ScannerImportStatementInfo> optionalModuleImports,
      ArrayRef<StringRef> buildCommandLine, ArrayRef<LinkLibrary> linkLibraries,
      bool isFramework, bool isStatic, StringRef RootID,
      StringRef moduleCacheKey, StringRef userModuleVersion)
      : ModuleDependencyInfoStorageBase(ModuleDependencyKind::CodiraInterface,
                                        moduleImports, optionalModuleImports,
                                        linkLibraries, moduleCacheKey),
        languageInterfaceFile(languageInterfaceFile),
        compiledModuleCandidates(compiledModuleCandidates.begin(),
                                 compiledModuleCandidates.end()),
        isFramework(isFramework), isStatic(isStatic),
        textualModuleDetails(buildCommandLine, RootID),
        userModuleVersion(userModuleVersion) {}

  ModuleDependencyInfoStorageBase *clone() const override {
    return new CodiraInterfaceModuleDependenciesStorage(*this);
  }

  static bool classof(const ModuleDependencyInfoStorageBase *base) {
    return base->dependencyKind == ModuleDependencyKind::CodiraInterface;
  }

  void updateCommandLine(const std::vector<std::string> &newCommandLine) {
    textualModuleDetails.buildCommandLine = newCommandLine;
  }
};

/// Describes the dependencies of a Codira module
///
/// This class is mostly an implementation detail for \c ModuleDependencyInfo.
class CodiraSourceModuleDependenciesStorage
    : public ModuleDependencyInfoStorageBase {
public:
  /// Codira source files that are part of the Codira module, when known.
  std::vector<std::string> sourceFiles;

  /// Details common to Codira textual (interface or source) modules
  CommonCodiraTextualModuleDependencyDetails textualModuleDetails;

  /// Collection of module imports that were detected to be `@Testable`
  toolchain::StringSet<> testableImports;

  /// The Codira frontend invocation arguments to build bridging header.
  std::vector<std::string> bridgingHeaderBuildCommandLine;

  /// The chained bridging header path if used.
  std::string chainedBridgingHeaderPath;

  /// The chained bridging header source buffer if used.
  std::string chainedBridgingHeaderContent;

  CodiraSourceModuleDependenciesStorage(
      StringRef RootID, ArrayRef<StringRef> buildCommandLine,
      ArrayRef<ScannerImportStatementInfo> moduleImports,
      ArrayRef<ScannerImportStatementInfo> optionalModuleImports,
      ArrayRef<StringRef> bridgingHeaderBuildCommandLine)
      : ModuleDependencyInfoStorageBase(ModuleDependencyKind::CodiraSource,
                                        moduleImports, optionalModuleImports, {}),
        textualModuleDetails(buildCommandLine, RootID),
        testableImports(toolchain::StringSet<>()),
        bridgingHeaderBuildCommandLine(bridgingHeaderBuildCommandLine.begin(),
                                       bridgingHeaderBuildCommandLine.end()) {}

  ModuleDependencyInfoStorageBase *clone() const override {
    return new CodiraSourceModuleDependenciesStorage(*this);
  }


  static bool classof(const ModuleDependencyInfoStorageBase *base) {
    return base->dependencyKind == ModuleDependencyKind::CodiraSource;
  }

  void updateCommandLine(const std::vector<std::string> &newCommandLine) {
    textualModuleDetails.buildCommandLine = newCommandLine;
  }

  void updateBridgingHeaderCommandLine(
      const std::vector<std::string> &newCommandLine) {
    bridgingHeaderBuildCommandLine = newCommandLine;
  }

  void addTestableImport(ImportPath::Module module) {
    testableImports.insert(module.front().Item.str());
  }

  void setChainedBridgingHeaderBuffer(StringRef path, StringRef buffer) {
    chainedBridgingHeaderPath = path.str();
    chainedBridgingHeaderContent = buffer.str();
  }
};

/// Describes the dependencies of a pre-built Codira module (with no
/// .codeinterface).
///
/// This class is mostly an implementation detail for \c ModuleDependencyInfo.
class CodiraBinaryModuleDependencyStorage
    : public ModuleDependencyInfoStorageBase {
public:
  CodiraBinaryModuleDependencyStorage(
      StringRef compiledModulePath, StringRef moduleDocPath,
      StringRef sourceInfoPath,
      ArrayRef<ScannerImportStatementInfo> moduleImports,
      ArrayRef<ScannerImportStatementInfo> optionalModuleImports,
      ArrayRef<LinkLibrary> linkLibraries,
      ArrayRef<serialization::SearchPath> serializedSearchPaths,
      StringRef headerImport, StringRef definingModuleInterface,
      bool isFramework, bool isStatic, StringRef moduleCacheKey,
      StringRef userModuleVersion)
      : ModuleDependencyInfoStorageBase(ModuleDependencyKind::CodiraBinary,
                                        moduleImports, optionalModuleImports,
                                        linkLibraries, moduleCacheKey),
        compiledModulePath(compiledModulePath), moduleDocPath(moduleDocPath),
        sourceInfoPath(sourceInfoPath), headerImport(headerImport),
        definingModuleInterfacePath(definingModuleInterface),
        serializedSearchPaths(serializedSearchPaths),
        isFramework(isFramework), isStatic(isStatic),
        userModuleVersion(userModuleVersion) {}

  ModuleDependencyInfoStorageBase *clone() const override {
    return new CodiraBinaryModuleDependencyStorage(*this);
  }

  /// The path to the .codemodule file.
  const std::string compiledModulePath;

  /// The path to the .codeModuleDoc file.
  const std::string moduleDocPath;

  /// The path to the .codeSourceInfo file.
  const std::string sourceInfoPath;

  /// The path of the .h dependency of this module.
  const std::string headerImport;

  /// The path of the defining .codeinterface that this
  /// binary .codemodule was built from, if one exists.
  const std::string definingModuleInterfacePath;

  /// Source files on which the header inputs depend.
  std::vector<std::string> headerSourceFiles;

  /// Search paths this module was built with which got serialized
  std::vector<serialization::SearchPath> serializedSearchPaths;

  /// (Clang) modules on which the header inputs depend.
  std::vector<ModuleDependencyID> headerModuleDependencies;

  /// A flag that indicates this dependency is a framework
  const bool isFramework;

  /// A flag that indicates this dependency is associated with a static archive
  const bool isStatic;

  /// The user module version of this binary module.
  const std::string userModuleVersion;

  /// Return the path to the defining .codeinterface of this module
  /// of one was determined. Otherwise, return the .codemodule path
  /// itself.
  std::string getDefiningModulePath() const {
    if (definingModuleInterfacePath.empty())
      return compiledModulePath;
    return definingModuleInterfacePath;
  }

  static bool classof(const ModuleDependencyInfoStorageBase *base) {
    return base->dependencyKind == ModuleDependencyKind::CodiraBinary;
  }
};

/// Describes the dependencies of a Clang module.
///
/// This class is mostly an implementation detail for \c ModuleDependencyInfo.
class ClangModuleDependencyStorage : public ModuleDependencyInfoStorageBase {
public:
  /// Destination output path
  const std::string pcmOutputPath;

  /// Same as \c pcmOutputPath, but possibly prefix-mapped using clang's prefix
  /// mapper.
  const std::string mappedPCMPath;

  /// The module map file used to generate the Clang module.
  const std::string moduleMapFile;

  /// The context hash describing the configuration options for this module.
  const std::string contextHash;

  /// Partial (Clang) command line that can be used to build this module.
  std::vector<std::string> buildCommandLine;

  /// The file dependencies
  const std::vector<std::string> fileDependencies;

  /// CASID for the Root of CASFS. Empty if CAS is not used.
  std::string CASFileSystemRootID;

  /// CASID for the Root of ClangIncludeTree. Empty if not used.
  std::string CASClangIncludeTreeRootID;

  /// Whether this is a "system" module.
  bool IsSystem;

  ClangModuleDependencyStorage(StringRef pcmOutputPath, StringRef mappedPCMPath,
                               StringRef moduleMapFile, StringRef contextHash,
                               ArrayRef<std::string> buildCommandLine,
                               ArrayRef<std::string> fileDependencies,
                               ArrayRef<LinkLibrary> linkLibraries,
                               StringRef CASFileSystemRootID,
                               StringRef clangIncludeTreeRoot,
                               StringRef moduleCacheKey, bool IsSystem)
      : ModuleDependencyInfoStorageBase(ModuleDependencyKind::Clang,
                                        {}, {},
                                        linkLibraries, moduleCacheKey),
        pcmOutputPath(pcmOutputPath), mappedPCMPath(mappedPCMPath),
        moduleMapFile(moduleMapFile), contextHash(contextHash),
        buildCommandLine(buildCommandLine), fileDependencies(fileDependencies),
        CASFileSystemRootID(CASFileSystemRootID),
        CASClangIncludeTreeRootID(clangIncludeTreeRoot), IsSystem(IsSystem) {}

  ModuleDependencyInfoStorageBase *clone() const override {
    return new ClangModuleDependencyStorage(*this);
  }

  static bool classof(const ModuleDependencyInfoStorageBase *base) {
    return base->dependencyKind == ModuleDependencyKind::Clang;
  }

  void updateCommandLine(ArrayRef<std::string> newCommandLine) {
    buildCommandLine = newCommandLine;
  }
};

// MARK: Module Dependency Info
/// Describes the dependencies of a given module.
///
/// The dependencies of a module include all of the source files that go
/// into that module, as well as any modules that are directly imported
/// into the module.
class ModuleDependencyInfo {
private:
  std::unique_ptr<ModuleDependencyInfoStorageBase> storage;

  ModuleDependencyInfo(
      std::unique_ptr<ModuleDependencyInfoStorageBase> &&storage)
      : storage(std::move(storage)) {}

public:
  ModuleDependencyInfo() = default;
  ModuleDependencyInfo(const ModuleDependencyInfo &other)
      : storage(other.storage->clone()) {}
  ModuleDependencyInfo(ModuleDependencyInfo &&other) = default;

  ModuleDependencyInfo &operator=(const ModuleDependencyInfo &other) {
    storage.reset(other.storage->clone());
    return *this;
  }

  ModuleDependencyInfo &operator=(ModuleDependencyInfo &&other) = default;

  /// Describe the module dependencies for a Codira module that can be
  /// built from a Codira interface file (\c .codeinterface).
  static ModuleDependencyInfo forCodiraInterfaceModule(
      StringRef languageInterfaceFile, ArrayRef<StringRef> compiledCandidates,
      ArrayRef<StringRef> buildCommands,
      ArrayRef<ScannerImportStatementInfo> moduleImports,
      ArrayRef<ScannerImportStatementInfo> optionalModuleImports,
      ArrayRef<LinkLibrary> linkLibraries, bool isFramework, bool isStatic,
      StringRef CASFileSystemRootID, StringRef moduleCacheKey,
      StringRef userModuleVersion) {
    return ModuleDependencyInfo(
        std::make_unique<CodiraInterfaceModuleDependenciesStorage>(
            languageInterfaceFile, compiledCandidates, moduleImports,
            optionalModuleImports, buildCommands, linkLibraries, isFramework,
            isStatic, CASFileSystemRootID, moduleCacheKey, userModuleVersion));
  }

  /// Describe the module dependencies for a serialized or parsed Codira module.
  static ModuleDependencyInfo forCodiraBinaryModule(
      StringRef compiledModulePath, StringRef moduleDocPath,
      StringRef sourceInfoPath,
      ArrayRef<ScannerImportStatementInfo> moduleImports,
      ArrayRef<ScannerImportStatementInfo> optionalModuleImports,
      ArrayRef<LinkLibrary> linkLibraries,
      ArrayRef<serialization::SearchPath> serializedSearchPaths,
      StringRef headerImport, StringRef definingModuleInterface,
      bool isFramework, bool isStatic, StringRef moduleCacheKey,
      StringRef userModuleVer) {
    return ModuleDependencyInfo(
        std::make_unique<CodiraBinaryModuleDependencyStorage>(
            compiledModulePath, moduleDocPath, sourceInfoPath, moduleImports,
            optionalModuleImports, linkLibraries, serializedSearchPaths,
            headerImport, definingModuleInterface,isFramework, isStatic,
            moduleCacheKey, userModuleVer));
  }

  /// Describe the main Codira module.
  static ModuleDependencyInfo
  forCodiraSourceModule(const std::string &CASFileSystemRootID,
                       ArrayRef<StringRef> buildCommands,
                       ArrayRef<ScannerImportStatementInfo> moduleImports,
                       ArrayRef<ScannerImportStatementInfo> optionalModuleImports,
                       ArrayRef<StringRef> bridgingHeaderBuildCommands) {
    return ModuleDependencyInfo(
        std::make_unique<CodiraSourceModuleDependenciesStorage>(
            CASFileSystemRootID, buildCommands, moduleImports,
            optionalModuleImports, bridgingHeaderBuildCommands));
  }

  static ModuleDependencyInfo
  forCodiraSourceModule() {
    return ModuleDependencyInfo(
        std::make_unique<CodiraSourceModuleDependenciesStorage>(
             StringRef(), ArrayRef<StringRef>(),
             ArrayRef<ScannerImportStatementInfo>(),
             ArrayRef<ScannerImportStatementInfo>(),
             ArrayRef<StringRef>()));
  }

  /// Describe the module dependencies for a Clang module that can be
  /// built from a module map and headers.
  static ModuleDependencyInfo forClangModule(
      StringRef pcmOutputPath, StringRef mappedPCMPath, StringRef moduleMapFile,
      StringRef contextHash, ArrayRef<std::string> nonPathCommandLine,
      ArrayRef<std::string> fileDependencies,
      ArrayRef<LinkLibrary> linkLibraries, StringRef CASFileSystemRootID,
      StringRef clangIncludeTreeRoot, StringRef moduleCacheKey, bool IsSystem) {
    return ModuleDependencyInfo(std::make_unique<ClangModuleDependencyStorage>(
        pcmOutputPath, mappedPCMPath, moduleMapFile, contextHash,
        nonPathCommandLine, fileDependencies, linkLibraries,
        CASFileSystemRootID, clangIncludeTreeRoot, moduleCacheKey, IsSystem));
  }

  /// Retrieve the module-level imports.
  ArrayRef<ScannerImportStatementInfo> getModuleImports() const {
    return storage->moduleImports;
  }

  /// Retrieve the module-level optional imports.
  ArrayRef<ScannerImportStatementInfo> getOptionalModuleImports() const {
    return storage->optionalModuleImports;
  }

  std::string getModuleCacheKey() const {
    return storage->moduleCacheKey;
  }

  void updateModuleCacheKey(const std::string &key) {
    storage->moduleCacheKey = key;
  }

  void
  setImportedCodiraDependencies(const ArrayRef<ModuleDependencyID> dependencyIDs) {
    assert(isCodiraModule());
    storage->importedCodiraModules.assign(dependencyIDs.begin(),
                                         dependencyIDs.end());
  }
  ArrayRef<ModuleDependencyID> getImportedCodiraDependencies() const {
    return storage->importedCodiraModules;
  }

  void
  setImportedClangDependencies(const ArrayRef<ModuleDependencyID> dependencyIDs) {
    storage->importedClangModules.assign(dependencyIDs.begin(),
                                         dependencyIDs.end());
  }
  ArrayRef<ModuleDependencyID> getImportedClangDependencies() const {
    return storage->importedClangModules;
  }

  void
  setHeaderClangDependencies(const ArrayRef<ModuleDependencyID> dependencyIDs) {
    assert(isCodiraModule());
    switch (getKind()) {
    case language::ModuleDependencyKind::CodiraInterface: {
      auto languageInterfaceStorage =
          cast<CodiraInterfaceModuleDependenciesStorage>(storage.get());
      languageInterfaceStorage->textualModuleDetails.bridgingModuleDependencies.assign(dependencyIDs.begin(),
                                                                                    dependencyIDs.end());
      break;
    }
    case language::ModuleDependencyKind::CodiraSource: {
      auto languageSourceStorage =
          cast<CodiraSourceModuleDependenciesStorage>(storage.get());
      languageSourceStorage->textualModuleDetails.bridgingModuleDependencies.assign(dependencyIDs.begin(),
                                                                                 dependencyIDs.end());
      break;
    }
    case language::ModuleDependencyKind::CodiraBinary: {
      auto languageBinaryStorage =
          cast<CodiraBinaryModuleDependencyStorage>(storage.get());
      languageBinaryStorage->headerModuleDependencies.assign(dependencyIDs.begin(),
                                                          dependencyIDs.end());
      break;
    }
    default: {
      toolchain_unreachable("Unexpected dependency kind");
    }
    }
  }
  ArrayRef<ModuleDependencyID> getHeaderClangDependencies() const {
    switch (getKind()) {
    case language::ModuleDependencyKind::CodiraInterface: {
      auto languageInterfaceStorage =
          cast<CodiraInterfaceModuleDependenciesStorage>(storage.get());
      return languageInterfaceStorage->textualModuleDetails.bridgingModuleDependencies;
    }
    case language::ModuleDependencyKind::CodiraSource: {
      auto languageSourceStorage =
          cast<CodiraSourceModuleDependenciesStorage>(storage.get());
      return languageSourceStorage->textualModuleDetails.bridgingModuleDependencies;
    }
    case language::ModuleDependencyKind::CodiraBinary: {
      auto languageBinaryStorage =
          cast<CodiraBinaryModuleDependencyStorage>(storage.get());
      return languageBinaryStorage->headerModuleDependencies;
    }
    default:
      return {};
    }
  }

  void
  setCodiraOverlayDependencies(const ArrayRef<ModuleDependencyID> dependencyIDs) {
    assert(isCodiraModule());
    storage->languageOverlayDependencies.assign(dependencyIDs.begin(),
                                             dependencyIDs.end());
  }
  ArrayRef<ModuleDependencyID> getCodiraOverlayDependencies() const {
    return storage->languageOverlayDependencies;
  }

  void
  setCrossImportOverlayDependencies(const ArrayRef<ModuleDependencyID> dependencyIDs) {
    assert(isCodiraModule());
    storage->crossImportOverlayModules.assign(dependencyIDs.begin(),
                                              dependencyIDs.end());
  }
  ArrayRef<ModuleDependencyID> getCrossImportOverlayDependencies() const {
    return storage->crossImportOverlayModules;
  }

  ArrayRef<LinkLibrary> getLinkLibraries() const {
    return storage->linkLibraries;
  }

  void
  setLinkLibraries(const ArrayRef<LinkLibrary> linkLibraries) {
    storage->linkLibraries.assign(linkLibraries.begin(), linkLibraries.end());
  }

  ArrayRef<std::string> getAuxiliaryFiles() const {
    return storage->auxiliaryFiles;
  }

  bool isStaticLibrary() const {
    if (auto *detail = getAsCodiraInterfaceModule())
      return detail->isStatic;
    if (auto *detail = getAsCodiraBinaryModule())
      return detail->isStatic;
    return false;
  }

  ArrayRef<std::string> getHeaderInputSourceFiles() const {
    if (auto *detail = getAsCodiraInterfaceModule())
      return detail->textualModuleDetails.bridgingSourceFiles;
    if (auto *detail = getAsCodiraSourceModule())
      return detail->textualModuleDetails.bridgingSourceFiles;
    if (auto *detail = getAsCodiraBinaryModule())
      return detail->headerSourceFiles;
    return {};
  }

  ArrayRef<std::string> getCommandline() const {
    if (auto *detail = getAsClangModule())
      return detail->buildCommandLine;
    if (auto *detail = getAsCodiraInterfaceModule())
      return detail->textualModuleDetails.buildCommandLine;
    if (auto *detail = getAsCodiraSourceModule())
      return detail->textualModuleDetails.buildCommandLine;
    return {};
  }

  void updateCommandLine(const std::vector<std::string> &newCommandLine) {
    if (isCodiraInterfaceModule())
      return cast<CodiraInterfaceModuleDependenciesStorage>(storage.get())
          ->updateCommandLine(newCommandLine);
    if (isCodiraSourceModule())
      return cast<CodiraSourceModuleDependenciesStorage>(storage.get())
          ->updateCommandLine(newCommandLine);
    if (isClangModule())
      return cast<ClangModuleDependencyStorage>(storage.get())
          ->updateCommandLine(newCommandLine);
    toolchain_unreachable("Unexpected type");
  }

  ArrayRef<std::string> getBridgingHeaderCommandline() const {
    if (auto *detail = getAsCodiraSourceModule())
      return detail->bridgingHeaderBuildCommandLine;
    return {};
  }

  void updateBridgingHeaderCommandLine(
      const std::vector<std::string> &newCommandLine) {
    if (isCodiraSourceModule())
      return cast<CodiraSourceModuleDependenciesStorage>(storage.get())
          ->updateBridgingHeaderCommandLine(newCommandLine);
    toolchain_unreachable("Unexpected type");
  }

  void addAuxiliaryFile(const std::string &file) {
    storage->auxiliaryFiles.emplace_back(file);
  }

  void addMacroDependency(StringRef macroModuleName, StringRef libraryPath,
                          StringRef executablePath) {
    storage->macroDependencies.insert(
        {macroModuleName.str(), {libraryPath.str(), executablePath.str()}});
  }

  std::map<std::string, MacroPluginDependency> &getMacroDependencies() const {
    return storage->macroDependencies;
  }

  void updateCASFileSystemRootID(const std::string &rootID) {
    if (isCodiraInterfaceModule())
      cast<CodiraInterfaceModuleDependenciesStorage>(storage.get())
          ->textualModuleDetails.CASFileSystemRootID = rootID;
    else if (isCodiraSourceModule())
      cast<CodiraSourceModuleDependenciesStorage>(storage.get())
          ->textualModuleDetails.CASFileSystemRootID = rootID;
    else if (isClangModule())
      cast<ClangModuleDependencyStorage>(storage.get())->CASFileSystemRootID =
          rootID;
    else
      toolchain_unreachable("Unexpected type");
  }

  /// Whether explicit input paths of all the module dependencies
  /// have been specified on the command-line recipe for this module.
  bool isFinalized() const { return storage->finalized; }
  void setIsFinalized(bool isFinalized) { storage->finalized = isFinalized; }

  /// For a Source dependency, register a `Testable` import
  void addTestableImport(ImportPath::Module module);

  /// Whether or not a queried module name is a `@Testable` import dependency
  /// of this module. Can only return `true` for Codira source modules.
  bool isTestableImport(StringRef moduleName) const;

  /// Whether the dependencies are for a Codira module: either Textual, Source,
  /// or Binary
  bool isCodiraModule() const;

  /// Whether the dependencies are for a textual interface Codira module or a
  /// Source Codira module.
  bool isTextualCodiraModule() const;

  /// Whether the dependencies are for a textual Codira module.
  bool isCodiraInterfaceModule() const;

  /// Whether the dependencies are for a textual Codira module.
  bool isCodiraSourceModule() const;

  /// Whether the dependencies are for a binary Codira module.
  bool isCodiraBinaryModule() const;

  /// Whether the dependencies are for a Clang module.
  bool isClangModule() const;

  ModuleDependencyKind getKind() const { return storage->dependencyKind; }

  /// Retrieve the dependencies for a Codira textual-interface module.
  const CodiraInterfaceModuleDependenciesStorage *
  getAsCodiraInterfaceModule() const;

  /// Retrieve the dependencies for a Codira module.
  const CodiraSourceModuleDependenciesStorage *getAsCodiraSourceModule() const;

  /// Retrieve the dependencies for a binary Codira module.
  const CodiraBinaryModuleDependencyStorage *getAsCodiraBinaryModule() const;

  /// Retrieve the dependencies for a Clang module.
  const ClangModuleDependencyStorage *getAsClangModule() const;

  /// Get the path to the module-defining file:
  /// `CodiraInterface`: Textual Interface path
  /// `CodiraBinary`: Binary module path
  /// `Clang`: Module map path
  std::string getModuleDefiningPath() const;

  /// Add a dependency on the given module, if it was not already in the set.
  void
  addOptionalModuleImport(StringRef module, bool isExported,
                          AccessLevel accessLevel,
                          toolchain::StringSet<> *alreadyAddedModules = nullptr);

  /// Add all of the module imports in the given source
  /// file to the set of module imports.
  void addModuleImports(const SourceFile &sourceFile,
                        toolchain::StringSet<> &alreadyAddedModules,
                        const SourceManager *sourceManager);

  /// Add a dependency on the given module, if it was not already in the set.
  void addModuleImport(ImportPath::Module module, bool isExported,
                       AccessLevel accessLevel,
                       toolchain::StringSet<> *alreadyAddedModules = nullptr,
                       const SourceManager *sourceManager = nullptr,
                       SourceLoc sourceLocation = SourceLoc());

  /// Add a dependency on the given module, if it was not already in the set.
  void addModuleImport(StringRef module, bool isExported,
                       AccessLevel accessLevel,
                       toolchain::StringSet<> *alreadyAddedModules = nullptr,
                       const SourceManager *sourceManager = nullptr,
                       SourceLoc sourceLocation = SourceLoc());

  /// Get the bridging header.
  std::optional<std::string> getBridgingHeader() const;

  /// Get CAS Filesystem RootID.
  std::optional<std::string> getCASFSRootID() const;

  /// Get Clang Include Tree ID.
  std::optional<std::string> getClangIncludeTree() const;

  /// Get bridging header Include Tree ID.
  std::optional<std::string> getBridgingHeaderIncludeTree() const;

  /// Get module output path.
  std::string getModuleOutputPath() const;

  /// Add a bridging header to a Codira module's dependencies.
  void addBridgingHeader(StringRef bridgingHeader);

  /// Add source files
  void addSourceFile(StringRef sourceFile);

  /// Add source files that the header input depends on.
  void setHeaderSourceFiles(const std::vector<std::string> &sourceFiles);

  /// Add bridging header include tree.
  void addBridgingHeaderIncludeTree(StringRef ID);

  /// Set the chained bridging header buffer.
  void setChainedBridgingHeaderBuffer(StringRef path, StringRef buffer);

  /// Set the output path and the context hash.
  void setOutputPathAndHash(StringRef outputPath, StringRef hash);

  /// Collect a map from a secondary module name to a list of cross-import
  /// overlays, when this current module serves as the primary module.
  toolchain::StringMap<toolchain::SmallSetVector<Identifier, 4>>
  collectCrossImportOverlayNames(
      ASTContext &ctx, StringRef moduleName,
      std::set<std::pair<std::string, std::string>> &overlayFiles) const;
};

using ModuleDependencyVector =
    toolchain::SmallVector<std::pair<ModuleDependencyID, ModuleDependencyInfo>, 1>;
using ModuleNameToDependencyMap = toolchain::StringMap<ModuleDependencyInfo>;
using ModuleDependenciesKindMap =
    std::unordered_map<ModuleDependencyKind, ModuleNameToDependencyMap,
                       ModuleDependencyKindHash>;

// MARK: CodiraDependencyScanningService
/// A carrier of state shared among possibly multiple invocations of the
/// dependency scanner.
class CodiraDependencyScanningService {
  /// The CASOption created the Scanning Service if used.
  std::optional<clang::CASOptions> CASOpts;

  /// The persistent Clang dependency scanner service
  std::optional<clang::tooling::dependencies::DependencyScanningService>
      ClangScanningService;

  /// Shared state mutual-exclusivity lock
  mutable toolchain::sys::SmartMutex<true> ScanningServiceGlobalLock;

public:
  CodiraDependencyScanningService();
  CodiraDependencyScanningService(const CodiraDependencyScanningService &) =
      delete;
  CodiraDependencyScanningService &
  operator=(const CodiraDependencyScanningService &) = delete;
  virtual ~CodiraDependencyScanningService() {}

  /// Setup caching service.
  bool setupCachingDependencyScanningService(CompilerInstance &Instance);

private:
  /// Enforce clients not being allowed to query this cache directly, it must be
  /// wrapped in an instance of `ModuleDependenciesCache`.
  friend class ModuleDependenciesCache;
  friend class ModuleDependencyScanner;
  friend class ModuleDependencyScanningWorker;
};

// MARK: ModuleDependenciesCache
/// This "local" dependencies cache persists only for the duration of a given
/// scanning action, and maintains a store of references to all dependencies
/// found within the current scanning action.
class ModuleDependenciesCache {
private:
  /// Discovered dependencies
  ModuleDependenciesKindMap ModuleDependenciesMap;
  /// Set containing all of the Clang modules that have already been seen.
  toolchain::DenseSet<clang::tooling::dependencies::ModuleID> alreadySeenClangModules;
  /// Name of the module under scan
  std::string mainScanModuleName;
  /// The context hash of the current scanning invocation
  std::string scannerContextHash;
  /// The timestamp of the beginning of the scanning query action
  /// using this cache
  const toolchain::sys::TimePoint<> scanInitializationTime;

  /// Retrieve the dependencies map that corresponds to the given dependency
  /// kind.
  toolchain::StringMap<const ModuleDependencyInfo *> &
  getDependencyReferencesMap(ModuleDependencyKind kind);
  const toolchain::StringMap<const ModuleDependencyInfo *> &
  getDependencyReferencesMap(ModuleDependencyKind kind) const;

public:
  ModuleDependenciesCache(const std::string &mainScanModuleName,
                          const std::string &scanningContextHash);
  ModuleDependenciesCache(const ModuleDependenciesCache &) = delete;
  ModuleDependenciesCache &operator=(const ModuleDependenciesCache &) = delete;

public:
  /// Retrieve the dependencies map that corresponds to the given dependency
  /// kind.
  ModuleNameToDependencyMap &getDependenciesMap(ModuleDependencyKind kind);
  const ModuleNameToDependencyMap &getDependenciesMap(ModuleDependencyKind kind) const;

  /// Whether we have cached dependency information for the given module.
  bool hasDependency(const ModuleDependencyID &moduleID) const;
  /// Whether we have cached dependency information for the given module.
  bool hasDependency(StringRef moduleName,
                     std::optional<ModuleDependencyKind> kind) const;
  /// Whether we have cached dependency information for the given module Name.
  bool hasDependency(StringRef moduleName) const;
  /// Whether we have cached dependency information for the given Codira module.
  bool hasCodiraDependency(StringRef moduleName) const;

  const toolchain::DenseSet<clang::tooling::dependencies::ModuleID> &
  getAlreadySeenClangModules() const {
    return alreadySeenClangModules;
  }
  void addSeenClangModule(clang::tooling::dependencies::ModuleID newModule) {
    alreadySeenClangModules.insert(newModule);
  }

  /// Query all dependencies
  ModuleDependencyIDSetVector
  getAllDependencies(const ModuleDependencyID &moduleID) const;

  /// Query all Clang module dependencies.
  ModuleDependencyIDSetVector
  getClangDependencies(const ModuleDependencyID &moduleID) const;

  /// Query all directly-imported Codira dependencies
  toolchain::ArrayRef<ModuleDependencyID>
  getImportedCodiraDependencies(const ModuleDependencyID &moduleID) const;
  /// Query all directly-imported Clang dependencies
  toolchain::ArrayRef<ModuleDependencyID>
  getImportedClangDependencies(const ModuleDependencyID &moduleID) const;
  /// Query all Clang module dependencies of this module's imported (bridging) header
  toolchain::ArrayRef<ModuleDependencyID>
  getHeaderClangDependencies(const ModuleDependencyID &moduleID) const;
  /// Query Codira overlay dependencies
  toolchain::ArrayRef<ModuleDependencyID>
  getCodiraOverlayDependencies(const ModuleDependencyID &moduleID) const;
  /// Query all cross-import overlay dependencies
  toolchain::ArrayRef<ModuleDependencyID>
  getCrossImportOverlayDependencies(const ModuleDependencyID &moduleID) const;

  /// Look for module dependencies for a module with the given ID
  ///
  /// \returns the cached result, or \c None if there is no cached entry.
  std::optional<const ModuleDependencyInfo *>
  findDependency(const ModuleDependencyID moduleID) const;

  /// Look for module dependencies for a module with the given name
  ///
  /// \returns the cached result, or \c None if there is no cached entry.
  std::optional<const ModuleDependencyInfo *>
  findDependency(StringRef moduleName,
                 std::optional<ModuleDependencyKind> kind = std::nullopt) const;

  /// Look for Codira module dependencies for a module with the given name
  ///
  /// \returns the cached result, or \c None if there is no cached entry.
  std::optional<const ModuleDependencyInfo *>
  findCodiraDependency(StringRef moduleName) const;

  /// Look for known existing dependencies.
  ///
  /// \returns the cached result.
  const ModuleDependencyInfo &
  findKnownDependency(const ModuleDependencyID &moduleID) const;

  /// Record dependencies for the given module.
  void recordDependency(StringRef moduleName,
                        ModuleDependencyInfo dependencies);

  /// Record dependencies for the given module collection.
  void recordDependencies(ModuleDependencyVector moduleDependencies,
                          DiagnosticEngine &diags);

  /// Update stored dependencies for the given module.
  void updateDependency(ModuleDependencyID moduleID,
                        ModuleDependencyInfo dependencyInfo);
  
  /// Remove a given dependency info from the cache.
  void removeDependency(ModuleDependencyID moduleID);

  /// Resolve this module's set of directly-imported Codira module
  /// dependencies
  void
  setImportedCodiraDependencies(ModuleDependencyID moduleID,
                               const ArrayRef<ModuleDependencyID> dependencyIDs);
  /// Resolve this module's set of directly-imported Clang module
  /// dependencies
  void
  setImportedClangDependencies(ModuleDependencyID moduleID,
                               const ArrayRef<ModuleDependencyID> dependencyIDs);
  /// Resolve this module's set of Codira module dependencies
  /// that are Codira overlays of Clang module dependencies.
  void
  setCodiraOverlayDependencies(ModuleDependencyID moduleID,
                              const ArrayRef<ModuleDependencyID> dependencyIDs);
  /// Resolve this Codira module's imported (bridging) header's
  /// Clang module dependencies
  void
  setHeaderClangDependencies(ModuleDependencyID moduleID,
                             const ArrayRef<ModuleDependencyID> dependencyIDs);
  /// Resolve this module's cross-import overlay dependencies
  void
  setCrossImportOverlayDependencies(ModuleDependencyID moduleID,
                                    const ArrayRef<ModuleDependencyID> dependencyIDs);

  StringRef getMainModuleName() const { return mainScanModuleName; }

private:
  friend class ModuleDependenciesCacheDeserializer;
  friend class ModuleDependenciesCacheSerializer;
};
} // namespace language

namespace std {
template <>
struct hash<language::ModuleDependencyID> {
  std::size_t operator()(const language::ModuleDependencyID &id) const {
    return toolchain::hash_combine(id.ModuleName, id.Kind);
  }
};
} // namespace std

#endif /* LANGUAGE_AST_MODULE_DEPENDENCIES_H */
