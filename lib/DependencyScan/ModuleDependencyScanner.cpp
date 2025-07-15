//===--- ModuleDependencyScanner.cpp - Compute module dependencies --------===//
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

#include "language/DependencyScan/ModuleDependencyScanner.h"
#include "language/AST/ASTContext.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticSuppression.h"
#include "language/AST/DiagnosticsCommon.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/DiagnosticsSema.h"
#include "language/AST/ModuleDependencies.h"
#include "language/AST/ModuleLoader.h"
#include "language/AST/PluginLoader.h"
#include "language/AST/SourceFile.h"
#include "language/AST/TypeCheckRequests.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/Basic/FileTypes.h"
#include "language/Basic/PrettyStackTrace.h"
#include "language/ClangImporter/ClangImporter.h"
#include "language/Frontend/CompileJobCacheKey.h"
#include "language/Frontend/ModuleInterfaceLoader.h"
#include "language/Serialization/ScanningLoaders.h"
#include "language/Serialization/SerializedModuleLoader.h"
#include "language/Subsystems.h"
#include "clang/CAS/IncludeTree.h"
#include "clang/Frontend/CompilerInstance.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/SetOperations.h"
#include "toolchain/CAS/CASProvidingFileSystem.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/MemoryBufferRef.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/Threading.h"
#include "toolchain/Support/VersionTuple.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include "toolchain/Support/VirtualOutputBackend.h"
#include "toolchain/Support/VirtualOutputBackends.h"
#include "toolchain/Support/VirtualOutputFile.h"
#include "toolchain/Support/raw_ostream.h"
#include <algorithm>
#include <optional>

using namespace language;

static void findPath_dfs(ModuleDependencyID X, ModuleDependencyID Y,
                         ModuleDependencyIDSet &visited,
                         std::vector<ModuleDependencyID> &stack,
                         std::vector<ModuleDependencyID> &result,
                         const ModuleDependenciesCache &cache) {
  stack.push_back(X);
  if (X == Y) {
    copy(stack.begin(), stack.end(), std::back_inserter(result));
    return;
  }
  visited.insert(X);
  auto optionalNode = cache.findDependency(X);
  auto node = optionalNode.value();
  assert(optionalNode.has_value() && "Expected cache value for dependency.");
  for (const auto &dep : node->getModuleImports()) {
    std::optional<ModuleDependencyKind> lookupKind = std::nullopt;
    // Underlying Clang module needs an explicit lookup to avoid confusing it
    // with the parent Codira module.
    if ((dep.importIdentifier == X.ModuleName && node->isCodiraModule()) ||
        node->isClangModule())
      lookupKind = ModuleDependencyKind::Clang;

    auto optionalDepNode =
        cache.findDependency(dep.importIdentifier, lookupKind);
    if (!optionalDepNode.has_value())
      continue;
    auto depNode = optionalDepNode.value();
    auto depID = ModuleDependencyID{dep.importIdentifier, depNode->getKind()};
    if (!visited.count(depID)) {
      findPath_dfs(depID, Y, visited, stack, result, cache);
    }
  }
  stack.pop_back();
}

static std::vector<ModuleDependencyID>
findPathToDependency(ModuleDependencyID dependency,
                     const ModuleDependenciesCache &cache) {
  auto mainModuleDep = cache.findDependency(cache.getMainModuleName(),
                                            ModuleDependencyKind::CodiraSource);
  if (!mainModuleDep.has_value())
    return {};
  auto mainModuleID = ModuleDependencyID{cache.getMainModuleName().str(),
                                         ModuleDependencyKind::CodiraSource};
  auto visited = ModuleDependencyIDSet();
  auto stack = std::vector<ModuleDependencyID>();
  auto dependencyPath = std::vector<ModuleDependencyID>();
  findPath_dfs(mainModuleID, dependency, visited, stack, dependencyPath, cache);
  return dependencyPath;
}

static bool isCodiraDependencyKind(ModuleDependencyKind Kind) {
  return Kind == ModuleDependencyKind::CodiraInterface ||
         Kind == ModuleDependencyKind::CodiraSource ||
         Kind == ModuleDependencyKind::CodiraBinary;
}

// The Codira compiler does not have a concept of a working directory.
// It is instead handled by the Codira driver by resolving relative paths
// according to the driver's notion of a working directory. On the other hand,
// Clang does have a concept working directory which may be specified on a
// Clang invocation with '-working-directory'. If so, it is crucial that we
// use this directory as an argument to the Clang scanner invocation below.
static std::string
computeClangWorkingDirectory(const std::vector<std::string> &commandLineArgs,
                             const ASTContext &ctx) {
  std::string workingDir;
  auto clangWorkingDirPos = std::find(
      commandLineArgs.rbegin(), commandLineArgs.rend(), "-working-directory");
  if (clangWorkingDirPos == commandLineArgs.rend())
    workingDir =
        ctx.SourceMgr.getFileSystem()->getCurrentWorkingDirectory().get();
  else {
    if (clangWorkingDirPos - 1 == commandLineArgs.rend()) {
      ctx.Diags.diagnose(SourceLoc(), diag::clang_dependency_scan_error,
                         "Missing '-working-directory' argument");
      workingDir =
          ctx.SourceMgr.getFileSystem()->getCurrentWorkingDirectory().get();
    } else
      workingDir = *(clangWorkingDirPos - 1);
  }
  return workingDir;
}

static std::string moduleCacheRelativeLookupModuleOutput(
    const clang::tooling::dependencies::ModuleDeps &MD,
    clang::tooling::dependencies::ModuleOutputKind MOK,
    const StringRef moduleCachePath, const StringRef stableModuleCachePath,
    const StringRef runtimeResourcePath) {
  toolchain::SmallString<128> outputPath(moduleCachePath);
  if (MD.IsInStableDirectories)
    outputPath = stableModuleCachePath;

  // FIXME: This is a hack to treat Clang modules defined in the compiler's
  // own resource directory as stable, when they are not reported as such
  // by the Clang scanner.
  if (!runtimeResourcePath.empty() &&
      hasPrefix(toolchain::sys::path::begin(MD.ClangModuleMapFile),
                toolchain::sys::path::end(MD.ClangModuleMapFile),
                toolchain::sys::path::begin(runtimeResourcePath),
                toolchain::sys::path::end(runtimeResourcePath)))
    outputPath = stableModuleCachePath;

  toolchain::sys::path::append(outputPath,
                          MD.ID.ModuleName + "-" + MD.ID.ContextHash);
  switch (MOK) {
  case clang::tooling::dependencies::ModuleOutputKind::ModuleFile:
    toolchain::sys::path::replace_extension(
        outputPath, getExtension(language::file_types::TY_ClangModuleFile));
    break;
  case clang::tooling::dependencies::ModuleOutputKind::DependencyFile:
    toolchain::sys::path::replace_extension(
        outputPath, getExtension(language::file_types::TY_Dependencies));
    break;
  case clang::tooling::dependencies::ModuleOutputKind::DependencyTargets:
    return MD.ID.ModuleName + "-" + MD.ID.ContextHash;
  case clang::tooling::dependencies::ModuleOutputKind::
      DiagnosticSerializationFile:
    toolchain::sys::path::replace_extension(
        outputPath, getExtension(language::file_types::TY_SerializedDiagnostics));
    break;
  }
  return outputPath.str().str();
}

static std::vector<std::string> inputSpecificClangScannerCommand(
    const std::vector<std::string> &baseCommandLineArgs,
    std::optional<StringRef> sourceFileName) {
  std::vector<std::string> result(baseCommandLineArgs.begin(),
                                  baseCommandLineArgs.end());
  auto sourceFilePos =
      std::find(result.begin(), result.end(), "<language-imported-modules>");
  assert(sourceFilePos != result.end());
  if (sourceFileName.has_value())
    *sourceFilePos = sourceFileName->str();
  else
    result.erase(sourceFilePos);
  return result;
}

static std::string remapPath(toolchain::PrefixMapper *PrefixMapper, StringRef Path) {
  if (!PrefixMapper)
    return Path.str();
  return PrefixMapper->mapToString(Path);
}

static toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem>
getClangScanningFS(std::shared_ptr<toolchain::cas::ObjectStore> cas,
                   ASTContext &ctx) {
  toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> baseFileSystem =
      toolchain::vfs::createPhysicalFileSystem();
  ClangInvocationFileMapping fileMapping =
    applyClangInvocationMapping(ctx, nullptr, baseFileSystem, false);

  if (cas)
    return toolchain::cas::createCASProvidingFileSystem(cas, baseFileSystem);
  return baseFileSystem;
}

ModuleDependencyScanningWorker::ModuleDependencyScanningWorker(
    CodiraDependencyScanningService &globalScanningService,
    const CompilerInvocation &ScanCompilerInvocation,
    const SILOptions &SILOptions, ASTContext &ScanASTContext,
    language::DependencyTracker &DependencyTracker,
    std::shared_ptr<toolchain::cas::ObjectStore> CAS,
    std::shared_ptr<toolchain::cas::ActionCache> ActionCache,
    toolchain::PrefixMapper *Mapper, DiagnosticEngine &Diagnostics)
    : workerCompilerInvocation(
          std::make_unique<CompilerInvocation>(ScanCompilerInvocation)),
      workerASTContext(std::unique_ptr<ASTContext>(
          ASTContext::get(workerCompilerInvocation->getLangOptions(),
                          workerCompilerInvocation->getTypeCheckerOptions(),
                          workerCompilerInvocation->getSILOptions(),
                          workerCompilerInvocation->getSearchPathOptions(),
                          workerCompilerInvocation->getClangImporterOptions(),
                          workerCompilerInvocation->getSymbolGraphOptions(),
                          workerCompilerInvocation->getCASOptions(),
                          workerCompilerInvocation->getSerializationOptions(),
                          ScanASTContext.SourceMgr, Diagnostics))),
      scanningASTDelegate(std::make_unique<InterfaceSubContextDelegateImpl>(
          workerASTContext->SourceMgr, &workerASTContext->Diags,
          workerASTContext->SearchPathOpts, workerASTContext->LangOpts,
          workerASTContext->ClangImporterOpts, workerASTContext->CASOpts,
          workerCompilerInvocation->getFrontendOptions(),
          /* buildModuleCacheDirIfAbsent */ false,
          getModuleCachePathFromClang(
              ScanASTContext.getClangModuleLoader()->getClangInstance()),
          workerCompilerInvocation->getFrontendOptions()
              .PrebuiltModuleCachePath,
          workerCompilerInvocation->getFrontendOptions()
              .BackupModuleInterfaceDir,
          workerCompilerInvocation->getFrontendOptions()
              .SerializeModuleInterfaceDependencyHashes,
          workerCompilerInvocation->getFrontendOptions()
              .shouldTrackSystemDependencies(),
          RequireOSSAModules_t(SILOptions))),
      clangScanningTool(*globalScanningService.ClangScanningService,
                        getClangScanningFS(CAS, ScanASTContext)),
      moduleOutputPath(workerCompilerInvocation->getFrontendOptions()
                           .ExplicitModulesOutputPath),
      sdkModuleOutputPath(workerCompilerInvocation->getFrontendOptions()
                              .ExplicitSDKModulesOutputPath),
      CAS(CAS), ActionCache(ActionCache), PrefixMapper(Mapper) {
  auto loader = std::make_unique<PluginLoader>(
      *workerASTContext, /*DepTracker=*/nullptr,
      workerCompilerInvocation->getFrontendOptions().CacheReplayPrefixMap,
      workerCompilerInvocation->getFrontendOptions().DisableSandbox);
  workerASTContext->setPluginLoader(std::move(loader));

  // Set up the required command-line arguments and working directory
  // configuration required for clang dependency scanner queries
  auto scanClangImporter =
      static_cast<ClangImporter *>(ScanASTContext.getClangModuleLoader());
  clangScanningBaseCommandLineArgs =
      scanClangImporter->getClangDepScanningInvocationArguments(ScanASTContext);
  clangScanningModuleCommandLineArgs = inputSpecificClangScannerCommand(
      clangScanningBaseCommandLineArgs, std::nullopt);
  clangScanningWorkingDirectoryPath = computeClangWorkingDirectory(
      clangScanningBaseCommandLineArgs, ScanASTContext);

  // Handle clang arguments. For caching build, all arguments are passed
  // with `-direct-clang-cc1-module-build`.
  if (ScanASTContext.ClangImporterOpts.ClangImporterDirectCC1Scan) {
    languageModuleClangCC1CommandLineArgs.push_back(
        "-direct-clang-cc1-module-build");
    for (auto &Arg : scanClangImporter->getCodiraExplicitModuleDirectCC1Args()) {
      languageModuleClangCC1CommandLineArgs.push_back("-Xcc");
      languageModuleClangCC1CommandLineArgs.push_back(Arg);
    }
  } else {
    languageModuleClangCC1CommandLineArgs.push_back("-Xcc");
    languageModuleClangCC1CommandLineArgs.push_back("-fno-implicit-modules");
    languageModuleClangCC1CommandLineArgs.push_back("-Xcc");
    languageModuleClangCC1CommandLineArgs.push_back("-fno-implicit-module-maps");
  }

  // Set up the Codira module loader for Codira queries.
  languageModuleScannerLoader = std::make_unique<CodiraModuleScanner>(
      *workerASTContext,
      workerCompilerInvocation->getSearchPathOptions().ModuleLoadMode,
      *scanningASTDelegate, moduleOutputPath, sdkModuleOutputPath,
      languageModuleClangCC1CommandLineArgs);
}

CodiraModuleScannerQueryResult
ModuleDependencyScanningWorker::scanFilesystemForCodiraModuleDependency(
    Identifier moduleName, bool isTestableImport) {
  return languageModuleScannerLoader->lookupCodiraModule(moduleName,
                                                     isTestableImport);
}

ModuleDependencyVector
ModuleDependencyScanningWorker::scanFilesystemForClangModuleDependency(
    Identifier moduleName,
    const toolchain::DenseSet<clang::tooling::dependencies::ModuleID>
        &alreadySeenModules) {
  auto lookupModuleOutput =
      [this](const clang::tooling::dependencies::ModuleDeps &MD,
             const clang::tooling::dependencies::ModuleOutputKind MOK)
      -> std::string {
    return moduleCacheRelativeLookupModuleOutput(
        MD, MOK, moduleOutputPath, sdkModuleOutputPath,
        workerASTContext->SearchPathOpts.RuntimeResourcePath);
  };

  auto clangModuleDependencies = clangScanningTool.getModuleDependencies(
      moduleName.str(), clangScanningModuleCommandLineArgs,
      clangScanningWorkingDirectoryPath, alreadySeenModules,
      lookupModuleOutput);
  if (!clangModuleDependencies) {
    auto errorStr = toString(clangModuleDependencies.takeError());
    // We ignore the "module 'foo' not found" error, the Codira dependency
    // scanner will report such an error only if all of the module loaders
    // fail as well.
    if (errorStr.find("fatal error: module '" + moduleName.str().str() +
                      "' not found") == std::string::npos)
      workerASTContext->Diags.diagnose(
          SourceLoc(), diag::clang_dependency_scan_error, errorStr);
    return {};
  }

  return ClangImporter::bridgeClangModuleDependencies(
      *workerASTContext, clangScanningTool, *clangModuleDependencies,
      lookupModuleOutput,
      [&](StringRef path) { return remapPath(PrefixMapper, path); });
}

bool ModuleDependencyScanningWorker::scanHeaderDependenciesOfCodiraModule(
    const ASTContext &ctx, ModuleDependencyID moduleID,
    std::optional<StringRef> headerPath,
    std::optional<toolchain::MemoryBufferRef> sourceBuffer,
    ModuleDependenciesCache &cache,
    ModuleDependencyIDSetVector &headerClangModuleDependencies,
    std::vector<std::string> &headerFileInputs,
    std::vector<std::string> &bridgingHeaderCommandLine,
    std::optional<std::string> &includeTreeID) {
  // Scan the specified textual header file and collect its dependencies
  auto scanHeaderDependencies = [&]()
      -> toolchain::Expected<clang::tooling::dependencies::TranslationUnitDeps> {
    auto lookupModuleOutput =
        [this, &ctx](const clang::tooling::dependencies::ModuleDeps &MD,
                     const clang::tooling::dependencies::ModuleOutputKind MOK)
        -> std::string {
      return moduleCacheRelativeLookupModuleOutput(
          MD, MOK, moduleOutputPath, sdkModuleOutputPath,
          ctx.SearchPathOpts.RuntimeResourcePath);
    };

    auto dependencies = clangScanningTool.getTranslationUnitDependencies(
        inputSpecificClangScannerCommand(clangScanningBaseCommandLineArgs,
                                         headerPath),
        clangScanningWorkingDirectoryPath, cache.getAlreadySeenClangModules(),
        lookupModuleOutput, sourceBuffer);
    if (!dependencies)
      return dependencies.takeError();

    // Record module dependencies for each new module we found.
    auto bridgedDeps = ClangImporter::bridgeClangModuleDependencies(
        ctx, clangScanningTool, dependencies->ModuleGraph, lookupModuleOutput,
        [this](StringRef path) { return remapPath(PrefixMapper, path); });
    cache.recordDependencies(bridgedDeps, ctx.Diags);

    toolchain::copy(dependencies->FileDeps, std::back_inserter(headerFileInputs));
    auto bridgedDependencyIDs =
        toolchain::map_range(dependencies->ClangModuleDeps, [](auto &input) {
          return ModuleDependencyID{input.ModuleName,
                                    ModuleDependencyKind::Clang};
        });
    headerClangModuleDependencies.insert(bridgedDependencyIDs.begin(),
                                         bridgedDependencyIDs.end());
    return dependencies;
  };

  // - If a generated header is provided, scan the generated header.
  // - Textual module dependencies require us to process their bridging header.
  // - Binary module dependnecies may have arbitrary header inputs.
  auto clangModuleDependencies = scanHeaderDependencies();
  if (!clangModuleDependencies) {
    auto errorStr = toString(clangModuleDependencies.takeError());
    workerASTContext->Diags.diagnose(
        SourceLoc(), diag::clang_header_dependency_scan_error, errorStr);
    return true;
  }

  auto targetModuleInfo = cache.findKnownDependency(moduleID);
  if (!targetModuleInfo.isTextualCodiraModule())
    return false;

  if (auto TreeID = clangModuleDependencies->IncludeTreeID)
    includeTreeID = TreeID;
  ClangImporter::getBridgingHeaderOptions(ctx, *clangModuleDependencies,
                                          bridgingHeaderCommandLine);
  return false;
}

template <typename Function, typename... Args>
auto ModuleDependencyScanner::withDependencyScanningWorker(Function &&F,
                                                           Args &&...ArgList) {
  NumLookups++;
  auto getWorker = [this]() -> std::unique_ptr<ModuleDependencyScanningWorker> {
    std::lock_guard<std::mutex> guard(WorkersLock);
    // If we have run out of workers, something has gone wrong as we must never
    // have the number of workers exceeding the size of the thread pool
    // requesting them.
    if (Workers.empty())
      language_unreachable("Out of Codira dependency scanning workers.");

    // Otherwise, return from the back.
    auto result = std::move(Workers.back());
    Workers.pop_back();
    return result;
  };

  auto releaseWorker =
      [this](std::unique_ptr<ModuleDependencyScanningWorker> &&worker) {
        std::lock_guard<std::mutex> guard(WorkersLock);
        Workers.emplace_front(std::move(worker));
      };

  std::unique_ptr<ModuleDependencyScanningWorker> worker = getWorker();
  auto Task = std::bind(std::forward<Function>(F), worker.get(),
                        std::forward<Args>(ArgList)...);
  auto result = Task();
  releaseWorker(std::move(worker));
  return result;
}

toolchain::Error ModuleDependencyScanningWorker::createCacheKeyForEmbeddedHeader(
    std::string embeddedHeaderIncludeTree,
    std::string chainedHeaderIncludeTree) {
  assert(CAS && ActionCache && "CAS is not available");
  auto chained = CAS->parseID(chainedHeaderIncludeTree);
  if (!chained)
    return chained.takeError();
  auto chainedRef = CAS->getReference(*chained);
  if (!chainedRef)
    return toolchain::createStringError("Chained IncludeTree missing");
  auto embedded = CAS->parseID(embeddedHeaderIncludeTree);
  if (!embedded)
    return embedded.takeError();
  auto key =
      ClangImporter::createEmbeddedBridgingHeaderCacheKey(*CAS, *chainedRef);
  if (!key)
    return key.takeError();

  return ActionCache->put(CAS->getID(*key), *embedded);
}

Identifier
ModuleDependencyScanner::getModuleImportIdentifier(StringRef moduleName) {
  return ScanASTContext.getIdentifier(moduleName);
}

CodiraDependencyTracker::CodiraDependencyTracker(
    std::shared_ptr<toolchain::cas::ObjectStore> CAS, toolchain::PrefixMapper *Mapper,
    const CompilerInvocation &CI)
    : CAS(CAS), Mapper(Mapper) {
  auto &SearchPathOpts = CI.getSearchPathOptions();

  FS = toolchain::cas::createCASProvidingFileSystem(
      CAS, toolchain::vfs::createPhysicalFileSystem());

  auto addCommonFile = [&](StringRef path) {
    auto file = FS->openFileForRead(path);
    if (!file)
      return;
    auto status = (*file)->status();
    if (!status)
      return;
    auto fileRef = (*file)->getObjectRefForContent();
    if (!fileRef)
      return;

    std::string realPath = Mapper ? Mapper->mapToString(path) : path.str();
    CommonFiles.try_emplace(realPath, **fileRef, (size_t)status->getSize());
  };

  // Add SDKSetting file.
  SmallString<256> SDKSettingPath;
  toolchain::sys::path::append(SDKSettingPath, SearchPathOpts.getSDKPath(),
                          "SDKSettings.json");
  addCommonFile(SDKSettingPath);

  // Add Legacy layout file.
  const std::vector<std::string> AllSupportedArches = {
      "arm64", "arm64e", "x86_64", "i386",
      "armv7", "armv7s", "armv7k", "arm64_32"};

  for (auto RuntimeLibPath : SearchPathOpts.RuntimeLibraryPaths) {
    std::error_code EC;
    for (auto &Arch : AllSupportedArches) {
      SmallString<256> LayoutFile(RuntimeLibPath);
      toolchain::sys::path::append(LayoutFile, "layouts-" + Arch + ".yaml");
      addCommonFile(LayoutFile);
    }
  }

  // Add VFSOverlay file.
  for (auto &Overlay: SearchPathOpts.VFSOverlayFiles)
    addCommonFile(Overlay);

  // Add blocklist file.
  for (auto &File: CI.getFrontendOptions().BlocklistConfigFilePaths)
    addCommonFile(File);
}

void CodiraDependencyTracker::startTracking(bool includeCommonDeps) {
  TrackedFiles.clear();
  if (includeCommonDeps) {
    for (auto &entry : CommonFiles)
      TrackedFiles.emplace(entry.first(), entry.second);
  }
}

void CodiraDependencyTracker::trackFile(const Twine &path) {
  auto file = FS->openFileForRead(path);
  if (!file)
    return;
  auto status = (*file)->status();
  if (!status)
    return;
  auto fileRef = (*file)->getObjectRefForContent();
  if (!fileRef)
    return;
  std::string realPath =
      Mapper ? Mapper->mapToString(path.str()) : path.str();
  TrackedFiles.try_emplace(realPath, **fileRef, (size_t)status->getSize());
}

toolchain::Expected<toolchain::cas::ObjectProxy>
CodiraDependencyTracker::createTreeFromDependencies() {
  toolchain::SmallVector<clang::cas::IncludeTree::FileList::FileEntry> Files;
  for (auto &file : TrackedFiles) {
    auto includeTreeFile = clang::cas::IncludeTree::File::create(
        *CAS, file.first, file.second.FileRef);
    if (!includeTreeFile) {
      return toolchain::createStringError("CASFS createTree failed for " +
                                     file.first + ": " +
                                     toString(includeTreeFile.takeError()));
    }
    Files.push_back(
        {includeTreeFile->getRef(),
         (clang::cas::IncludeTree::FileList::FileSizeTy)file.second.Size});
  }

  auto includeTreeList =
      clang::cas::IncludeTree::FileList::create(*CAS, Files, {});
  if (!includeTreeList)
    return toolchain::createStringError("casfs include-tree filelist error: " +
                                   toString(includeTreeList.takeError()));

  return *includeTreeList;
}

ModuleDependencyScanner::ModuleDependencyScanner(
    CodiraDependencyScanningService &ScanningService,
    const CompilerInvocation &ScanCompilerInvocation,
    const SILOptions &SILOptions, ASTContext &ScanASTContext,
    language::DependencyTracker &DependencyTracker,
    std::shared_ptr<toolchain::cas::ObjectStore> CAS,
    std::shared_ptr<toolchain::cas::ActionCache> ActionCache,
    DiagnosticEngine &Diagnostics, bool ParallelScan)
    : ScanCompilerInvocation(ScanCompilerInvocation),
      ScanASTContext(ScanASTContext), IssueReporter(Diagnostics),
      NumThreads(ParallelScan
                     ? toolchain::hardware_concurrency().compute_thread_count()
                     : 1),
      ScanningThreadPool(toolchain::hardware_concurrency(NumThreads)), CAS(CAS),
      ActionCache(ActionCache) {
  // Setup prefix mapping.
  auto &ScannerPrefixMapper =
      ScanCompilerInvocation.getSearchPathOptions().ScannerPrefixMapper;
  if (!ScannerPrefixMapper.empty()) {
    PrefixMapper = std::make_unique<toolchain::PrefixMapper>();
    SmallVector<toolchain::MappedPrefix, 4> Prefixes;
    toolchain::MappedPrefix::transformPairs(ScannerPrefixMapper, Prefixes);
    PrefixMapper->addRange(Prefixes);
    PrefixMapper->sort();
  }

  if (CAS)
    CacheFS = toolchain::cas::createCASProvidingFileSystem(
        CAS, ScanASTContext.SourceMgr.getFileSystem());

  // TODO: Make num threads configurable
  for (size_t i = 0; i < NumThreads; ++i)
    Workers.emplace_front(std::make_unique<ModuleDependencyScanningWorker>(
        ScanningService, ScanCompilerInvocation, SILOptions, ScanASTContext,
        DependencyTracker, CAS, ActionCache, PrefixMapper.get(), Diagnostics));
}

/// Find all of the imported Clang modules starting with the given module name.
static void findAllImportedClangModules(StringRef moduleName,
                                        const ModuleDependenciesCache &cache,
                                        std::vector<std::string> &allModules,
                                        toolchain::StringSet<> &knownModules) {
  if (!knownModules.insert(moduleName).second)
    return;
  allModules.push_back(moduleName.str());
  auto moduleID =
      ModuleDependencyID{moduleName.str(), ModuleDependencyKind::Clang};
  auto optionalDependencies = cache.findDependency(moduleID);
  if (!optionalDependencies.has_value())
    return;

  for (const auto &dep : cache.getClangDependencies(moduleID))
    findAllImportedClangModules(dep.ModuleName, cache, allModules,
                                knownModules);
}

static std::set<ModuleDependencyID>
collectBinaryCodiraDeps(const ModuleDependenciesCache &cache) {
  std::set<ModuleDependencyID> binaryCodiraModuleDepIDs;
  auto binaryDepsMap =
      cache.getDependenciesMap(ModuleDependencyKind::CodiraBinary);
  for (const auto &binaryDepName : binaryDepsMap.keys())
    binaryCodiraModuleDepIDs.insert(ModuleDependencyID{
        binaryDepName.str(), ModuleDependencyKind::CodiraBinary});
  return binaryCodiraModuleDepIDs;
}

toolchain::ErrorOr<ModuleDependencyInfo>
ModuleDependencyScanner::getMainModuleDependencyInfo(ModuleDecl *mainModule) {
  // Main module file name.
  auto newExt = file_types::getExtension(file_types::TY_CodiraModuleFile);
  toolchain::SmallString<32> mainModulePath = mainModule->getName().str();
  toolchain::sys::path::replace_extension(mainModulePath, newExt);

  std::string apinotesVer = (toolchain::Twine("-fapinotes-language-version=") +
                             ScanASTContext.LangOpts.EffectiveLanguageVersion
                                 .asAPINotesVersionString())
                                .str();

  auto clangImporter =
      static_cast<ClangImporter *>(ScanASTContext.getClangModuleLoader());
  std::vector<std::string> buildArgs;
  if (ScanASTContext.ClangImporterOpts.ClangImporterDirectCC1Scan) {
    buildArgs.push_back("-direct-clang-cc1-module-build");
    for (auto &arg : clangImporter->getCodiraExplicitModuleDirectCC1Args()) {
      buildArgs.push_back("-Xcc");
      buildArgs.push_back(arg);
    }
  }

  toolchain::SmallVector<StringRef> buildCommands;
  buildCommands.reserve(buildArgs.size());
  toolchain::for_each(buildArgs, [&](const std::string &arg) {
    buildCommands.emplace_back(arg);
  });

  auto mainDependencies =
      ModuleDependencyInfo::forCodiraSourceModule({}, buildCommands, {}, {}, {});

  toolchain::StringSet<> alreadyAddedModules;
  // Compute Implicit dependencies of the main module
  {
    const auto &importInfo = mainModule->getImplicitImportInfo();
    // Codira standard library.
    switch (importInfo.StdlibKind) {
    case ImplicitStdlibKind::None:
    case ImplicitStdlibKind::Builtin:
      break;

    case ImplicitStdlibKind::Stdlib:
      mainDependencies.addModuleImport("Codira", /* isExported */ false,
                                       AccessLevel::Public,
                                       &alreadyAddedModules);
      break;
    }

    // Add any implicit module names.
    for (const auto &import : importInfo.AdditionalUnloadedImports) {
      mainDependencies.addModuleImport(
          import.module.getModulePath(),
          import.options.contains(ImportFlags::Exported), import.accessLevel,
          &alreadyAddedModules, &ScanASTContext.SourceMgr);
    }

    // Already-loaded, implicitly imported module names.
    for (const auto &import : importInfo.AdditionalImports) {
      mainDependencies.addModuleImport(
          import.module.importedModule->getNameStr(),
          import.options.contains(ImportFlags::Exported), import.accessLevel,
          &alreadyAddedModules);
    }

    // Add the bridging header.
    if (!importInfo.BridgingHeaderPath.empty()) {
      mainDependencies.addBridgingHeader(importInfo.BridgingHeaderPath);
    }

    // If we are to import the underlying Clang module of the same name,
    // add a dependency with the same name to trigger the search.
    if (importInfo.ShouldImportUnderlyingModule) {
      mainDependencies.addModuleImport(
          mainModule->getName().str(),
          /* isExported */ true, AccessLevel::Public, &alreadyAddedModules);
    }

    // All modules specified with `-embed-tbd-for-module` are treated as
    // implicit dependnecies for this compilation since they are not guaranteed
    // to be impored in the source.
    for (const auto &tbdSymbolModule :
         ScanCompilerInvocation.getTBDGenOptions().embedSymbolsFromModules) {
      mainDependencies.addModuleImport(
          tbdSymbolModule,
          /* isExported */ false, AccessLevel::Public, &alreadyAddedModules);
    }
  }

  // Add source-specified `import` dependencies
  {
    for (auto fileUnit : mainModule->getFiles()) {
      auto sourceFile = dyn_cast<SourceFile>(fileUnit);
      if (!sourceFile)
        continue;

      mainDependencies.addModuleImports(*sourceFile, alreadyAddedModules,
                                        &ScanASTContext.SourceMgr);
    }

    // Pass all the successful canImport checks from the ASTContext as part of
    // build command to main module to ensure frontend gets the same result.
    // This needs to happen after visiting all the top-level decls from all
    // SourceFiles.
    std::vector<std::string> buildArgs = mainDependencies.getCommandline();
    mainModule->getASTContext().forEachCanImportVersionCheck(
        [&](StringRef moduleName, const toolchain::VersionTuple &Version,
            const toolchain::VersionTuple &UnderlyingVersion) {
          if (Version.empty() && UnderlyingVersion.empty()) {
            buildArgs.push_back("-module-can-import");
            buildArgs.push_back(moduleName.str());
          } else {
            buildArgs.push_back("-module-can-import-version");
            buildArgs.push_back(moduleName.str());
            buildArgs.push_back(Version.getAsString());
            buildArgs.push_back(UnderlyingVersion.getAsString());
          }
        });
    mainDependencies.updateCommandLine(buildArgs);
  }

  return mainDependencies;
}

/// For the dependency set of the main module, discover all
/// cross-import overlays and their corresponding '.codecrossimport'
/// files. Cross-import overlay dependencies are required when
/// the two constituent modules are imported *from the same source file*,
/// directly or indirectly.
///
/// Given a complete module dependency graph in this stage of the scan,
/// the algorithm for discovering cross-import overlays is:
/// 1. For each source file of the module under scan construct a
///    set of module dependnecies only reachable from this source file.
/// 2. For each module set constructed in (1), perform pair-wise lookup
///    of cross import files for each pair of modules in the set.
///
/// Notably, if for some pair of modules 'A' and 'B' there exists
/// a cross-import overlay '_A_B', and these two modules are not reachable
/// from any single source file via direct or indirect imports, then
/// the cross-import overlay module is not required for compilation.
static void discoverCrossImportOverlayFiles(
    StringRef mainModuleName, ModuleDependenciesCache &cache,
    ASTContext &scanASTContext, toolchain::SetVector<Identifier> &newOverlays,
    std::set<std::pair<std::string, std::string>> &overlayFiles) {
  auto mainModuleInfo = cache.findKnownDependency(ModuleDependencyID{
      mainModuleName.str(), ModuleDependencyKind::CodiraSource});

  toolchain::StringMap<ModuleDependencyIDSet> perSourceFileDependencies;
  const ModuleDependencyIDSet mainModuleDirectCodiraDepsSet{
      mainModuleInfo.getImportedCodiraDependencies().begin(),
      mainModuleInfo.getImportedCodiraDependencies().end()};
  const ModuleDependencyIDSet mainModuleDirectClangDepsSet{
      mainModuleInfo.getImportedClangDependencies().begin(),
      mainModuleInfo.getImportedClangDependencies().end()};

  // A utility to map an import identifier to one of the
  // known resolved module dependencies
  auto getModuleIDForImportIdentifier =
      [](const std::string &importIdentifierStr,
         const ModuleDependencyIDSet &directCodiraDepsSet,
         const ModuleDependencyIDSet &directClangDepsSet)
      -> std::optional<ModuleDependencyID> {
    if (auto textualDepIt = directCodiraDepsSet.find(
            {importIdentifierStr, ModuleDependencyKind::CodiraInterface});
        textualDepIt != directCodiraDepsSet.end())
      return *textualDepIt;
    else if (auto binaryDepIt = directCodiraDepsSet.find(
                 {importIdentifierStr, ModuleDependencyKind::CodiraBinary});
             binaryDepIt != directCodiraDepsSet.end())
      return *binaryDepIt;
    else if (auto clangDepIt = directClangDepsSet.find(
                 {importIdentifierStr, ModuleDependencyKind::Clang});
             clangDepIt != directClangDepsSet.end())
      return *clangDepIt;
    else
      return std::nullopt;
  };

  // Collect the set of directly-imported module dependencies
  // for each source file in the source module under scan.
  for (const auto &import : mainModuleInfo.getModuleImports()) {
    auto importResolvedModuleID = getModuleIDForImportIdentifier(
        import.importIdentifier, mainModuleDirectCodiraDepsSet,
        mainModuleDirectClangDepsSet);
    if (importResolvedModuleID)
      for (const auto &importLocation : import.importLocations)
        perSourceFileDependencies[importLocation.bufferIdentifier].insert(
            *importResolvedModuleID);
  }

  // For each source-file, build a set of module dependencies of the
  // module under scan corresponding to a sub-graph of modules only reachable
  // from this source file's direct imports.
  for (auto &keyValPair : perSourceFileDependencies) {
    const auto &bufferIdentifier = keyValPair.getKey();
    auto &directDependencyIDs = keyValPair.second;
    SmallVector<ModuleDependencyID, 8> worklist{directDependencyIDs.begin(),
                                                directDependencyIDs.end()};
    while (!worklist.empty()) {
      auto moduleID = worklist.pop_back_val();
      perSourceFileDependencies[bufferIdentifier].insert(moduleID);
      if (isCodiraDependencyKind(moduleID.Kind)) {
        auto moduleInfo = cache.findKnownDependency(moduleID);
        if (toolchain::any_of(moduleInfo.getModuleImports(),
                         [](const ScannerImportStatementInfo &importInfo) {
                           return importInfo.isExported;
                         })) {
          const ModuleDependencyIDSet directCodiraDepsSet{
              moduleInfo.getImportedCodiraDependencies().begin(),
              moduleInfo.getImportedCodiraDependencies().end()};
          const ModuleDependencyIDSet directClangDepsSet{
              moduleInfo.getImportedClangDependencies().begin(),
              moduleInfo.getImportedClangDependencies().end()};
          for (const auto &import : moduleInfo.getModuleImports()) {
            if (import.isExported) {
              auto importResolvedDepID = getModuleIDForImportIdentifier(
                  import.importIdentifier, directCodiraDepsSet,
                  directClangDepsSet);
              if (importResolvedDepID &&
                  !perSourceFileDependencies[bufferIdentifier].count(
                      *importResolvedDepID))
                worklist.push_back(*importResolvedDepID);
            }
          }
        }
      }
    }
  }

  // Within a provided set of module dependencies reachable via
  // direct imports from a given file, determine the available and required
  // cross-import overlays.
  auto discoverCrossImportOverlayFilesForModuleSet =
      [&mainModuleName, &cache, &scanASTContext, &newOverlays,
       &overlayFiles](const ModuleDependencyIDSet &inputDependencies) {
        for (auto moduleID : inputDependencies) {
          auto moduleName = moduleID.ModuleName;
          // Do not look for overlays of main module under scan
          if (moduleName == mainModuleName)
            continue;

          auto dependencies =
              cache.findDependency(moduleName, moduleID.Kind).value();
          // Collect a map from secondary module name to cross-import overlay
          // names.
          auto overlayMap = dependencies->collectCrossImportOverlayNames(
              scanASTContext, moduleName, overlayFiles);
          if (overlayMap.empty())
            continue;
          for (const auto &dependencyId : inputDependencies) {
            auto moduleName = dependencyId.ModuleName;
            // Do not look for overlays of main module under scan
            if (moduleName == mainModuleName)
              continue;
            // check if any explicitly imported modules can serve as a
            // secondary module, and add the overlay names to the
            // dependencies list.
            for (auto overlayName : overlayMap[moduleName]) {
              if (overlayName.str() != mainModuleName &&
                  std::find_if(inputDependencies.begin(),
                               inputDependencies.end(),
                               [&](ModuleDependencyID Id) {
                                 return moduleName == overlayName.str();
                               }) == inputDependencies.end()) {
                newOverlays.insert(overlayName);
              }
            }
          }
        }
      };

  for (const auto &keyValPair : perSourceFileDependencies)
    discoverCrossImportOverlayFilesForModuleSet(keyValPair.second);
}

std::vector<ModuleDependencyID>
ModuleDependencyScanner::performDependencyScan(ModuleDependencyID rootModuleID,
                                               ModuleDependenciesCache &cache) {
  PrettyStackTraceStringAction trace("Performing dependency scan of: ",
                                     rootModuleID.ModuleName);
  // If scanning for an individual Clang module, simply resolve its imports
  if (rootModuleID.Kind == ModuleDependencyKind::Clang) {
    ModuleDependencyIDSetVector discoveredClangModules;
    resolveAllClangModuleDependencies({}, cache, discoveredClangModules);
    return discoveredClangModules.takeVector();
  }

  // Resolve all, direct and transitive, imported module dependencies:
  // 1. Imported Codira modules
  // 2. Imported Clang modules
  // 3. Clang module dependencies of bridging headers
  // 4. Codira Overlay modules of imported Clang modules
  //    This may call into 'resolveImportedModuleDependencies'
  //    for the newly-added Codira overlay dependencies.
  ModuleDependencyIDSetVector allModules =
      resolveImportedModuleDependencies(rootModuleID, cache);

  // 5. Resolve cross-import overlays
  // This must only be done for the main source module, since textual and
  // binary Codira modules already encode their dependencies on cross-import
  // overlays with explicit imports.
  if (ScanCompilerInvocation.getLangOptions().EnableCrossImportOverlays)
    resolveCrossImportOverlayDependencies(
        rootModuleID.ModuleName, cache,
        [&](ModuleDependencyID id) { allModules.insert(id); });

  if (ScanCompilerInvocation.getSearchPathOptions().BridgingHeaderChaining) {
    auto err = performBridgingHeaderChaining(rootModuleID, cache, allModules);
    if (err)
      IssueReporter.Diagnostics.diagnose(SourceLoc(),
                                         diag::error_scanner_extra,
                                         toString(std::move(err)));
  }

  return allModules.takeVector();
}

ModuleDependencyIDSetVector
ModuleDependencyScanner::resolveImportedModuleDependencies(
    const ModuleDependencyID &rootModuleID, ModuleDependenciesCache &cache) {
  PrettyStackTraceStringAction trace(
      "Resolving transitive closure of dependencies of: ",
      rootModuleID.ModuleName);
  ModuleDependencyIDSetVector allModules;

  // Resolve all imports for which a Codira module can be found,
  // transitively, starting at 'rootModuleID'.
  ModuleDependencyIDSetVector discoveredCodiraModules;
  resolveCodiraModuleDependencies(rootModuleID, cache, discoveredCodiraModules);
  allModules.insert(discoveredCodiraModules.begin(),
                    discoveredCodiraModules.end());

  ModuleDependencyIDSetVector discoveredClangModules;
  resolveAllClangModuleDependencies(discoveredCodiraModules.getArrayRef(), cache,
                                    discoveredClangModules);
  allModules.insert(discoveredClangModules.begin(),
                    discoveredClangModules.end());

  ModuleDependencyIDSetVector discoveredHeaderDependencyClangModules;
  resolveHeaderDependencies(discoveredCodiraModules.getArrayRef(), cache,
                            discoveredHeaderDependencyClangModules);
  allModules.insert(discoveredHeaderDependencyClangModules.begin(),
                    discoveredHeaderDependencyClangModules.end());

  ModuleDependencyIDSetVector discoveredCodiraOverlayDependencyModules;
  resolveCodiraOverlayDependencies(discoveredCodiraModules.getArrayRef(), cache,
                                  discoveredCodiraOverlayDependencyModules);
  allModules.insert(discoveredCodiraOverlayDependencyModules.begin(),
                    discoveredCodiraOverlayDependencyModules.end());

  return allModules;
}

void ModuleDependencyScanner::resolveCodiraModuleDependencies(
    const ModuleDependencyID &rootModuleID, ModuleDependenciesCache &cache,
    ModuleDependencyIDSetVector &allDiscoveredCodiraModules) {
  PrettyStackTraceStringAction trace(
      "Resolving transitive closure of Codira dependencies of: ",
      rootModuleID.ModuleName);
  // Clang modules cannot have Codira module dependencies
  if (!isCodiraDependencyKind(rootModuleID.Kind))
    return;

  allDiscoveredCodiraModules.insert(rootModuleID);
  for (unsigned currentModuleIdx = 0;
       currentModuleIdx < allDiscoveredCodiraModules.size();
       ++currentModuleIdx) {
    auto moduleID = allDiscoveredCodiraModules[currentModuleIdx];
    auto moduleDependencyInfo = cache.findKnownDependency(moduleID);

    // If this dependency module's Codira imports are already resolved,
    // we do not need to scan it.
    if (!moduleDependencyInfo.getImportedCodiraDependencies().empty()) {
      for (const auto &dep :
           moduleDependencyInfo.getImportedCodiraDependencies())
        allDiscoveredCodiraModules.insert(dep);
    } else {
      // Find the Codira dependencies of every module this module directly
      // depends on.
      ModuleDependencyIDSetVector importedCodiraDependencies;
      resolveCodiraImportsForModule(moduleID, cache, importedCodiraDependencies);
      allDiscoveredCodiraModules.insert(importedCodiraDependencies.begin(),
                                       importedCodiraDependencies.end());
    }
  }
  return;
}

void ModuleDependencyScanner::resolveAllClangModuleDependencies(
    ArrayRef<ModuleDependencyID> languageModuleDependents,
    ModuleDependenciesCache &cache,
    ModuleDependencyIDSetVector &allDiscoveredClangModules) {
  // Gather all unresolved imports which must correspond to
  // Clang modules (since no Codira module for them was found).
  toolchain::StringSet<> unresolvedImportIdentifiers;
  toolchain::StringSet<> unresolvedOptionalImportIdentifiers;
  std::unordered_map<ModuleDependencyID,
                     std::vector<ScannerImportStatementInfo>>
      unresolvedImportsMap;
  std::unordered_map<ModuleDependencyID,
                     std::vector<ScannerImportStatementInfo>>
      unresolvedOptionalImportsMap;

  for (const auto &moduleID : languageModuleDependents) {
    auto moduleDependencyInfo = cache.findKnownDependency(moduleID);
    auto unresolvedImports =
        &unresolvedImportsMap
             .emplace(moduleID, std::vector<ScannerImportStatementInfo>())
             .first->second;
    auto unresolvedOptionalImports =
        &unresolvedOptionalImportsMap
             .emplace(moduleID, std::vector<ScannerImportStatementInfo>())
             .first->second;

    // If we have already resolved Clang dependencies for this module,
    // then we have the entire dependency sub-graph already computed for
    // it and ready to be added to 'allDiscoveredClangModules' without
    // additional scanning.
    if (!moduleDependencyInfo.getImportedClangDependencies().empty()) {
      auto directClangDeps = cache.getImportedClangDependencies(moduleID);
      ModuleDependencyIDSetVector reachableClangModules;
      reachableClangModules.insert(directClangDeps.begin(),
                                   directClangDeps.end());
      for (unsigned currentModuleIdx = 0;
           currentModuleIdx < reachableClangModules.size();
           ++currentModuleIdx) {
        auto moduleID = reachableClangModules[currentModuleIdx];
        auto dependencies =
            cache.findKnownDependency(moduleID).getImportedClangDependencies();
        reachableClangModules.insert(dependencies.begin(), dependencies.end());
      }
      allDiscoveredClangModules.insert(reachableClangModules.begin(),
                                       reachableClangModules.end());
      continue;
    } else {
      // We need to query the Clang dependency scanner for this module's
      // unresolved imports
      toolchain::StringSet<> resolvedImportIdentifiers;
      for (const auto &resolvedDep :
           moduleDependencyInfo.getImportedCodiraDependencies())
        resolvedImportIdentifiers.insert(resolvedDep.ModuleName);

      // When querying a *clang* module 'CxxStdlib' we must
      // instead expect a module called 'std'...
      auto addCanonicalClangModuleImport =
          [this](const ScannerImportStatementInfo &importInfo,
                 std::vector<ScannerImportStatementInfo> &unresolvedImports,
                 toolchain::StringSet<> &unresolvedImportIdentifiers) {
            if (importInfo.importIdentifier ==
                ScanASTContext.Id_CxxStdlib.str()) {
              auto canonicalImportInfo = ScannerImportStatementInfo(
                  "std", importInfo.isExported, importInfo.accessLevel,
                  importInfo.importLocations);
              unresolvedImports.push_back(canonicalImportInfo);
              unresolvedImportIdentifiers.insert(
                  canonicalImportInfo.importIdentifier);
            } else {
              unresolvedImports.push_back(importInfo);
              unresolvedImportIdentifiers.insert(importInfo.importIdentifier);
            }
          };

      for (const auto &depImport : moduleDependencyInfo.getModuleImports())
        if (!resolvedImportIdentifiers.contains(depImport.importIdentifier))
          addCanonicalClangModuleImport(depImport, *unresolvedImports,
                                        unresolvedImportIdentifiers);
      for (const auto &depImport :
           moduleDependencyInfo.getOptionalModuleImports())
        if (!resolvedImportIdentifiers.contains(depImport.importIdentifier))
          addCanonicalClangModuleImport(depImport, *unresolvedOptionalImports,
                                        unresolvedOptionalImportIdentifiers);
    }
  }

  // Prepare the module lookup result collection
  toolchain::StringMap<std::optional<ModuleDependencyVector>> moduleLookupResult;
  for (const auto &unresolvedIdentifier : unresolvedImportIdentifiers)
    moduleLookupResult.insert(
        std::make_pair(unresolvedIdentifier.getKey(), std::nullopt));

  // We need a copy of the shared already-seen module set, which will be shared
  // amongst all the workers. In `recordDependencies`, each worker will
  // contribute its results back to the shared set for future lookups.
  const toolchain::DenseSet<clang::tooling::dependencies::ModuleID>
      seenClangModules = cache.getAlreadySeenClangModules();
  std::mutex cacheAccessLock;
  auto scanForClangModuleDependency =
      [this, &cache, &moduleLookupResult, &cacheAccessLock,
       &seenClangModules](Identifier moduleIdentifier) {
        auto moduleName = moduleIdentifier.str();
        {
          std::lock_guard<std::mutex> guard(cacheAccessLock);
          if (cache.hasDependency(moduleName, ModuleDependencyKind::Clang))
            return;
        }

        auto moduleDependencies = withDependencyScanningWorker(
            [&seenClangModules,
             moduleIdentifier](ModuleDependencyScanningWorker *ScanningWorker) {
              return ScanningWorker->scanFilesystemForClangModuleDependency(
                  moduleIdentifier, seenClangModules);
            });

        // Update the `moduleLookupResult` and cache all discovered dependencies
        // so that subsequent queries do not have to call into the scanner
        // if looking for a module that was discovered as a transitive
        // dependency in this scan.
        {
          std::lock_guard<std::mutex> guard(cacheAccessLock);
          moduleLookupResult.insert_or_assign(moduleName, moduleDependencies);
          if (!moduleDependencies.empty())
            cache.recordDependencies(moduleDependencies,
                                     IssueReporter.Diagnostics);
        }
      };

  // Enque asynchronous lookup tasks
  for (const auto &unresolvedIdentifier : unresolvedImportIdentifiers)
    ScanningThreadPool.async(
        scanForClangModuleDependency,
        getModuleImportIdentifier(unresolvedIdentifier.getKey()));
  for (const auto &unresolvedIdentifier : unresolvedOptionalImportIdentifiers)
    ScanningThreadPool.async(
        scanForClangModuleDependency,
        getModuleImportIdentifier(unresolvedIdentifier.getKey()));
  ScanningThreadPool.wait();

  // Use the computed scan results to update the dependency info
  for (const auto &moduleID : languageModuleDependents) {
    std::vector<ScannerImportStatementInfo> failedToResolveImports;
    ModuleDependencyIDSetVector importedClangDependencies;
    auto recordResolvedClangModuleImport =
        [&moduleLookupResult, &importedClangDependencies,
         &allDiscoveredClangModules, moduleID, &failedToResolveImports](
            const ScannerImportStatementInfo &moduleImport,
            bool optionalImport) {
          auto lookupResult = moduleLookupResult[moduleImport.importIdentifier];
          // The imported module was found in the cache
          if (lookupResult == std::nullopt) {
            importedClangDependencies.insert(
                {moduleImport.importIdentifier, ModuleDependencyKind::Clang});
          } else {
            // Cache discovered module dependencies.
            if (!lookupResult.value().empty()) {
              importedClangDependencies.insert(
                  {moduleImport.importIdentifier, ModuleDependencyKind::Clang});
              // Add the full transitive dependency set
              for (const auto &dep : lookupResult.value())
                allDiscoveredClangModules.insert(dep.first);
            } else if (!optionalImport) {
              // Otherwise, we failed to resolve this dependency. We will try
              // again using the cache after all other imports have been
              // resolved. If that fails too, a scanning failure will be
              // diagnosed.
              failedToResolveImports.push_back(moduleImport);
            }
          }
        };

    for (const auto &unresolvedImport : unresolvedImportsMap[moduleID])
      recordResolvedClangModuleImport(unresolvedImport, false);
    for (const auto &unresolvedImport : unresolvedOptionalImportsMap[moduleID])
      recordResolvedClangModuleImport(unresolvedImport, true);

    // It is possible that a specific import resolution failed because we are
    // attempting to resolve a module which can only be brought in via a
    // modulemap of a different Clang module dependency which is not otherwise
    // on the current search paths. For example, suppose we are scanning a
    // `.codeinterface` for module `Foo`, which contains:
    // -----
    // @_exported import Foo
    // import Bar
    // ...
    // -----
    // Where `Foo` is the underlying Framework clang module whose .modulemap
    // defines an auxiliary module `Bar`. Because Foo is a framework, its
    // modulemap is under
    // `<some_framework_search_path>/Foo.framework/Modules/module.modulemap`.
    // Which means that lookup of `Bar` alone from Codira will not be able to
    // locate the module in it. However, the lookup of Foo will itself bring in
    // the auxiliary module becuase the Clang scanner instance scanning for
    // clang module Foo will be able to find it in the corresponding framework
    // module's modulemap and register it as a dependency which means it will be
    // registered with the scanner's cache in the step above. To handle such
    // cases, we first add all successfully-resolved modules and (for Clang
    // modules) their transitive dependencies to the cache, and then attempt to
    // re-query imports for which resolution originally failed from the cache.
    // If this fails, then the scanner genuinely failed to resolve this
    // dependency.
    for (const auto &unresolvedImport : failedToResolveImports) {
      auto unresolvedModuleID = ModuleDependencyID{
          unresolvedImport.importIdentifier, ModuleDependencyKind::Clang};
      auto optionalCachedModuleInfo = cache.findDependency(unresolvedModuleID);
      if (optionalCachedModuleInfo.has_value())
        importedClangDependencies.insert(unresolvedModuleID);
      else {
        // Failed to resolve module dependency.
        IssueReporter.diagnoseModuleNotFoundFailure(
            unresolvedImport, cache, moduleID,
            attemptToFindResolvingSerializedSearchPath(unresolvedImport,
                                                       cache));
      }
    }

    if (!importedClangDependencies.empty())
      cache.setImportedClangDependencies(
          moduleID, importedClangDependencies.takeVector());
  }

  return;
}

void ModuleDependencyScanner::resolveHeaderDependencies(
    ArrayRef<ModuleDependencyID> allCodiraModules,
    ModuleDependenciesCache &cache,
    ModuleDependencyIDSetVector &allDiscoveredHeaderDependencyClangModules) {
  for (const auto &moduleID : allCodiraModules) {
    auto moduleDependencyInfo = cache.findKnownDependency(moduleID);
    if (!moduleDependencyInfo.getHeaderClangDependencies().empty()) {
      allDiscoveredHeaderDependencyClangModules.insert(
          moduleDependencyInfo.getHeaderClangDependencies().begin(),
          moduleDependencyInfo.getHeaderClangDependencies().end());
    } else {
      ModuleDependencyIDSetVector headerClangModuleDependencies;
      resolveHeaderDependenciesForModule(moduleID, cache,
                                         headerClangModuleDependencies);
      allDiscoveredHeaderDependencyClangModules.insert(
          headerClangModuleDependencies.begin(),
          headerClangModuleDependencies.end());
    }
  }
}

void ModuleDependencyScanner::resolveCodiraOverlayDependencies(
    ArrayRef<ModuleDependencyID> allCodiraModules,
    ModuleDependenciesCache &cache,
    ModuleDependencyIDSetVector &allDiscoveredDependencies) {
  ModuleDependencyIDSetVector discoveredCodiraOverlays;
  for (const auto &moduleID : allCodiraModules) {
    auto moduleDependencyInfo = cache.findKnownDependency(moduleID);
    if (!moduleDependencyInfo.getCodiraOverlayDependencies().empty()) {
      allDiscoveredDependencies.insert(
          moduleDependencyInfo.getCodiraOverlayDependencies().begin(),
          moduleDependencyInfo.getCodiraOverlayDependencies().end());
    } else {
      ModuleDependencyIDSetVector languageOverlayDependencies;
      resolveCodiraOverlayDependenciesForModule(moduleID, cache,
                                               languageOverlayDependencies);
      discoveredCodiraOverlays.insert(languageOverlayDependencies.begin(),
                                     languageOverlayDependencies.end());
    }
  }

  // For each additional Codira overlay dependency, ensure we perform a full scan
  // in case it itself has unresolved module dependencies.
  for (const auto &overlayDepID : discoveredCodiraOverlays) {
    ModuleDependencyIDSetVector allNewModules =
        resolveImportedModuleDependencies(overlayDepID, cache);
    allDiscoveredDependencies.insert(allNewModules.begin(),
                                     allNewModules.end());
  }

  allDiscoveredDependencies.insert(discoveredCodiraOverlays.begin(),
                                   discoveredCodiraOverlays.end());
}

void ModuleDependencyScanner::resolveCodiraImportsForModule(
    const ModuleDependencyID &moduleID, ModuleDependenciesCache &cache,
    ModuleDependencyIDSetVector &importedCodiraDependencies) {
  PrettyStackTraceStringAction trace("Resolving Codira imports of: ",
                                     moduleID.ModuleName);
  if (!isCodiraDependencyKind(moduleID.Kind))
    return;

  auto moduleDependencyInfo = cache.findKnownDependency(moduleID);
  toolchain::StringMap<CodiraModuleScannerQueryResult> moduleLookupResult;
  std::mutex lookupResultLock;

  // A scanning task to query a module by-name. If the module already exists
  // in the cache, do nothing and return.
  auto scanForCodiraModuleDependency =
      [this, &cache, &lookupResultLock,
       &moduleLookupResult](Identifier moduleIdentifier, bool isTestable) {
        auto moduleName = moduleIdentifier.str().str();
        {
          std::lock_guard<std::mutex> guard(lookupResultLock);
          if (cache.hasCodiraDependency(moduleName))
            return;
        }

        auto moduleDependencies = withDependencyScanningWorker(
            [moduleIdentifier,
             isTestable](ModuleDependencyScanningWorker *ScanningWorker) {
              return ScanningWorker->scanFilesystemForCodiraModuleDependency(
                  moduleIdentifier, isTestable);
            });

        {
          std::lock_guard<std::mutex> guard(lookupResultLock);
          moduleLookupResult.insert_or_assign(moduleName, moduleDependencies);
        }
      };

  // Enque asynchronous lookup tasks
  for (const auto &dependsOn : moduleDependencyInfo.getModuleImports()) {
    // Avoid querying the underlying Clang module here
    if (moduleID.ModuleName == dependsOn.importIdentifier)
      continue;
    ScanningThreadPool.async(
        scanForCodiraModuleDependency,
        getModuleImportIdentifier(dependsOn.importIdentifier),
        moduleDependencyInfo.isTestableImport(dependsOn.importIdentifier));
  }
  for (const auto &dependsOn :
       moduleDependencyInfo.getOptionalModuleImports()) {
    // Avoid querying the underlying Clang module here
    if (moduleID.ModuleName == dependsOn.importIdentifier)
      continue;
    ScanningThreadPool.async(
        scanForCodiraModuleDependency,
        getModuleImportIdentifier(dependsOn.importIdentifier),
        moduleDependencyInfo.isTestableImport(dependsOn.importIdentifier));
  }
  ScanningThreadPool.wait();

  auto recordResolvedModuleImport =
      [this, &cache, &moduleLookupResult, &importedCodiraDependencies,
       moduleID](const ScannerImportStatementInfo &moduleImport) {
        if (moduleID.ModuleName == moduleImport.importIdentifier)
          return;
        auto lookupResult = moduleLookupResult[moduleImport.importIdentifier];

        // Query found module
        if (lookupResult.foundDependencyInfo) {
          cache.recordDependency(moduleImport.importIdentifier,
                                 *(lookupResult.foundDependencyInfo));
          importedCodiraDependencies.insert(
              {moduleImport.importIdentifier,
               lookupResult.foundDependencyInfo->getKind()});
          IssueReporter.warnOnIncompatibleCandidates(
              moduleImport.importIdentifier,
              lookupResult.incompatibleCandidates);
          // Module was resolved from a cache
        } else if (auto cachedInfo = cache.findCodiraDependency(
                       moduleImport.importIdentifier))
          importedCodiraDependencies.insert(
              {moduleImport.importIdentifier, cachedInfo.value()->getKind()});
        else
          IssueReporter.diagnoseFailureOnOnlyIncompatibleCandidates(
              moduleImport, lookupResult.incompatibleCandidates, cache,
              std::nullopt);
      };

  for (const auto &importInfo : moduleDependencyInfo.getModuleImports())
    recordResolvedModuleImport(importInfo);
  for (const auto &importInfo : moduleDependencyInfo.getOptionalModuleImports())
    recordResolvedModuleImport(importInfo);

  // Resolve the dependency info with Codira dependency module information.
  cache.setImportedCodiraDependencies(moduleID,
                                     importedCodiraDependencies.getArrayRef());
}

void ModuleDependencyScanner::resolveHeaderDependenciesForModule(
    const ModuleDependencyID &moduleID, ModuleDependenciesCache &cache,
    ModuleDependencyIDSetVector &headerClangModuleDependencies) {
  PrettyStackTraceStringAction trace(
      "Resolving header dependencies of Codira module", moduleID.ModuleName);
  std::vector<std::string> allClangModules;
  toolchain::StringSet<> alreadyKnownModules;
  auto moduleDependencyInfo = cache.findKnownDependency(moduleID);

  bool isTextualModuleWithABridgingHeader =
      moduleDependencyInfo.isTextualCodiraModule() &&
      moduleDependencyInfo.getBridgingHeader();
  bool isBinaryModuleWithHeaderInput =
      moduleDependencyInfo.isCodiraBinaryModule() &&
      !moduleDependencyInfo.getAsCodiraBinaryModule()->headerImport.empty();

  if (!isTextualModuleWithABridgingHeader && !isBinaryModuleWithHeaderInput)
    return;

  std::optional<std::string> headerPath;
  std::unique_ptr<toolchain::MemoryBuffer> sourceBuffer;
  std::optional<toolchain::MemoryBufferRef> sourceBufferRef;

  auto extractHeaderContent =
      [&](const CodiraBinaryModuleDependencyStorage &binaryMod)
      -> std::unique_ptr<toolchain::MemoryBuffer> {
    auto header = binaryMod.headerImport;
    // Check to see if the header input exists on disk.
    auto FS = ScanASTContext.SourceMgr.getFileSystem();
    if (FS->exists(header))
      return nullptr;

    auto moduleBuf = FS->getBufferForFile(binaryMod.compiledModulePath);
    if (!moduleBuf)
      return nullptr;

    auto content = extractEmbeddedBridgingHeaderContent(std::move(*moduleBuf),
                                                        ScanASTContext);
    if (content.empty())
      return nullptr;

    return toolchain::MemoryBuffer::getMemBufferCopy(content, header);
  };

  if (isBinaryModuleWithHeaderInput) {
    auto &binaryMod = *moduleDependencyInfo.getAsCodiraBinaryModule();
    if (auto embeddedHeader = extractHeaderContent(binaryMod)) {
      sourceBuffer = std::move(embeddedHeader);
      sourceBufferRef = sourceBuffer->getMemBufferRef();
    } else
      headerPath = binaryMod.headerImport;
  } else
    headerPath = *moduleDependencyInfo.getBridgingHeader();

  withDependencyScanningWorker(
      [&](ModuleDependencyScanningWorker *ScanningWorker) {
        std::vector<std::string> headerFileInputs;
        std::optional<std::string> includeTreeID;
        std::vector<std::string> bridgingHeaderCommandLine;
        auto headerScan = ScanningWorker->scanHeaderDependenciesOfCodiraModule(
            *ScanningWorker->workerASTContext, moduleID, headerPath,
            sourceBufferRef, cache, headerClangModuleDependencies,
            headerFileInputs, bridgingHeaderCommandLine, includeTreeID);
        if (!headerScan) {
          // Record direct header Clang dependencies
          cache.setHeaderClangDependencies(
              moduleID, headerClangModuleDependencies.getArrayRef());
          // Record include Tree ID
          if (includeTreeID)
            moduleDependencyInfo.addBridgingHeaderIncludeTree(*includeTreeID);
          // Record the bridging header command line
          if (isTextualModuleWithABridgingHeader)
            moduleDependencyInfo.updateBridgingHeaderCommandLine(
                bridgingHeaderCommandLine);
          moduleDependencyInfo.setHeaderSourceFiles(headerFileInputs);
          // Update the dependency in the cache
          cache.updateDependency(moduleID, moduleDependencyInfo);
        } else {
          // Failure to scan header
        }
        return true;
      });
  cache.setHeaderClangDependencies(moduleID,
                                   headerClangModuleDependencies.getArrayRef());
}

void ModuleDependencyScanner::resolveCodiraOverlayDependenciesForModule(
    const ModuleDependencyID &moduleID, ModuleDependenciesCache &cache,
    ModuleDependencyIDSetVector &languageOverlayDependencies) {
  PrettyStackTraceStringAction trace(
      "Resolving Codira Overlay dependencies of module", moduleID.ModuleName);
  std::vector<std::string> allClangDependencies;
  toolchain::StringSet<> knownModules;

  // Find all of the discovered Clang modules that this module depends on.
  for (const auto &dep : cache.getClangDependencies(moduleID))
    findAllImportedClangModules(dep.ModuleName, cache, allClangDependencies,
                                knownModules);

  toolchain::StringMap<CodiraModuleScannerQueryResult> languageOverlayLookupResult;
  std::mutex lookupResultLock;
  
  // A scanning task to query a Codira module by-name. If the module already
  // exists in the cache, do nothing and return.
  auto scanForCodiraDependency = [this, &cache, &lookupResultLock,
                                 &languageOverlayLookupResult](
                                    Identifier moduleIdentifier) {
    auto moduleName = moduleIdentifier.str();
    {
      std::lock_guard<std::mutex> guard(lookupResultLock);
      if (cache.hasDependency(moduleName, ModuleDependencyKind::CodiraInterface) ||
          cache.hasDependency(moduleName, ModuleDependencyKind::CodiraBinary))
        return;
    }

    auto moduleDependencies = withDependencyScanningWorker(
        [moduleIdentifier](ModuleDependencyScanningWorker *ScanningWorker) {
          return ScanningWorker->scanFilesystemForCodiraModuleDependency(
              moduleIdentifier, /* isTestableImport */ false);
        });
                                      
    {
      std::lock_guard<std::mutex> guard(lookupResultLock);
      languageOverlayLookupResult.insert_or_assign(moduleName, moduleDependencies);
    }
  };

  // Enque asynchronous lookup tasks
  for (const auto &clangDep : allClangDependencies)
    ScanningThreadPool.async(scanForCodiraDependency,
                             getModuleImportIdentifier(clangDep));
  ScanningThreadPool.wait();

  // Aggregate both previously-cached and freshly-scanned module results
  auto recordResult = [this, &cache, &languageOverlayLookupResult,
                       &languageOverlayDependencies,
                       moduleID](const std::string &moduleName) {
    auto lookupResult = languageOverlayLookupResult[moduleName];
    if (moduleName != moduleID.ModuleName) {

      // Query found module
      if (lookupResult.foundDependencyInfo) {
        cache.recordDependency(moduleName, *(lookupResult.foundDependencyInfo));
        languageOverlayDependencies.insert(
            {moduleName, lookupResult.foundDependencyInfo->getKind()});
        IssueReporter.warnOnIncompatibleCandidates(
            moduleName, lookupResult.incompatibleCandidates);
        // Module was resolved from a cache
      } else if (auto cachedInfo = cache.findCodiraDependency(moduleName))
        languageOverlayDependencies.insert(
            {moduleName, cachedInfo.value()->getKind()});
      else
        IssueReporter.diagnoseFailureOnOnlyIncompatibleCandidates(
            ScannerImportStatementInfo(moduleName),
            lookupResult.incompatibleCandidates, cache, std::nullopt);
    }
  };
  for (const auto &clangDep : allClangDependencies)
    recordResult(clangDep);

  // C++ Interop requires additional handling
  bool lookupCxxStdLibOverlay = ScanCompilerInvocation.getLangOptions().EnableCXXInterop;
  if (lookupCxxStdLibOverlay && moduleID.Kind == ModuleDependencyKind::CodiraInterface) {
    const auto &moduleInfo = cache.findKnownDependency(moduleID);
    const auto commandLine = moduleInfo.getCommandline();
    // If the textual interface was built without C++ interop, do not query
    // the C++ Standard Library Codira overlay for its compilation.
    //
    // FIXME: We always declare the 'Darwin' module as formally having been built
    // without C++Interop, for compatibility with prior versions. Once we are certain
    // that we are only building against modules built with support of
    // '-formal-cxx-interoperability-mode', this hard-coded check should be removed.
    if (moduleID.ModuleName == "Darwin" ||
        toolchain::find(commandLine, "-formal-cxx-interoperability-mode=off") !=
         commandLine.end())
      lookupCxxStdLibOverlay = false;
  }

  if (lookupCxxStdLibOverlay) {
    for (const auto &clangDepName : allClangDependencies) {
      // If this Clang module is a part of the C++ stdlib, and we haven't
      // loaded the overlay for it so far, it is a split libc++ module (e.g.
      // std_vector). Load the CxxStdlib overlay explicitly.
      const auto &clangDepInfo =
          cache.findDependency(clangDepName, ModuleDependencyKind::Clang)
              .value()
              ->getAsClangModule();
      if (importer::isCxxStdModule(clangDepName, clangDepInfo->IsSystem) &&
          !languageOverlayDependencies.contains(
              {clangDepName, ModuleDependencyKind::CodiraInterface}) &&
          !languageOverlayDependencies.contains(
              {clangDepName, ModuleDependencyKind::CodiraBinary})) {
        scanForCodiraDependency(
            getModuleImportIdentifier(ScanASTContext.Id_CxxStdlib.str()));
        recordResult(ScanASTContext.Id_CxxStdlib.str().str());
        break;
      }
    }
  }

  // Resolve the dependency info with Codira overlay dependency module
  // information.
  cache.setCodiraOverlayDependencies(moduleID,
                                    languageOverlayDependencies.getArrayRef());
}

void ModuleDependencyScanner::resolveCrossImportOverlayDependencies(
    StringRef mainModuleName, ModuleDependenciesCache &cache,
    toolchain::function_ref<void(ModuleDependencyID)> action) {
  // Modules explicitly imported. Only these can be secondary module.
  toolchain::SetVector<Identifier> newOverlays;
  std::set<std::pair<std::string, std::string>> overlayFiles;
  discoverCrossImportOverlayFiles(mainModuleName, cache, ScanASTContext,
                                  newOverlays, overlayFiles);

  // No new cross-import overlays are found, return.
  if (newOverlays.empty())
    return;

  // Construct a dummy main to resolve the newly discovered cross import
  // overlays.
  StringRef dummyMainName = "MainModuleCrossImportOverlays";
  auto dummyMainID = ModuleDependencyID{dummyMainName.str(),
                                        ModuleDependencyKind::CodiraSource};
  auto actualMainID = ModuleDependencyID{mainModuleName.str(),
                                         ModuleDependencyKind::CodiraSource};
  auto dummyMainDependencies = ModuleDependencyInfo::forCodiraSourceModule();
  std::for_each(
      newOverlays.begin(), newOverlays.end(), [&](Identifier modName) {
        dummyMainDependencies.addModuleImport(
            modName.str(),
            /* isExported */ false,
            // TODO: What is the right access level for a cross-import overlay?
            AccessLevel::Public);
      });

  // Record the dummy main module's direct dependencies. The dummy main module
  // only directly depend on these newly discovered overlay modules.
  if (cache.findDependency(dummyMainID))
    cache.updateDependency(dummyMainID, dummyMainDependencies);
  else
    cache.recordDependency(dummyMainName, dummyMainDependencies);

  ModuleDependencyIDSetVector allModules =
      resolveImportedModuleDependencies(dummyMainID, cache);

  // Update main module's dependencies to include these new overlays.
  auto newOverlayDeps = cache.getAllDependencies(dummyMainID);
  cache.setCrossImportOverlayDependencies(actualMainID,
                                          newOverlayDeps.getArrayRef());

  // Update the command-line on the main module to
  // disable implicit cross-import overlay search.
  auto mainDep = cache.findKnownDependency(actualMainID);
  std::vector<std::string> cmdCopy = mainDep.getCommandline();
  cmdCopy.push_back("-disable-cross-import-overlay-search");
  for (auto &entry : overlayFiles) {
    mainDep.addAuxiliaryFile(entry.second);
    cmdCopy.push_back("-language-module-cross-import");
    cmdCopy.push_back(entry.first);
    auto overlayPath = remapPath(entry.second);
    cmdCopy.push_back(overlayPath);
  }
  mainDep.updateCommandLine(cmdCopy);
  cache.updateDependency(actualMainID, mainDep);

  // Report any discovered modules to the clients, which include all overlays
  // and their dependencies.
  std::for_each(/* +1 to exclude dummy main*/ allModules.begin() + 1,
                allModules.end(), action);
}

toolchain::Error ModuleDependencyScanner::performBridgingHeaderChaining(
    const ModuleDependencyID &rootModuleID, ModuleDependenciesCache &cache,
    ModuleDependencyIDSetVector &allModules) {
  if (rootModuleID.Kind != ModuleDependencyKind::CodiraSource)
    return toolchain::Error::success();

  bool hasBridgingHeader = false;
  toolchain::vfs::OnDiskOutputBackend outputBackend;

  SmallString<256> outputPath(
      ScanCompilerInvocation.getFrontendOptions().ScannerOutputDir);

  if (outputPath.empty())
    outputPath = "/<compiler-generated>";

  toolchain::sys::path::append(
      outputPath, ScanCompilerInvocation.getFrontendOptions().ModuleName + "-" +
                      ScanCompilerInvocation.getModuleScanningHash() +
                      "-ChainedBridgingHeader.h");

  toolchain::SmallString<256> sourceBuf;
  toolchain::raw_svector_ostream outOS(sourceBuf);

  // Iterate through all the modules and collect all the bridging header
  // and chain them into a single file. The allModules list is in the order of
  // discover, thus providing stable ordering for a deterministic generated
  // buffer.
  auto FS = ScanASTContext.SourceMgr.getFileSystem();
  for (const auto &moduleID : allModules) {
    if (moduleID.Kind != ModuleDependencyKind::CodiraSource &&
        moduleID.Kind != ModuleDependencyKind::CodiraBinary)
      continue;

    auto moduleDependencyInfo = cache.findKnownDependency(moduleID);
    if (auto *binaryMod = moduleDependencyInfo.getAsCodiraBinaryModule()) {
      if (!binaryMod->headerImport.empty()) {
        hasBridgingHeader = true;
        if (FS->exists(binaryMod->headerImport)) {
          outOS << "#include \"" << binaryMod->headerImport << "\"\n";
        } else {
          // Extract the embedded bridging header
          auto moduleBuf = FS->getBufferForFile(binaryMod->compiledModulePath);
          if (!moduleBuf)
            return toolchain::errorCodeToError(moduleBuf.getError());

          auto content = extractEmbeddedBridgingHeaderContent(
              std::move(*moduleBuf), ScanASTContext);
          if (content.empty())
            return toolchain::createStringError("can't load embedded header from " +
                                           binaryMod->compiledModulePath);

          outOS << content << "\n";
        }
      }
    } else if (auto *srcMod = moduleDependencyInfo.getAsCodiraSourceModule()) {
      if (srcMod->textualModuleDetails.bridgingHeaderFile) {
        hasBridgingHeader = true;
        outOS << "#include \""
              << *srcMod->textualModuleDetails.bridgingHeaderFile << "\"\n";
      }
    }
  }

  if (!hasBridgingHeader)
    return toolchain::Error::success();

  if (ScanCompilerInvocation.getFrontendOptions().WriteScannerOutput) {
    auto outFile = outputBackend.createFile(outputPath);
    if (!outFile)
      return outFile.takeError();
    *outFile << sourceBuf;
    if (auto err = outFile->keep())
      return err;
  }

  auto sourceBuffer =
      toolchain::MemoryBuffer::getMemBufferCopy(sourceBuf, outputPath);
  // Scan and update the main module dependency.
  auto mainModuleDeps = cache.findKnownDependency(rootModuleID);
  ModuleDependencyIDSetVector headerClangModuleDependencies;
  std::optional<std::string> includeTreeID;
  auto err = withDependencyScanningWorker(
      [&](ModuleDependencyScanningWorker *ScanningWorker) -> toolchain::Error {
        std::vector<std::string> headerFileInputs;
        std::vector<std::string> bridgingHeaderCommandLine;
        if (ScanningWorker->scanHeaderDependenciesOfCodiraModule(
                *ScanningWorker->workerASTContext, rootModuleID,
                /*headerPath=*/std::nullopt, sourceBuffer->getMemBufferRef(),
                cache, headerClangModuleDependencies, headerFileInputs,
                bridgingHeaderCommandLine, includeTreeID))
          return toolchain::createStringError(
              "failed to scan generated bridging header " + outputPath);

        cache.setHeaderClangDependencies(
            rootModuleID, headerClangModuleDependencies.getArrayRef());
        // Record include Tree ID
        if (includeTreeID) {
          // Save the old include tree ID inside the CAS for lookup. Old include
          // tree can be used to create embedded header for the original
          // bridging header.
          if (auto embeddedHeaderIncludeTree =
                  mainModuleDeps.getBridgingHeaderIncludeTree()) {
            if (auto err = ScanningWorker->createCacheKeyForEmbeddedHeader(
                    *embeddedHeaderIncludeTree, *includeTreeID))
              return err;
          }
          mainModuleDeps.addBridgingHeaderIncludeTree(*includeTreeID);
        }
        mainModuleDeps.updateBridgingHeaderCommandLine(
            bridgingHeaderCommandLine);
        mainModuleDeps.setHeaderSourceFiles(headerFileInputs);
        mainModuleDeps.setChainedBridgingHeaderBuffer(
            outputPath, sourceBuffer->getBuffer());
        // Update the dependency in the cache
        cache.updateDependency(rootModuleID, mainModuleDeps);
        return toolchain::Error::success();
      });

  if (err)
    return err;

  cache.setHeaderClangDependencies(rootModuleID,
                                   headerClangModuleDependencies.getArrayRef());

  toolchain::for_each(
      headerClangModuleDependencies,
      [&allModules](const ModuleDependencyID &dep) { allModules.insert(dep); });

  return toolchain::Error::success();
}

void ModuleDependencyIssueReporter::diagnoseModuleNotFoundFailure(
    const ScannerImportStatementInfo &moduleImport,
    const ModuleDependenciesCache &cache,
    std::optional<ModuleDependencyID> dependencyOf,
    std::optional<std::pair<ModuleDependencyID, std::string>>
        resolvingSerializedSearchPath,
    std::optional<
        std::vector<CodiraModuleScannerQueryResult::IncompatibleCandidate>>
        foundIncompatibleCandidates) {
  // Do not report the same failure multiple times. This can
  // happen, for example, when we first report that a valid Codira module is
  // missing and then look it up again as a Codira overlay of its underlying
  // Clang module.
  if (ReportedMissing.find(moduleImport.importIdentifier) !=
      ReportedMissing.end())
    return;

  SourceLoc importLoc = SourceLoc();
  if (!moduleImport.importLocations.empty()) {
    auto locInfo = moduleImport.importLocations[0];
    importLoc = Diagnostics.SourceMgr.getLocFromExternalSource(
        locInfo.bufferIdentifier, locInfo.lineNumber, locInfo.columnNumber);
  }

  // Emit the top-level error diagnostic, which is one of 3 possibilities:
  // 1. Module dependency can not be found but could be resolved if using search
  // paths serialized into some binary Codira module dependency
  //
  // 2. Codira Module dependency can not be found but we did find binary Codira
  // module candidates for this module which are not compatible with current
  // compilation
  //
  // 3. All other generic "module not found" cases
  if (resolvingSerializedSearchPath) {
    Diagnostics.diagnose(
        importLoc,
        diag::dependency_scan_module_not_found_on_specified_search_paths,
        moduleImport.importIdentifier);
    Diagnostics.diagnose(importLoc, diag::inherited_search_path_resolves_module,
                         moduleImport.importIdentifier,
                         resolvingSerializedSearchPath->first.ModuleName,
                         resolvingSerializedSearchPath->second);
  } else if (foundIncompatibleCandidates) {
    Diagnostics.diagnose(
        importLoc, diag::dependency_scan_compatible_language_module_not_found,
        moduleImport.importIdentifier);
    for (const auto &candidate : *foundIncompatibleCandidates)
      Diagnostics.diagnose(importLoc,
                           diag::dependency_scan_incompatible_module_found,
                           candidate.path, candidate.incompatibilityReason);
  } else
    Diagnostics.diagnose(importLoc, diag::dependency_scan_module_not_found,
                         moduleImport.importIdentifier);

  // Emit notes for every link in the dependency chain from the root
  // module-under-scan to the module whose import failed to resolve.
  if (dependencyOf.has_value()) {
    auto path = findPathToDependency(dependencyOf.value(), cache);
    // We may fail to construct a path in some cases, such as a Codira overlay of
    // a Clang module dependnecy.
    if (path.empty())
      path = {dependencyOf.value()};

    for (auto it = path.rbegin(), end = path.rend(); it != end; ++it) {
      const auto &entry = *it;
      auto optionalEntryNode = cache.findDependency(entry);
      assert(optionalEntryNode.has_value());
      auto entryNode = optionalEntryNode.value();
      std::string moduleFilePath = "";
      bool isClang = false;
      switch (entryNode->getKind()) {
      case language::ModuleDependencyKind::CodiraSource:
        Diagnostics.diagnose(importLoc,
                             diag::dependency_as_imported_by_main_module,
                             entry.ModuleName);
        continue;
      case language::ModuleDependencyKind::CodiraInterface:
        moduleFilePath =
            entryNode->getAsCodiraInterfaceModule()->languageInterfaceFile;
        break;
      case language::ModuleDependencyKind::CodiraBinary:
        moduleFilePath =
            entryNode->getAsCodiraBinaryModule()->compiledModulePath;
        break;
      case language::ModuleDependencyKind::Clang:
        moduleFilePath = entryNode->getAsClangModule()->moduleMapFile;
        isClang = true;
        break;
      default:
        toolchain_unreachable("Unexpected dependency kind");
      }

      Diagnostics.diagnose(importLoc, diag::dependency_as_imported_by,
                           entry.ModuleName, moduleFilePath, isClang);
    }
  }

  // Emit notes for every other known location where the failed-to-resolve
  // module is imported.
  if (moduleImport.importLocations.size() > 1) {
    for (size_t i = 1; i < moduleImport.importLocations.size(); ++i) {
      auto locInfo = moduleImport.importLocations[i];
      auto importLoc = Diagnostics.SourceMgr.getLocFromExternalSource(
          locInfo.bufferIdentifier, locInfo.lineNumber, locInfo.columnNumber);
      Diagnostics.diagnose(importLoc, diag::unresolved_import_location);
    }
  }

  ReportedMissing.insert(moduleImport.importIdentifier);
}

void ModuleDependencyIssueReporter::diagnoseFailureOnOnlyIncompatibleCandidates(
    const ScannerImportStatementInfo &moduleImport,
    const std::vector<CodiraModuleScannerQueryResult::IncompatibleCandidate>
        &candidates,
    const ModuleDependenciesCache &cache,
    std::optional<ModuleDependencyID> dependencyOf) {
  // If no incompatible candidates were discovered,
  // the dependency scanning failure will be caught downstream.
  if (candidates.empty())
    return;

  diagnoseModuleNotFoundFailure(moduleImport, cache, dependencyOf,
                                /* resolvingSerializedSearchPath */ std::nullopt,
                                candidates);
}

void ModuleDependencyIssueReporter::warnOnIncompatibleCandidates(
    StringRef moduleName,
    const std::vector<CodiraModuleScannerQueryResult::IncompatibleCandidate>
        &candidates) {
  // If the dependency was ultimately resolved to a different
  // binary module or a textual module, emit warnings about
  // having encountered incompatible binary modules.
  for (const auto &candidate : candidates)
    Diagnostics.diagnose(SourceLoc(), diag::dependency_scan_module_incompatible,
                         candidate.path, candidate.incompatibilityReason);
}

std::optional<std::pair<ModuleDependencyID, std::string>>
ModuleDependencyScanner::attemptToFindResolvingSerializedSearchPath(
    const ScannerImportStatementInfo &moduleImport,
    const ModuleDependenciesCache &cache) {
  std::set<ModuleDependencyID> binaryCodiraModuleDepIDs =
      collectBinaryCodiraDeps(cache);

  std::optional<std::pair<ModuleDependencyID, std::string>> result;
  for (const auto &binaryDepID : binaryCodiraModuleDepIDs) {
    auto binaryModInfo =
        cache.findKnownDependency(binaryDepID).getAsCodiraBinaryModule();
    assert(binaryModInfo);
    if (binaryModInfo->serializedSearchPaths.empty())
      continue;

    // Note: this will permanently mutate this worker with additional search
    // paths. That's fine because we are diagnosing a scan failure here, but
    // worth being aware of.
    result = withDependencyScanningWorker(
        [this, &binaryModInfo, &moduleImport,
         &binaryDepID](ModuleDependencyScanningWorker *ScanningWorker)
            -> std::optional<std::pair<ModuleDependencyID, std::string>> {
          for (const auto &sp : binaryModInfo->serializedSearchPaths)
            ScanningWorker->workerASTContext->addSearchPath(
                sp.Path, sp.IsFramework, sp.IsSystem);

          auto importIdentifier =
              getModuleImportIdentifier(moduleImport.importIdentifier);
          CodiraModuleScannerQueryResult languageResult =
              ScanningWorker->scanFilesystemForCodiraModuleDependency(
                  importIdentifier, /* isTestableImport */ false);
          if (languageResult.foundDependencyInfo)
            return std::make_pair(
                binaryDepID,
                languageResult.foundDependencyInfo->getModuleDefiningPath());

          ModuleDependencyVector clangResult =
              ScanningWorker->scanFilesystemForClangModuleDependency(
                  importIdentifier, {});
          if (!clangResult.empty())
            return std::make_pair(
                binaryDepID, clangResult[0].second.getModuleDefiningPath());
          return std::nullopt;
        });
    if (result)
      break;
  }

  return result;
}
