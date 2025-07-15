//===--- ModuleDependencyScanner.h - Import Codira modules ------*- C++ -*-===//
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

#include "language/AST/ASTContext.h"
#include "language/AST/Identifier.h"
#include "language/AST/ModuleDependencies.h"
#include "language/Frontend/ModuleInterfaceLoader.h"
#include "language/Serialization/ScanningLoaders.h"
#include "clang/Tooling/DependencyScanning/DependencyScanningTool.h"
#include "toolchain/CAS/CASReference.h"
#include "toolchain/Support/ThreadPool.h"

namespace language {
class DependencyTracker;
}

namespace language {

/// A dependency scanning worker which performs filesystem lookup
/// of a named module dependency.
class ModuleDependencyScanningWorker {
public:
  ModuleDependencyScanningWorker(
      CodiraDependencyScanningService &globalScanningService,
      const CompilerInvocation &ScanCompilerInvocation,
      const SILOptions &SILOptions, ASTContext &ScanASTContext,
      DependencyTracker &DependencyTracker,
      std::shared_ptr<toolchain::cas::ObjectStore> CAS,
      std::shared_ptr<toolchain::cas::ActionCache> ActionCache,
      toolchain::PrefixMapper *mapper, DiagnosticEngine &diags);

private:
  /// Retrieve the module dependencies for the Clang module with the given name.
  ModuleDependencyVector scanFilesystemForClangModuleDependency(
      Identifier moduleName,
      const toolchain::DenseSet<clang::tooling::dependencies::ModuleID>
          &alreadySeenModules);

  /// Retrieve the module dependencies for the Codira module with the given name.
  CodiraModuleScannerQueryResult scanFilesystemForCodiraModuleDependency(
      Identifier moduleName, bool isTestableImport = false);

  /// Query dependency information for header dependencies
  /// of a binary Codira module.
  ///
  /// \param moduleID the name of the Codira module whose dependency
  /// information will be augmented with information about the given
  /// textual header inputs.
  ///
  /// \param headerPath the path to the header to be scanned.
  ///
  /// \param clangScanningTool The clang dependency scanner.
  ///
  /// \param cache The module dependencies cache to update, with information
  /// about new Clang modules discovered along the way.
  ///
  /// \returns \c true if an error occurred, \c false otherwise
  bool scanHeaderDependenciesOfCodiraModule(
      const ASTContext &ctx,
      ModuleDependencyID moduleID, std::optional<StringRef> headerPath,
      std::optional<toolchain::MemoryBufferRef> sourceBuffer,
      ModuleDependenciesCache &cache,
      ModuleDependencyIDSetVector &headerClangModuleDependencies,
      std::vector<std::string> &headerFileInputs,
      std::vector<std::string> &bridgingHeaderCommandLine,
      std::optional<std::string> &includeTreeID);


  /// Store cache entry for include tree.
  toolchain::Error
  createCacheKeyForEmbeddedHeader(std::string embeddedHeaderIncludeTree,
                                  std::string chainedHeaderIncludeTree);

  // Worker-specific instance of CompilerInvocation
  std::unique_ptr<CompilerInvocation> workerCompilerInvocation;
  // Worker-specific instance of ASTContext
  std::unique_ptr<ASTContext> workerASTContext;
  // An AST delegate for interface scanning.
  std::unique_ptr<InterfaceSubContextDelegateImpl> scanningASTDelegate;
  // The Clang scanner tool used by this worker.
  clang::tooling::dependencies::DependencyScanningTool clangScanningTool;
  // Codira and Clang module loaders acting as scanners.
  std::unique_ptr<CodiraModuleScanner> languageModuleScannerLoader;

  /// The location of where the explicitly-built modules will be output to
  std::string moduleOutputPath;
  /// The location of where the explicitly-built SDK modules will be output to
  std::string sdkModuleOutputPath;

  // CAS instance.
  std::shared_ptr<toolchain::cas::ObjectStore> CAS;
  std::shared_ptr<toolchain::cas::ActionCache> ActionCache;
  /// File prefix mapper.
  toolchain::PrefixMapper *PrefixMapper;

  // Base command line invocation for clang scanner queries (both module and header)
  std::vector<std::string> clangScanningBaseCommandLineArgs;
  // Command line invocation for clang by-name module lookups
  std::vector<std::string> clangScanningModuleCommandLineArgs;
  // Clang-specific (-Xcc) command-line flags to include on
  // Codira module compilation commands
  std::vector<std::string> languageModuleClangCC1CommandLineArgs;
  // Working directory for clang module lookup queries
  std::string clangScanningWorkingDirectoryPath;
  // Restrict access to the parent scanner class.
  friend class ModuleDependencyScanner;
};

// MARK: CodiraDependencyTracker
/// Track language dependency
class CodiraDependencyTracker {
public:
  CodiraDependencyTracker(std::shared_ptr<toolchain::cas::ObjectStore> CAS,
                         toolchain::PrefixMapper *Mapper,
                         const CompilerInvocation &CI);
  
  void startTracking(bool includeCommonDeps = true);
  void trackFile(const Twine &path);
  toolchain::Expected<toolchain::cas::ObjectProxy> createTreeFromDependencies();
  
private:
  toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS;
  std::shared_ptr<toolchain::cas::ObjectStore> CAS;
  toolchain::PrefixMapper *Mapper;
  
  struct FileEntry {
    toolchain::cas::ObjectRef FileRef;
    size_t Size;
    
    FileEntry(toolchain::cas::ObjectRef FileRef, size_t Size)
    : FileRef(FileRef), Size(Size) {}
  };
  toolchain::StringMap<FileEntry> CommonFiles;
  std::map<std::string, FileEntry> TrackedFiles;
};

class ModuleDependencyIssueReporter {
private:
  ModuleDependencyIssueReporter(DiagnosticEngine &Diagnostics)
      : Diagnostics(Diagnostics) {}

  /// Diagnose scanner failure and attempt to reconstruct the dependency
  /// path from the main module to the missing dependency
  void diagnoseModuleNotFoundFailure(
      const ScannerImportStatementInfo &moduleImport,
      const ModuleDependenciesCache &cache,
      std::optional<ModuleDependencyID> dependencyOf,
      std::optional<std::pair<ModuleDependencyID, std::string>>
          resolvingSerializedSearchPath,
      std::optional<
          std::vector<CodiraModuleScannerQueryResult::IncompatibleCandidate>>
          foundIncompatibleCandidates = std::nullopt);

  /// Upon query failure, if incompatible binary module
  /// candidates were found, emit a failure diagnostic
  void diagnoseFailureOnOnlyIncompatibleCandidates(
      const ScannerImportStatementInfo &moduleImport,
      const std::vector<CodiraModuleScannerQueryResult::IncompatibleCandidate>
          &candidates,
      const ModuleDependenciesCache &cache,
      std::optional<ModuleDependencyID> dependencyOf);

  /// Emit warnings for each discovered binary Codira module
  /// which was incompatible with the current compilation
  /// when querying \c moduleName
  void warnOnIncompatibleCandidates(
      StringRef moduleName,
      const std::vector<CodiraModuleScannerQueryResult::IncompatibleCandidate>
          &candidates);

  DiagnosticEngine &Diagnostics;
  std::unordered_set<std::string> ReportedMissing;
  // Restrict access to the parent scanner class.
  friend class ModuleDependencyScanner;
};

class ModuleDependencyScanner {
public:
  ModuleDependencyScanner(CodiraDependencyScanningService &ScanningService,
                          const CompilerInvocation &ScanCompilerInvocation,
                          const SILOptions &SILOptions,
                          ASTContext &ScanASTContext,
                          DependencyTracker &DependencyTracker,
                          std::shared_ptr<toolchain::cas::ObjectStore> CAS,
                          std::shared_ptr<toolchain::cas::ActionCache> ActionCache,
                          DiagnosticEngine &diags, bool ParallelScan);

  /// Identify the scanner invocation's main module's dependencies
  toolchain::ErrorOr<ModuleDependencyInfo>
  getMainModuleDependencyInfo(ModuleDecl *mainModule);

  /// Resolve module dependencies of the given module, computing a full
  /// transitive closure dependency graph.
  std::vector<ModuleDependencyID>
  performDependencyScan(ModuleDependencyID rootModuleID,
                        ModuleDependenciesCache &cache);

  /// How many filesystem lookups were performed by the scanner
  unsigned getNumLookups() { return NumLookups; }

  /// CAS Dependency Tracker.
  std::optional<CodiraDependencyTracker>
  createCodiraDependencyTracker(const CompilerInvocation &CI) {
    if (!CAS)
      return std::nullopt;

    return CodiraDependencyTracker(CAS, PrefixMapper.get(), CI);
  }

  /// PrefixMapper for scanner.
  bool hasPathMapping() const {
    return PrefixMapper && !PrefixMapper->getMappings().empty();
  }
  toolchain::PrefixMapper *getPrefixMapper() const { return PrefixMapper.get(); }
  std::string remapPath(StringRef Path) const {
    if (!PrefixMapper)
      return Path.str();
    return PrefixMapper->mapToString(Path);
  }

  /// CAS options.
  toolchain::cas::ObjectStore &getCAS() const {
    assert(CAS && "Expect CAS available");
    return *CAS;
  }

  toolchain::vfs::FileSystem &getSharedCachingFS() const {
    assert(CacheFS && "Expect CacheFS available");
    return *CacheFS;
  }

private:
  /// Main routine that computes imported module dependency transitive
  /// closure for the given module.
  /// 1. Codira modules imported directly or via another Codira dependency
  /// 2. Clang modules imported directly or via a Codira dependency
  /// 3. Clang modules imported via textual header inputs to Codira modules
  /// (bridging headers)
  /// 4. Codira overlay modules of all of the transitively imported Clang modules
  /// that have one
  ModuleDependencyIDSetVector
  resolveImportedModuleDependencies(const ModuleDependencyID &rootModuleID,
                                    ModuleDependenciesCache &cache);
  void resolveCodiraModuleDependencies(
      const ModuleDependencyID &rootModuleID, ModuleDependenciesCache &cache,
      ModuleDependencyIDSetVector &discoveredCodiraModules);
  void resolveAllClangModuleDependencies(
      ArrayRef<ModuleDependencyID> languageModules, ModuleDependenciesCache &cache,
      ModuleDependencyIDSetVector &discoveredClangModules);
  void resolveHeaderDependencies(
      ArrayRef<ModuleDependencyID> languageModules, ModuleDependenciesCache &cache,
      ModuleDependencyIDSetVector &discoveredHeaderDependencyClangModules);
  void resolveCodiraOverlayDependencies(
      ArrayRef<ModuleDependencyID> languageModules, ModuleDependenciesCache &cache,
      ModuleDependencyIDSetVector &discoveredDependencies);

  /// Resolve all of a given module's imports to a Codira module, if one exists.
  void resolveCodiraImportsForModule(
      const ModuleDependencyID &moduleID, ModuleDependenciesCache &cache,
      ModuleDependencyIDSetVector &importedCodiraDependencies);

  /// If a module has a bridging header or other header inputs, execute a
  /// dependency scan on it and record the dependencies.
  void resolveHeaderDependenciesForModule(
      const ModuleDependencyID &moduleID, ModuleDependenciesCache &cache,
      ModuleDependencyIDSetVector &headerClangModuleDependencies);

  /// Resolve all module dependencies comprised of Codira overlays
  /// of this module's Clang module dependencies.
  void resolveCodiraOverlayDependenciesForModule(
      const ModuleDependencyID &moduleID, ModuleDependenciesCache &cache,
      ModuleDependencyIDSetVector &languageOverlayDependencies);

  /// Identify all cross-import overlay module dependencies of the
  /// source module under scan and apply an action for each.
  void resolveCrossImportOverlayDependencies(
      StringRef mainModuleName, ModuleDependenciesCache &cache,
      toolchain::function_ref<void(ModuleDependencyID)> action);

  /// Perform Bridging Header Chaining.
  toolchain::Error
  performBridgingHeaderChaining(const ModuleDependencyID &rootModuleID,
                                ModuleDependenciesCache &cache,
                                ModuleDependencyIDSetVector &allModules);

  /// Perform an operation utilizing one of the Scanning workers
  /// available to this scanner.
  template <typename Function, typename... Args>
  auto withDependencyScanningWorker(Function &&F, Args &&...ArgList);

  /// Use the scanner's ASTContext to construct an `Identifier`
  /// for a given module name.
  Identifier getModuleImportIdentifier(StringRef moduleName);

  /// Assuming the \c `moduleImport` failed to resolve,
  /// iterate over all binary Codira module dependencies with serialized
  /// search paths and attempt to diagnose if the failed-to-resolve module
  /// can be found on any of them. Returns the path containing
  /// the module, if one is found.
  std::optional<std::pair<ModuleDependencyID, std::string>>
  attemptToFindResolvingSerializedSearchPath(
      const ScannerImportStatementInfo &moduleImport,
      const ModuleDependenciesCache &cache);

private:
  const CompilerInvocation &ScanCompilerInvocation;
  ASTContext &ScanASTContext;
  ModuleDependencyIssueReporter IssueReporter;

  /// The available pool of workers for filesystem module search
  unsigned NumThreads;
  std::list<std::unique_ptr<ModuleDependencyScanningWorker>> Workers;
  toolchain::DefaultThreadPool ScanningThreadPool;
  // CAS instance.
  std::shared_ptr<toolchain::cas::ObjectStore> CAS;
  std::shared_ptr<toolchain::cas::ActionCache> ActionCache;
  /// File prefix mapper.
  std::unique_ptr<toolchain::PrefixMapper> PrefixMapper;
  /// CAS file system for loading file content.
  toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> CacheFS;
  /// Protect worker access.
  std::mutex WorkersLock;
  /// Count of filesystem queries performed
  std::atomic<unsigned> NumLookups = 0;
};

} // namespace language
