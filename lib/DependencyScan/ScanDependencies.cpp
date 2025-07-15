//===--- ScanDependencies.cpp -- Scans the dependencies of a module -------===//
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

#include "language-c/DependencyScan/DependencyScan.h"
#include "language/AST/DiagnosticsCommon.h"
#include "language/Basic/PrettyStackTrace.h"

#include "language/AST/ASTContext.h"
#include "language/AST/Decl.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsDriver.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/DiagnosticsSema.h"
#include "language/AST/FileSystem.h"
#include "language/AST/Module.h"
#include "language/AST/ModuleDependencies.h"
#include "language/AST/ModuleLoader.h"
#include "language/AST/SourceFile.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/Basic/FileTypes.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/STLExtras.h"
#include "language/ClangImporter/ClangImporter.h"
#include "language/DependencyScan/DependencyScanImpl.h"
#include "language/DependencyScan/DependencyScanJSON.h"
#include "language/DependencyScan/DependencyScanningTool.h"
#include "language/DependencyScan/ModuleDependencyScanner.h"
#include "language/DependencyScan/ScanDependencies.h"
#include "language/DependencyScan/SerializedModuleDependencyCacheFormat.h"
#include "language/DependencyScan/StringUtils.h"
#include "language/Frontend/CachingUtils.h"
#include "language/Frontend/CompileJobCacheKey.h"
#include "language/Frontend/CompileJobCacheResult.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/FrontendOptions.h"
#include "language/Frontend/ModuleInterfaceLoader.h"
#include "language/Strings.h"
#include "clang/CAS/IncludeTree.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SetOperations.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/StringSet.h"
#include "toolchain/CAS/ActionCache.h"
#include "toolchain/CAS/CASReference.h"
#include "toolchain/CAS/ObjectStore.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/StringSaver.h"
#include "toolchain/Support/VirtualOutputBackend.h"
#include "toolchain/Support/YAMLParser.h"
#include "toolchain/Support/YAMLTraits.h"
#include "toolchain/Support/raw_ostream.h"
#include <algorithm>
#include <set>
#include <sstream>
#include <stack>
#include <string>

using namespace language;
using namespace language::dependencies;
using namespace language::c_string_utils;
using namespace toolchain::yaml;

namespace {

class ExplicitModuleDependencyResolver {
public:
  ExplicitModuleDependencyResolver(
      const ModuleDependencyID &moduleID,
      const ModuleDependencyScanner &scanner,
      ModuleDependenciesCache &cache, CompilerInstance &instance,
      std::optional<CodiraDependencyTracker> tracker)
      : moduleID(moduleID), scanner(scanner), cache(cache), instance(instance),
        resolvingDepInfo(cache.findKnownDependency(moduleID)),
        tracker(std::move(tracker)) {
    // Copy commandline.
    commandline = resolvingDepInfo.getCommandline();
  }

  // Resolve the dependencies for the current moduleID. Return true on error.
  bool resolve(const std::set<ModuleDependencyID> &dependencies,
               std::optional<std::set<ModuleDependencyID>> bridgingHeaderDeps) {
    // If the dependency is already finalized, nothing needs to be done.
    if (resolvingDepInfo.isFinalized())
      return false;

    for (const auto &depModuleID : dependencies) {
      const auto &depInfo = cache.findKnownDependency(depModuleID);
      switch (depModuleID.Kind) {
      case language::ModuleDependencyKind::CodiraInterface: {
        auto interfaceDepDetails = depInfo.getAsCodiraInterfaceModule();
        assert(interfaceDepDetails && "Expected Codira Interface dependency.");
        if (handleCodiraInterfaceModuleDependency(depModuleID,
                                                 *interfaceDepDetails))
          return true;
      } break;
      case language::ModuleDependencyKind::CodiraBinary: {
        auto binaryDepDetails = depInfo.getAsCodiraBinaryModule();
        assert(binaryDepDetails && "Expected Codira Binary Module dependency.");
        if (handleCodiraBinaryModuleDependency(depModuleID, *binaryDepDetails))
          return true;
      } break;
      case language::ModuleDependencyKind::Clang: {
        auto clangDepDetails = depInfo.getAsClangModule();
        assert(clangDepDetails && "Expected Clang Module dependency.");
        if (handleClangModuleDependency(depModuleID, *clangDepDetails))
          return true;
      } break;
      case language::ModuleDependencyKind::CodiraSource: {
        auto sourceDepDetails = depInfo.getAsCodiraSourceModule();
        assert(sourceDepDetails && "Expected Codira Source Module dependency.");
        if (handleCodiraSourceModuleDependency(depModuleID, *sourceDepDetails))
          return true;
      } break;
      default:
        toolchain_unreachable("Unhandled dependency kind.");
      }
    }

    // Update bridging header build command if there is a bridging header
    // dependency.
    if (addBridgingHeaderDeps(resolvingDepInfo))
      return true;

    if (bridgingHeaderDeps) {
      bridgingHeaderBuildCmd = resolvingDepInfo.getBridgingHeaderCommandline();
      for (auto bridgingDep : *bridgingHeaderDeps) {
        auto &dep = cache.findKnownDependency(bridgingDep);
        auto *clangDep = dep.getAsClangModule();
        assert(clangDep && "wrong module dependency kind");
        if (!clangDep->moduleCacheKey.empty()) {
          bridgingHeaderBuildCmd.push_back("-Xcc");
          bridgingHeaderBuildCmd.push_back("-fmodule-file-cache-key");
          bridgingHeaderBuildCmd.push_back("-Xcc");
          bridgingHeaderBuildCmd.push_back(clangDep->mappedPCMPath);
          bridgingHeaderBuildCmd.push_back("-Xcc");
          bridgingHeaderBuildCmd.push_back(clangDep->moduleCacheKey);
        }
      }
      addDeterministicCheckFlags(bridgingHeaderBuildCmd);
    }

    CodiraInterfaceModuleOutputPathResolution::ResultTy languageInterfaceOutputPath;
    if (resolvingDepInfo.isCodiraInterfaceModule()) {
      pruneUnusedVFSOverlay(languageInterfaceOutputPath);
      addCodiraInterfaceModuleOutputPathToCommandLine(languageInterfaceOutputPath);
    }

    // Update the dependency in the cache with the modified command-line.
    if (resolvingDepInfo.isCodiraInterfaceModule() ||
        resolvingDepInfo.isClangModule()) {
      if (scanner.hasPathMapping())
        commandline =
            remapPathsFromCommandLine(commandline, [&](StringRef path) {
              return scanner.remapPath(path);
            });
      addDeterministicCheckFlags(commandline);
    }

    auto dependencyInfoCopy = resolvingDepInfo;
    if (finalize(dependencyInfoCopy, languageInterfaceOutputPath))
      return true;

    dependencyInfoCopy.setIsFinalized(true);
    cache.updateDependency(moduleID, dependencyInfoCopy);
    return false;
  }

private:
  // Finalize the resolving dependency info.
  bool finalize(ModuleDependencyInfo &depInfo,
                const CodiraInterfaceModuleOutputPathResolution::ResultTy
                    &languageInterfaceModuleOutputPath) {
    if (resolvingDepInfo.isCodiraInterfaceModule())
      depInfo.setOutputPathAndHash(
          languageInterfaceModuleOutputPath.outputPath.str().str(),
          languageInterfaceModuleOutputPath.hash.str());

    // Add macros.
    for (auto &macro : macros)
      depInfo.addMacroDependency(macro.first(), macro.second.LibraryPath,
                                 macro.second.ExecutablePath);

    bool needPathRemapping = instance.getInvocation()
                                 .getSearchPathOptions()
                                 .ResolvedPluginVerification &&
                             scanner.hasPathMapping();
    auto mapPath = [&](StringRef path) {
      if (!needPathRemapping)
        return path.str();

      return scanner.remapPath(path);
    };
    if (needPathRemapping)
      commandline.push_back("-resolved-plugin-verification");

    for (auto &macro : depInfo.getMacroDependencies()) {
      std::string arg = mapPath(macro.second.LibraryPath) + "#" +
                        mapPath(macro.second.ExecutablePath) + "#" +
                        macro.first;
      commandline.push_back("-load-resolved-plugin");
      commandline.push_back(arg);
    }

    // Update CAS dependencies.
    if (auto err = collectCASDependencies(depInfo))
      return err;

    if (!bridgingHeaderBuildCmd.empty())
      depInfo.updateBridgingHeaderCommandLine(bridgingHeaderBuildCmd);
    if (!resolvingDepInfo.isCodiraBinaryModule()) {
      depInfo.updateCommandLine(commandline);
      if (updateModuleCacheKey(depInfo))
        return true;
    }

    return false;
  }

  bool handleCodiraInterfaceModuleDependency(
      ModuleDependencyID depModuleID,
      const CodiraInterfaceModuleDependenciesStorage &interfaceDepDetails) {
    if (!resolvingDepInfo.isCodiraSourceModule()) {
      auto &path = interfaceDepDetails.moduleCacheKey.empty()
                       ? interfaceDepDetails.moduleOutputPath
                       : interfaceDepDetails.moduleCacheKey;
      commandline.push_back("-language-module-file=" + depModuleID.ModuleName +
                            "=" + path);
    }
    addMacroDependencies(depModuleID, interfaceDepDetails);
    return false;
  }

  bool handleCodiraBinaryModuleDependency(
      ModuleDependencyID depModuleID,
      const CodiraBinaryModuleDependencyStorage &binaryDepDetails) {
    if (!resolvingDepInfo.isCodiraSourceModule()) {
      auto &path = binaryDepDetails.moduleCacheKey.empty()
                       ? binaryDepDetails.compiledModulePath
                       : binaryDepDetails.moduleCacheKey;
      commandline.push_back("-language-module-file=" + depModuleID.ModuleName +
                            "=" + path);
      // If this binary module was built with a header, the header's module
      // dependencies must also specify a .modulemap to the compilation, in
      // order to resolve the header's own header include directives.
      for (const auto &bridgingHeaderDepID :
           binaryDepDetails.headerModuleDependencies) {
        auto optionalBridgingHeaderDepModuleInfo =
            cache.findKnownDependency(bridgingHeaderDepID);
        const auto bridgingHeaderDepModuleDetails =
            optionalBridgingHeaderDepModuleInfo.getAsClangModule();
        commandline.push_back("-Xcc");
        commandline.push_back(
            "-fmodule-map-file=" +
            scanner.remapPath(bridgingHeaderDepModuleDetails->moduleMapFile));
      }
    }
    addMacroDependencies(depModuleID, binaryDepDetails);
    return false;
  }

  bool handleClangModuleDependency(
      ModuleDependencyID depModuleID,
      const ClangModuleDependencyStorage &clangDepDetails) {
    if (!resolvingDepInfo.isCodiraSourceModule()) {
      if (!resolvingDepInfo.isClangModule()) {
        commandline.push_back("-Xcc");
        commandline.push_back("-fmodule-file=" + depModuleID.ModuleName + "=" +
                              clangDepDetails.mappedPCMPath);
      }
      if (!clangDepDetails.moduleCacheKey.empty()) {
        commandline.push_back("-Xcc");
        commandline.push_back("-fmodule-file-cache-key");
        commandline.push_back("-Xcc");
        commandline.push_back(clangDepDetails.mappedPCMPath);
        commandline.push_back("-Xcc");
        commandline.push_back(clangDepDetails.moduleCacheKey);
      }
    }

    // Collect CAS deppendencies from clang modules.
    if (!clangDepDetails.CASClangIncludeTreeRootID.empty()) {
      if (addIncludeTree(clangDepDetails.CASClangIncludeTreeRootID))
        return true;
    }

    collectUsedVFSOverlay(clangDepDetails);

    return false;
  }

  bool handleCodiraSourceModuleDependency(
      ModuleDependencyID depModuleID,
      const CodiraSourceModuleDependenciesStorage &sourceDepDetails) {
    addMacroDependencies(depModuleID, sourceDepDetails);
    return addBridgingHeaderDeps(sourceDepDetails);
  }

  bool addBridgingHeaderDeps(const ModuleDependencyInfo &depInfo) {
    auto sourceDepDetails = depInfo.getAsCodiraSourceModule();
    if (!sourceDepDetails)
      return false;

    return addBridgingHeaderDeps(*sourceDepDetails);
  }

  bool addBridgingHeaderDeps(
      const CodiraSourceModuleDependenciesStorage &sourceDepDetails) {
    if (sourceDepDetails.textualModuleDetails.CASBridgingHeaderIncludeTreeRootID
            .empty()) {
      if (!sourceDepDetails.textualModuleDetails.bridgingSourceFiles.empty()) {
        if (tracker) {
          tracker->startTracking(/*includeCommonDeps*/ false);
          for (auto &file :
               sourceDepDetails.textualModuleDetails.bridgingSourceFiles)
            tracker->trackFile(file);
          auto bridgeRoot = tracker->createTreeFromDependencies();
          if (!bridgeRoot)
            return diagnoseCASFSCreationError(bridgeRoot.takeError());

          fileListRefs.push_back(bridgeRoot->getRef());
        }
      }
    } else if (addIncludeTree(sourceDepDetails.textualModuleDetails
                                  .CASBridgingHeaderIncludeTreeRootID))
      return true;

    return false;
  };

  void addMacroDependencies(ModuleDependencyID moduleID,
                            const ModuleDependencyInfoStorageBase &dep) {
    auto directDeps = cache.getAllDependencies(this->moduleID);
    if (toolchain::find(directDeps, moduleID) == directDeps.end())
      return;

    for (auto &entry : dep.macroDependencies)
      macros.insert({entry.first,
                     {entry.second.LibraryPath, entry.second.ExecutablePath}});
  }

  static bool isVFSOverlayFlag(StringRef arg) {
    return arg == "-ivfsoverlay" || arg == "-vfsoverlay";
  };
  static bool isXCCArg(StringRef arg) { return arg == "-Xcc"; };

  void
  collectUsedVFSOverlay(const ClangModuleDependencyStorage &clangDepDetails) {
    // true if the previous argument was the dash-option of an option pair
    bool getNext = false;
    for (const auto &A : clangDepDetails.buildCommandLine) {
      StringRef arg(A);
      if (isXCCArg(arg))
        continue;
      if (getNext) {
        getNext = false;
        usedVFSOverlayPaths.insert(arg);
      } else if (isVFSOverlayFlag(arg))
        getNext = true;
    }
  }

  void pruneUnusedVFSOverlay(
      CodiraInterfaceModuleOutputPathResolution::ResultTy &outputPath) {
    // Pruning of unused VFS overlay options for Clang dependencies is performed
    // by the Clang dependency scanner.
    if (moduleID.Kind == ModuleDependencyKind::Clang)
      return;

    // Prune the command line.
    std::vector<std::string> resolvedCommandLine;
    size_t skip = 0;
    for (auto it = commandline.begin(), end = commandline.end(); it != end;
         it++) {
      if (skip) {
        skip--;
        continue;
      }
      // If this VFS overlay was not used across any of the dependencies, skip
      // it.
      if ((it + 1) != end && isXCCArg(*it) && isVFSOverlayFlag(*(it + 1))) {
        assert(it + 2 != end); // Extra -Xcc
        assert(it + 3 != end); // Actual VFS overlay path argument
        if (!usedVFSOverlayPaths.contains(*(it + 3))) {
          skip = 3;
          continue;
        }
      }
      resolvedCommandLine.push_back(*it);
    }

    commandline = std::move(resolvedCommandLine);

    // Prune the clang impoter options. We do not need to deal with -Xcc because
    // these are clang options.
    const auto &CI = instance.getInvocation();

    CodiraInterfaceModuleOutputPathResolution::ArgListTy extraArgsList;
    const auto &clangImporterOptions =
        CI.getClangImporterOptions()
            .getReducedExtraArgsForCodiraModuleDependency();

    skip = 0;
    for (auto it = clangImporterOptions.begin(),
              end = clangImporterOptions.end();
         it != end; it++) {
      if (skip) {
        skip = 0;
        continue;
      }

      if ((it + 1) != end && isVFSOverlayFlag(*it)) {
        if (!usedVFSOverlayPaths.contains(*(it + 1))) {
          skip = 1;
          continue;
        }
      }

      extraArgsList.push_back(*it);
    }

    auto languageTextualDeps = resolvingDepInfo.getAsCodiraInterfaceModule();
    auto &interfacePath = languageTextualDeps->languageInterfaceFile;
    auto sdkPath = instance.getASTContext().SearchPathOpts.getSDKPath();
    CodiraInterfaceModuleOutputPathResolution::setOutputPath(
        outputPath, moduleID.ModuleName, interfacePath, sdkPath, CI,
        extraArgsList);

    return;
  }

  void addCodiraInterfaceModuleOutputPathToCommandLine(
      const CodiraInterfaceModuleOutputPathResolution::ResultTy &outputPath) {
    StringRef outputName = outputPath.outputPath.str();

    commandline.push_back("-o");
    commandline.push_back(outputName.str());

    return;
  }

  bool collectCASDependencies(ModuleDependencyInfo &dependencyInfoCopy) {
    if (!instance.getInvocation().getCASOptions().EnableCaching)
      return false;

    // Collect CAS info from current resolving module.
    if (auto *sourceDep = resolvingDepInfo.getAsCodiraSourceModule()) {
      tracker->startTracking();
      toolchain::for_each(sourceDep->sourceFiles, [this](const std::string &file) {
        tracker->trackFile(file);
      });
      toolchain::for_each(
          sourceDep->auxiliaryFiles,
          [this](const std::string &file) { tracker->trackFile(file); });
      toolchain::for_each(dependencyInfoCopy.getMacroDependencies(),
                     [this](const auto &entry) {
                       tracker->trackFile(entry.second.LibraryPath);
                     });
      auto root = tracker->createTreeFromDependencies();
      if (!root)
        return diagnoseCASFSCreationError(root.takeError());
      fileListRefs.push_back(root->getRef());
    } else if (auto *textualDep =
                   resolvingDepInfo.getAsCodiraInterfaceModule()) {
      tracker->startTracking();
      tracker->trackFile(textualDep->languageInterfaceFile);
      toolchain::for_each(
          textualDep->auxiliaryFiles,
          [this](const std::string &file) { tracker->trackFile(file); });
      toolchain::for_each(dependencyInfoCopy.getMacroDependencies(),
                     [this](const auto &entry) {
                       tracker->trackFile(entry.second.LibraryPath);
                     });
      auto root = tracker->createTreeFromDependencies();
      if (!root)
        return diagnoseCASFSCreationError(root.takeError());
      fileListRefs.push_back(root->getRef());
    }

    // Update build command line.
    if (resolvingDepInfo.isCodiraInterfaceModule() ||
        resolvingDepInfo.isCodiraSourceModule()) {
      // Update with casfs option.
      if (computeCASFileSystem(dependencyInfoCopy))
        return true;
    }

    // Compute and update module cache key.
    if (auto *binaryDep = dependencyInfoCopy.getAsCodiraBinaryModule()) {
      if (setupBinaryCacheKey(binaryDep->compiledModulePath,
                              dependencyInfoCopy))
        return true;
    }
    return false;
  }

  bool updateModuleCacheKey(ModuleDependencyInfo &depInfo) {
    if (!instance.getInvocation().getCASOptions().EnableCaching)
      return false;

    auto &CAS = scanner.getCAS();
    auto commandLine = depInfo.getCommandline();
    std::vector<const char *> Args;
    if (commandLine.size() > 1)
      for (auto &c : ArrayRef<std::string>(commandLine).drop_front(1))
        Args.push_back(c.c_str());

    auto base = createCompileJobBaseCacheKey(CAS, Args);
    if (!base) {
      instance.getDiags().diagnose(SourceLoc(), diag::error_cache_key_creation,
                                   moduleID.ModuleName,
                                   toString(base.takeError()));
      return true;
    }

    // Module compilation commands always have only one input and the input
    // index is always 0.
    auto key = createCompileJobCacheKeyForOutput(CAS, *base, /*InputIndex=*/0);
    if (!key) {
      instance.getDiags().diagnose(SourceLoc(), diag::error_cache_key_creation,
                                   moduleID.ModuleName,
                                   toString(key.takeError()));
      return true;
    }

    depInfo.updateModuleCacheKey(CAS.getID(*key).toString());
    return false;
  }

  bool setupBinaryCacheKey(StringRef path, ModuleDependencyInfo &depInfo) {
    auto &CASFS = scanner.getSharedCachingFS();
    auto &CAS = scanner.getCAS();
    // For binary module, we need to make sure the lookup key is setup here in
    // action cache. We just use the CASID of the binary module itself as key.
    auto Ref = CASFS.getObjectRefForFileContent(path);
    if (!Ref) {
      instance.getDiags().diagnose(SourceLoc(), diag::error_cas_file_ref, path);
      return true;
    }
    assert(*Ref && "Binary module should be loaded into CASFS already");
    depInfo.updateModuleCacheKey(CAS.getID(**Ref).toString());

    language::cas::CompileJobCacheResult::Builder Builder;
    Builder.addOutput(file_types::ID::TY_CodiraModuleFile, **Ref);
    auto Result = Builder.build(CAS);
    if (!Result) {
      instance.getDiags().diagnose(SourceLoc(), diag::error_cas,
                                   "adding binary module dependencies",
                                   toString(Result.takeError()));
      return true;
    }
    if (auto E = instance.getActionCache().put(CAS.getID(**Ref),
                                               CAS.getID(*Result))) {
      instance.getDiags().diagnose(
          SourceLoc(), diag::error_cas,
          "adding binary module dependencies cache entry",
          toString(std::move(E)));
      return true;
    }
    return false;
  }

  bool diagnoseCASFSCreationError(toolchain::Error err) {
    if (!err)
      return false;

    instance.getDiags().diagnose(SourceLoc(), diag::error_cas_fs_creation,
                                 toString(std::move(err)));
    return true;
  }

  void addDeterministicCheckFlags(std::vector<std::string> &cmd) {
    // Propagate the deterministic check to explicit built module command.
    if (!instance.getInvocation().getFrontendOptions().DeterministicCheck)
      return;
    cmd.push_back("-enable-deterministic-check");
    cmd.push_back("-always-compile-output-files");
    // disable cache replay because that defeat the purpose of the check.
    if (instance.getInvocation().getCASOptions().EnableCaching)
      cmd.push_back("-cache-disable-replay");
  }

  bool addIncludeTree(StringRef includeTree) {
    auto &db = scanner.getCAS();
    auto casID = db.parseID(includeTree);
    if (!casID) {
      instance.getDiags().diagnose(SourceLoc(), diag::error_invalid_cas_id,
                                   includeTree, toString(casID.takeError()));
      return true;
    }
    auto ref = db.getReference(*casID);
    if (!ref) {
      instance.getDiags().diagnose(SourceLoc(), diag::error_load_input_from_cas,
                                   includeTree);
      return true;
    }

    auto root = clang::cas::IncludeTreeRoot::get(db, *ref);
    if (!root) {
      instance.getDiags().diagnose(SourceLoc(), diag::error_cas_malformed_input,
                                   includeTree, toString(root.takeError()));
      return true;
    }

    fileListRefs.push_back(root->getFileListRef());
    return false;
  }

  bool computeCASFileSystem(ModuleDependencyInfo &dependencyInfoCopy) {
    if (fileListRefs.empty())
      return false;

    auto &db = scanner.getCAS();
    auto casFS =
        clang::cas::IncludeTree::FileList::create(db, {}, fileListRefs);
    if (!casFS) {
      instance.getDiags().diagnose(SourceLoc(), diag::error_cas,
                                   "CAS IncludeTree FileList creation",
                                   toString(casFS.takeError()));
      return true;
    }

    auto casID = casFS->getID().toString();
    dependencyInfoCopy.updateCASFileSystemRootID(casID);
    commandline.push_back("-clang-include-tree-filelist");
    commandline.push_back(casID);
    return false;
  }

private:
  const ModuleDependencyID &moduleID;
  const ModuleDependencyScanner &scanner;
  ModuleDependenciesCache &cache;
  CompilerInstance &instance;
  const ModuleDependencyInfo &resolvingDepInfo;

  std::optional<CodiraDependencyTracker> tracker;
  std::vector<toolchain::cas::ObjectRef> fileListRefs;
  std::vector<std::string> commandline;
  std::vector<std::string> bridgingHeaderBuildCmd;
  toolchain::StringMap<MacroPluginDependency> macros;
  toolchain::StringSet<> usedVFSOverlayPaths;
};

static bool resolveExplicitModuleInputs(
    const ModuleDependencyID &moduleID,
    const std::set<ModuleDependencyID> &dependencies,
    const ModuleDependencyScanner &scanner,
    ModuleDependenciesCache &cache, CompilerInstance &instance,
    std::optional<std::set<ModuleDependencyID>> bridgingHeaderDeps,
    std::optional<CodiraDependencyTracker> tracker) {
  ExplicitModuleDependencyResolver resolver(moduleID, scanner, cache, instance,
                                            std::move(tracker));
  return resolver.resolve(dependencies, bridgingHeaderDeps);
}

static bool writePrescanJSONToOutput(DiagnosticEngine &diags,
                                     toolchain::vfs::OutputBackend &backend,
                                     StringRef path,
                                     languagescan_import_set_t importSet) {
  return withOutputPath(diags, backend, path, [&](toolchain::raw_pwrite_stream &os) {
    writePrescanJSON(os, importSet);
    return false;
  });
}

static bool writeJSONToOutput(DiagnosticEngine &diags,
                              toolchain::vfs::OutputBackend &backend, StringRef path,
                              languagescan_dependency_graph_t dependencies) {
  return withOutputPath(diags, backend, path, [&](toolchain::raw_pwrite_stream &os) {
    writeJSON(os, dependencies);
    return false;
  });
}

static void
bridgeDependencyIDs(const ArrayRef<ModuleDependencyID> dependencies,
                    std::vector<std::string> &bridgedDependencyNames) {
  for (const auto &dep : dependencies) {
    std::string dependencyKindAndName;
    switch (dep.Kind) {
    case ModuleDependencyKind::CodiraInterface:
    case ModuleDependencyKind::CodiraSource:
      dependencyKindAndName = "languageTextual";
      break;
    case ModuleDependencyKind::CodiraBinary:
      dependencyKindAndName = "languageBinary";
      break;
    case ModuleDependencyKind::Clang:
      dependencyKindAndName = "clang";
      break;
    default:
      toolchain_unreachable("Unhandled dependency kind.");
    }
    dependencyKindAndName += ":";
    dependencyKindAndName += dep.ModuleName;
    bridgedDependencyNames.push_back(dependencyKindAndName);
  }
}

static languagescan_macro_dependency_set_t *createMacroDependencySet(
    const std::map<std::string, MacroPluginDependency> &macroDeps) {
  if (macroDeps.empty())
    return nullptr;

  languagescan_macro_dependency_set_t *set = new languagescan_macro_dependency_set_t;
  set->count = macroDeps.size();
  set->macro_dependencies = new languagescan_macro_dependency_t[set->count];
  unsigned SI = 0;
  for (auto &entry : macroDeps) {
    set->macro_dependencies[SI] = new languagescan_macro_dependency_s;
    set->macro_dependencies[SI]->module_name =
        create_clone(entry.first.c_str());
    set->macro_dependencies[SI]->library_path =
        create_clone(entry.second.LibraryPath.c_str());
    set->macro_dependencies[SI]->executable_path =
        create_clone(entry.second.ExecutablePath.c_str());
    ++SI;
  }
  return set;
}

static languagescan_dependency_graph_t generateFullDependencyGraph(
    const CompilerInstance &instance,
    const DependencyScanDiagnosticCollector *diagnosticCollector,
    const ModuleDependenciesCache &cache,
    const ArrayRef<ModuleDependencyID> allModules) {
  if (allModules.empty()) {
    return nullptr;
  }

  std::string mainModuleName = allModules.front().ModuleName;
  languagescan_dependency_set_t *dependencySet = new languagescan_dependency_set_t;
  dependencySet->count = allModules.size();
  dependencySet->modules =
      new languagescan_dependency_info_t[dependencySet->count];

  for (size_t i = 0; i < allModules.size(); ++i) {
    const auto &moduleID = allModules[i];
    auto &moduleDependencyInfo = cache.findKnownDependency(moduleID);
    // Collect all the required pieces to build a ModuleInfo
    auto languageTextualDeps = moduleDependencyInfo.getAsCodiraInterfaceModule();
    auto languageSourceDeps = moduleDependencyInfo.getAsCodiraSourceModule();
    auto languageBinaryDeps = moduleDependencyInfo.getAsCodiraBinaryModule();
    auto clangDeps = moduleDependencyInfo.getAsClangModule();

    // ModulePath
    const char *modulePathSuffix =
        moduleDependencyInfo.isCodiraModule() ? ".codemodule" : ".pcm";
    std::string modulePath;
    if (languageTextualDeps)
      modulePath = languageTextualDeps->moduleOutputPath;
    else if (languageBinaryDeps)
      modulePath = languageBinaryDeps->compiledModulePath;
    else if (clangDeps)
      modulePath = clangDeps->pcmOutputPath;
    else
      modulePath = moduleID.ModuleName + modulePathSuffix;

    // SourceFiles
    std::vector<std::string> sourceFiles;
    if (languageSourceDeps) {
      sourceFiles = languageSourceDeps->sourceFiles;
    } else if (clangDeps) {
      sourceFiles = clangDeps->fileDependencies;
    }

    auto directDependencies = cache.getAllDependencies(moduleID);
    std::vector<std::string> clangHeaderDependencyNames;
    for (const auto &headerDepID :
         moduleDependencyInfo.getHeaderClangDependencies())
      clangHeaderDependencyNames.push_back(headerDepID.ModuleName);

    // Generate a languagescan_clang_details_t object based on the dependency kind
    auto getModuleDetails = [&]() -> languagescan_module_details_t {
      languagescan_module_details_s *details = new languagescan_module_details_s;
      if (languageTextualDeps) {
        languagescan_string_ref_t moduleInterfacePath =
            create_clone(languageTextualDeps->languageInterfaceFile.c_str());
        languagescan_string_ref_t bridgingHeaderPath =
            languageTextualDeps->textualModuleDetails.bridgingHeaderFile
                    .has_value()
                ? create_clone(
                      languageTextualDeps->textualModuleDetails.bridgingHeaderFile
                          .value()
                          .c_str())
                : create_null();
        details->kind = LANGUAGESCAN_DEPENDENCY_INFO_LANGUAGE_TEXTUAL;
        // Create an overlay dependencies set according to the output format
        std::vector<std::string> bridgedOverlayDependencyNames;
        bridgeDependencyIDs(languageTextualDeps->languageOverlayDependencies,
                            bridgedOverlayDependencyNames);

        details->language_textual_details = {
            moduleInterfacePath,
            create_set(languageTextualDeps->compiledModuleCandidates),
            bridgingHeaderPath,
            create_set(
                languageTextualDeps->textualModuleDetails.bridgingSourceFiles),
            create_set(clangHeaderDependencyNames),
            create_set(bridgedOverlayDependencyNames),
            /*sourceImportedDependencies*/ create_set({}),
            create_set(languageTextualDeps->textualModuleDetails.buildCommandLine),
            /*bridgingHeaderBuildCommand*/ create_set({}),
            create_clone(languageTextualDeps->contextHash.c_str()),
            languageTextualDeps->isFramework,
            languageTextualDeps->isStatic,
            create_clone(languageTextualDeps->textualModuleDetails
                             .CASFileSystemRootID.c_str()),
            create_clone(languageTextualDeps->textualModuleDetails
                             .CASBridgingHeaderIncludeTreeRootID.c_str()),
            create_clone(languageTextualDeps->moduleCacheKey.c_str()),
            createMacroDependencySet(languageTextualDeps->macroDependencies),
            create_clone(languageTextualDeps->userModuleVersion.c_str()),
            /*chained_bridging_header_path=*/create_clone(""),
            /*chained_bridging_header_content=*/create_clone("")};
      } else if (languageSourceDeps) {
        languagescan_string_ref_t moduleInterfacePath = create_null();
        languagescan_string_ref_t bridgingHeaderPath =
            languageSourceDeps->textualModuleDetails.bridgingHeaderFile.has_value()
                ? create_clone(
                      languageSourceDeps->textualModuleDetails.bridgingHeaderFile
                          .value()
                          .c_str())
                : create_null();
        details->kind = LANGUAGESCAN_DEPENDENCY_INFO_LANGUAGE_TEXTUAL;
        // Create an overlay dependencies set according to the output format
        std::vector<std::string> bridgedOverlayDependencyNames;
        bridgeDependencyIDs(languageSourceDeps->languageOverlayDependencies,
                            bridgedOverlayDependencyNames);

        // Create a set of directly-source-imported dependencies
        std::vector<ModuleDependencyID> sourceImportDependencies;
        std::copy(languageSourceDeps->importedCodiraModules.begin(),
                  languageSourceDeps->importedCodiraModules.end(),
                  std::back_inserter(sourceImportDependencies));
        std::copy(languageSourceDeps->importedClangModules.begin(),
                  languageSourceDeps->importedClangModules.end(),
                  std::back_inserter(sourceImportDependencies));
        std::vector<std::string> bridgedSourceImportedDependencyNames;
        bridgeDependencyIDs(sourceImportDependencies,
                            bridgedSourceImportedDependencyNames);

        details->language_textual_details = {
            moduleInterfacePath, create_empty_set(), bridgingHeaderPath,
            create_set(
                languageSourceDeps->textualModuleDetails.bridgingSourceFiles),
            create_set(clangHeaderDependencyNames),
            create_set(bridgedOverlayDependencyNames),
            create_set(bridgedSourceImportedDependencyNames),
            create_set(languageSourceDeps->textualModuleDetails.buildCommandLine),
            create_set(languageSourceDeps->bridgingHeaderBuildCommandLine),
            /*contextHash*/
            create_clone(
                instance.getInvocation().getModuleScanningHash().c_str()),
            /*isFramework*/ false,
            /*isStatic*/ false,
            /*CASFS*/
            create_clone(languageSourceDeps->textualModuleDetails
                             .CASFileSystemRootID.c_str()),
            /*IncludeTree*/
            create_clone(languageSourceDeps->textualModuleDetails
                             .CASBridgingHeaderIncludeTreeRootID.c_str()),
            /*CacheKey*/ create_clone(""),
            createMacroDependencySet(languageSourceDeps->macroDependencies),
            /*userModuleVersion*/ create_clone(""),
            create_clone(languageSourceDeps->chainedBridgingHeaderPath.c_str()),
            create_clone(
                languageSourceDeps->chainedBridgingHeaderContent.c_str())};
      } else if (languageBinaryDeps) {
        details->kind = LANGUAGESCAN_DEPENDENCY_INFO_LANGUAGE_BINARY;
        // Create an overlay dependencies set according to the output format
        std::vector<std::string> bridgedOverlayDependencyNames;
        bridgeDependencyIDs(languageBinaryDeps->languageOverlayDependencies,
                            bridgedOverlayDependencyNames);
        details->language_binary_details = {
            create_clone(languageBinaryDeps->compiledModulePath.c_str()),
            create_clone(languageBinaryDeps->moduleDocPath.c_str()),
            create_clone(languageBinaryDeps->sourceInfoPath.c_str()),
            create_set(bridgedOverlayDependencyNames),
            create_clone(languageBinaryDeps->headerImport.c_str()),
            create_set(clangHeaderDependencyNames),
            create_set(languageBinaryDeps->headerSourceFiles),
            languageBinaryDeps->isFramework,
            languageBinaryDeps->isStatic,
            createMacroDependencySet(languageBinaryDeps->macroDependencies),
            create_clone(languageBinaryDeps->moduleCacheKey.c_str()),
            create_clone(languageBinaryDeps->userModuleVersion.c_str())};
      } else {
        // Clang module details
        details->kind = LANGUAGESCAN_DEPENDENCY_INFO_CLANG;
        details->clang_details = {
            create_clone(clangDeps->moduleMapFile.c_str()),
            create_clone(clangDeps->contextHash.c_str()),
            create_set(clangDeps->buildCommandLine),
            create_clone(clangDeps->CASFileSystemRootID.c_str()),
            create_clone(clangDeps->CASClangIncludeTreeRootID.c_str()),
            create_clone(clangDeps->moduleCacheKey.c_str())};
      }
      return details;
    };

    languagescan_dependency_info_s *moduleInfo = new languagescan_dependency_info_s;
    dependencySet->modules[i] = moduleInfo;

    std::string encodedModuleName = createEncodedModuleKindAndName(moduleID);
    auto ttt = create_clone(encodedModuleName.c_str());
    moduleInfo->module_name = ttt;
    moduleInfo->module_path = create_clone(modulePath.c_str());
    moduleInfo->source_files = create_set(sourceFiles);

    // Create a direct dependencies set according to the output format
    std::vector<std::string> bridgedDependencyNames;
    bridgeDependencyIDs(directDependencies.getArrayRef(),
                        bridgedDependencyNames);
    moduleInfo->direct_dependencies = create_set(bridgedDependencyNames);
    moduleInfo->details = getModuleDetails();

    // Create a link libraries set for this module
    auto linkLibraries = moduleDependencyInfo.getLinkLibraries();
    languagescan_link_library_set_t *linkLibrarySet =
        new languagescan_link_library_set_t;
    linkLibrarySet->count = linkLibraries.size();
    linkLibrarySet->link_libraries =
        new languagescan_link_library_info_t[linkLibrarySet->count];
    for (size_t i = 0; i < linkLibraries.size(); ++i) {
      const auto &ll = linkLibraries[i];
      languagescan_link_library_info_s *llInfo = new languagescan_link_library_info_s;
      llInfo->name = create_clone(ll.getName().str().c_str());
      llInfo->isStatic = ll.isStaticLibrary();
      llInfo->isFramework = ll.getKind() == LibraryKind::Framework;
      llInfo->forceLoad = ll.shouldForceLoad();
      linkLibrarySet->link_libraries[i] = llInfo;
    }
    moduleInfo->link_libraries = linkLibrarySet;

    // Create source import infos set for this module
    auto imports = moduleDependencyInfo.getModuleImports();
    languagescan_import_info_set_t *importInfoSet =
        new languagescan_import_info_set_t;
    importInfoSet->count = imports.size();
    importInfoSet->imports = new languagescan_import_info_t[importInfoSet->count];
    for (size_t i = 0; i < imports.size(); ++i) {
      const auto &ii = imports[i];
      languagescan_import_info_s *iInfo = new languagescan_import_info_s;
      iInfo->import_identifier = create_clone(ii.importIdentifier.c_str());
      iInfo->access_level =
          static_cast<languagescan_access_level_t>(ii.accessLevel);

      const auto &sourceLocations = ii.importLocations;
      languagescan_source_location_set_t *sourceLocSet =
          new languagescan_source_location_set_t;
      sourceLocSet->count = sourceLocations.size();
      sourceLocSet->source_locations =
          new languagescan_source_location_t[sourceLocSet->count];
      for (size_t j = 0; j < sourceLocations.size(); ++j) {
        const auto &sl = sourceLocations[j];
        languagescan_source_location_s *slInfo = new languagescan_source_location_s;
        slInfo->buffer_identifier = create_clone(sl.bufferIdentifier.c_str());
        slInfo->line_number = sl.lineNumber;
        slInfo->column_number = sl.columnNumber;
        sourceLocSet->source_locations[j] = slInfo;
      }
      iInfo->source_locations = sourceLocSet;
      importInfoSet->imports[i] = iInfo;
    }
    moduleInfo->imports = importInfoSet;
  }

  languagescan_dependency_graph_t result = new languagescan_dependency_graph_s;
  result->main_module_name = create_clone(mainModuleName.c_str());
  result->dependencies = dependencySet;
  result->diagnostics =
      diagnosticCollector
          ? mapCollectedDiagnosticsForOutput(diagnosticCollector)
          : nullptr;
  return result;
}

/// Implements a topological sort via recursion and reverse postorder DFS.
/// Does not bother handling cycles, relying on a DAG guarantee by the client.
static std::vector<ModuleDependencyID>
computeTopologicalSortOfExplicitDependencies(
    const std::vector<ModuleDependencyID> &allModules,
    const ModuleDependenciesCache &cache) {
  std::unordered_set<ModuleDependencyID> visited;
  std::vector<ModuleDependencyID> result;
  std::stack<ModuleDependencyID> stack;

  // Must be explicitly-typed to allow recursion
  std::function<void(const ModuleDependencyID &)> visit;
  visit = [&visit, &cache, &visited, &result,
           &stack](const ModuleDependencyID &moduleID) {
    // Mark this node as visited -- we are done if it already was.
    if (!visited.insert(moduleID).second)
      return;

    // Otherwise, visit each adjacent node.
    for (const auto &succID : cache.getAllDependencies(moduleID)) {
      // We don't worry if successor is already in this current stack,
      // since that would mean we have found a cycle, which should not
      // be possible because we checked for cycles earlier.
      stack.push(succID);
      visit(succID);
      auto top = stack.top();
      stack.pop();
      assert(top == succID);
    }

    // Add to the result.
    result.push_back(moduleID);
  };

  for (const auto &modID : allModules) {
    assert(stack.empty());
    stack.push(modID);
    visit(modID);
    auto top = stack.top();
    stack.pop();
    assert(top == modID);
  }

  std::reverse(result.begin(), result.end());
  return result;
}

/// For each module in the graph, compute a set of all its dependencies,
/// direct *and* transitive.
static std::unordered_map<ModuleDependencyID, std::set<ModuleDependencyID>>
computeTransitiveClosureOfExplicitDependencies(
    const std::vector<ModuleDependencyID> &topologicallySortedModuleList,
    const ModuleDependenciesCache &cache) {
  // The usage of an ordered ::set is important to ensure the
  // dependencies are listed in a deterministic order.
  std::unordered_map<ModuleDependencyID, std::set<ModuleDependencyID>> result;
  for (const auto &modID : topologicallySortedModuleList)
    result[modID] = {modID};

  // Traverse the set of modules in reverse topological order, assimilating
  // transitive closures
  for (auto it = topologicallySortedModuleList.rbegin(),
            end = topologicallySortedModuleList.rend();
       it != end; ++it) {
    const auto &modID = *it;
    auto &modReachableSet = result[modID];
    for (const auto &succID : cache.getAllDependencies(modID)) {
      const auto &succReachableSet = result[succID];
      toolchain::set_union(modReachableSet, succReachableSet);
    }
  }
  // For ease of use down-the-line, remove the node's self from its set of
  // reachable nodes
  for (const auto &modID : topologicallySortedModuleList)
    result[modID].erase(modID);

  return result;
}

static std::set<ModuleDependencyID> computeBridgingHeaderTransitiveDependencies(
    const ModuleDependencyInfo &dep,
    const std::unordered_map<ModuleDependencyID, std::set<ModuleDependencyID>>
        &transitiveClosures,
    const ModuleDependenciesCache &cache) {
  std::set<ModuleDependencyID> result;
  if (!dep.isCodiraSourceModule())
    return result;

  for (auto &depID : dep.getHeaderClangDependencies()) {
    result.insert(depID);
    auto succDeps = transitiveClosures.find(depID);
    assert(succDeps != transitiveClosures.end() && "unknown dependency");
    toolchain::set_union(result, succDeps->second);
  }

  return result;
}

static std::vector<ModuleDependencyID>
findClangDepPath(const ModuleDependencyID &from, const ModuleDependencyID &to,
                 const ModuleDependenciesCache &cache) {
  std::unordered_set<ModuleDependencyID> visited;
  std::vector<ModuleDependencyID> result;
  std::stack<ModuleDependencyID, std::vector<ModuleDependencyID>> stack;

  // Must be explicitly-typed to allow recursion
  std::function<void(const ModuleDependencyID &)> visit;

  visit = [&visit, &cache, &visited, &result, &stack,
           to](const ModuleDependencyID &moduleID) {
    if (!visited.insert(moduleID).second)
      return;

    if (moduleID == to) {
      // Copy stack contents to the result
      auto end = &stack.top() + 1;
      auto begin = end - stack.size();
      result.assign(begin, end);
      return;
    }

    // Otherwise, visit each child node.
    for (const auto &succID : cache.getImportedClangDependencies(moduleID)) {
      stack.push(succID);
      visit(succID);
      stack.pop();
    }
  };

  stack.push(from);
  visit(from);
  return result;
}

static bool diagnoseCycle(const CompilerInstance &instance,
                          const ModuleDependenciesCache &cache,
                          ModuleDependencyID mainId) {
  ModuleDependencyIDSetVector openSet;
  ModuleDependencyIDSetVector closeSet;

  auto kindIsCodiraDependency = [&](const ModuleDependencyID &ID) {
    return ID.Kind == language::ModuleDependencyKind::CodiraInterface ||
           ID.Kind == language::ModuleDependencyKind::CodiraBinary ||
           ID.Kind == language::ModuleDependencyKind::CodiraSource;
  };

  auto emitModulePath = [&](const std::vector<ModuleDependencyID> path,
                            toolchain::SmallString<64> &buffer) {
    toolchain::interleave(
        path,
        [&buffer](const ModuleDependencyID &id) {
          buffer.append(id.ModuleName);
          switch (id.Kind) {
          case language::ModuleDependencyKind::CodiraSource:
            buffer.append(" (Source Target)");
            break;
          case language::ModuleDependencyKind::CodiraInterface:
            buffer.append(".codeinterface");
            break;
          case language::ModuleDependencyKind::CodiraBinary:
            buffer.append(".codemodule");
            break;
          case language::ModuleDependencyKind::Clang:
            buffer.append(".pcm");
            break;
          default:
            toolchain::report_fatal_error(
                Twine("Invalid Module Dependency Kind in cycle: ") +
                id.ModuleName);
            break;
          }
        },
        [&buffer] { buffer.append(" -> "); });
  };

  auto emitCycleDiagnostic = [&](const ModuleDependencyID &sourceId,
                                 const ModuleDependencyID &sinkId) {
    auto startIt = std::find(openSet.begin(), openSet.end(), sourceId);
    assert(startIt != openSet.end());
    std::vector<ModuleDependencyID> cycleNodes(startIt, openSet.end());
    cycleNodes.push_back(sinkId);
    toolchain::SmallString<64> errorBuffer;
    emitModulePath(cycleNodes, errorBuffer);
    instance.getASTContext().Diags.diagnose(
        SourceLoc(), diag::scanner_find_cycle, errorBuffer.str());

    // TODO: for (std::tuple<const ModuleDependencyID&, const
    // ModuleDependencyID&> v : cycleNodes | std::views::adjacent<2>)
    for (auto it = cycleNodes.begin(), end = cycleNodes.end(); it != end;
         it++) {
      if (it + 1 == cycleNodes.end())
        continue;

      const auto &thisID = *it;
      const auto &nextID = *(it + 1);
      if (kindIsCodiraDependency(thisID) && kindIsCodiraDependency(nextID) &&
          toolchain::any_of(
              cache.getCodiraOverlayDependencies(thisID),
              [&](const ModuleDependencyID id) { return id == nextID; })) {
        toolchain::SmallString<64> noteBuffer;
        auto clangDepPath = findClangDepPath(
            thisID,
            ModuleDependencyID{nextID.ModuleName, ModuleDependencyKind::Clang},
            cache);
        emitModulePath(clangDepPath, noteBuffer);
        instance.getASTContext().Diags.diagnose(
            SourceLoc(), diag::scanner_find_cycle_language_overlay_path,
            thisID.ModuleName, nextID.ModuleName, noteBuffer.str());
      }
    }

    // Check if this is a case of a source target shadowing
    // a module with the same name
    if (sourceId.Kind == language::ModuleDependencyKind::CodiraSource) {
      auto sinkModuleDefiningPath =
          cache.findKnownDependency(sinkId).getModuleDefiningPath();
      auto SDKPath =
          instance.getInvocation().getSearchPathOptions().getSDKPath();
      auto sinkIsInSDK =
          !SDKPath.empty() &&
          hasPrefix(toolchain::sys::path::begin(sinkModuleDefiningPath),
                    toolchain::sys::path::end(sinkModuleDefiningPath),
                    toolchain::sys::path::begin(SDKPath),
                    toolchain::sys::path::end(SDKPath));
      instance.getASTContext().Diags.diagnose(
          SourceLoc(), diag::scanner_cycle_source_target_shadow_module,
          sourceId.ModuleName, sinkModuleDefiningPath, sinkIsInSDK);
    }
  };

  // Start from the main module and check direct and overlay dependencies
  openSet.insert(mainId);
  while (!openSet.empty()) {
    auto lastOpen = openSet.back();
    auto beforeSize = openSet.size();
    assert(cache.findDependency(lastOpen).has_value() &&
           "Missing dependency info during cycle diagnosis.");
    for (const auto &depId : cache.getAllDependencies(lastOpen)) {
      if (closeSet.count(depId))
        continue;
      // Ensure we detect dependency of the Source target
      // on an existing Codira module with the same name
      if (kindIsCodiraDependency(depId) &&
          depId.ModuleName == mainId.ModuleName && openSet.contains(mainId)) {
        emitCycleDiagnostic(mainId, depId);
        return true;
      }
      if (openSet.insert(depId)) {
        break;
      } else {
        emitCycleDiagnostic(depId, depId);
        return true;
      }
    }
    // No new node added. We can close this node
    if (openSet.size() == beforeSize) {
      closeSet.insert(openSet.back());
      openSet.pop_back();
    } else {
      assert(openSet.size() == beforeSize + 1);
    }
  }
  assert(openSet.empty());
  closeSet.clear();
  return false;
}
} // namespace

bool language::dependencies::scanDependencies(CompilerInstance &CI) {
  ASTContext &ctx = CI.getASTContext();
  std::string depGraphOutputPath =
      CI.getInvocation()
          .getFrontendOptions()
          .InputsAndOutputs.getSingleOutputFilename();
  // `-scan-dependencies` invocations use a single new instance
  // of a module cache
  CodiraDependencyScanningService *service =
      ctx.Allocate<CodiraDependencyScanningService>();
  ModuleDependenciesCache cache(CI.getMainModule()->getNameStr().str(),
                                CI.getInvocation().getModuleScanningHash());

  if (service->setupCachingDependencyScanningService(CI))
    return true;

  // Execute scan
  toolchain::ErrorOr<languagescan_dependency_graph_t> dependenciesOrErr =
      performModuleScan(*service, CI, cache);

  if (dependenciesOrErr.getError())
    return true;
  auto dependencies = std::move(*dependenciesOrErr);

  if (writeJSONToOutput(ctx.Diags, CI.getOutputBackend(), depGraphOutputPath,
                        dependencies))
    return true;

  // This process succeeds regardless of whether any errors occurred.
  // FIXME: We shouldn't need this, but it's masking bugs in our scanning
  // logic where we don't create a fresh context when scanning Codira interfaces
  // that includes their own command-line flags.
  ctx.Diags.resetHadAnyError();
  return false;
}

bool language::dependencies::prescanDependencies(CompilerInstance &instance) {
  ASTContext &Context = instance.getASTContext();
  const FrontendOptions &opts = instance.getInvocation().getFrontendOptions();
  std::string path = opts.InputsAndOutputs.getSingleOutputFilename();
  // `-scan-dependencies` invocations use a single new instance
  // of a module cache
  CodiraDependencyScanningService *singleUseService =
      Context.Allocate<CodiraDependencyScanningService>();
  ModuleDependenciesCache cache(
      instance.getMainModule()->getNameStr().str(),
      instance.getInvocation().getModuleScanningHash());

  // Execute import prescan, and write JSON output to the output stream
  auto importSetOrErr =
      performModulePrescan(*singleUseService, instance, cache);
  if (importSetOrErr.getError())
    return true;
  auto importSet = std::move(*importSetOrErr);

  // Serialize and output main module dependencies only and exit.
  if (writePrescanJSONToOutput(Context.Diags, instance.getOutputBackend(), path,
                               importSet))
    return true;

  // This process succeeds regardless of whether any errors occurred.
  // FIXME: We shouldn't need this, but it's masking bugs in our scanning
  // logic where we don't create a fresh context when scanning Codira interfaces
  // that includes their own command-line flags.
  Context.Diags.resetHadAnyError();
  return false;
}

std::string
language::dependencies::createEncodedModuleKindAndName(ModuleDependencyID id) {
  switch (id.Kind) {
  case ModuleDependencyKind::CodiraInterface:
  case ModuleDependencyKind::CodiraSource:
    return "languageTextual:" + id.ModuleName;
  case ModuleDependencyKind::CodiraBinary:
    return "languageBinary:" + id.ModuleName;
  case ModuleDependencyKind::Clang:
    return "clang:" + id.ModuleName;
  default:
    toolchain_unreachable("Unhandled dependency kind.");
  }
}

static bool resolveDependencyCommandLineArguments(
    ModuleDependencyScanner &scanner, CompilerInstance &instance,
    ModuleDependenciesCache &cache,
    const std::vector<ModuleDependencyID> &topoSortedModuleList) {
  auto moduleTransitiveClosures =
      computeTransitiveClosureOfExplicitDependencies(topoSortedModuleList,
                                                     cache);
  auto tracker = scanner.createCodiraDependencyTracker(instance.getInvocation());
  for (const auto &modID : toolchain::reverse(topoSortedModuleList)) {
    auto dependencyClosure = moduleTransitiveClosures[modID];
    // For main module or binary modules, no command-line to resolve.
    // For Clang modules, their dependencies are resolved by the clang Scanner
    // itself for us.
    auto &deps = cache.findKnownDependency(modID);
    std::optional<std::set<ModuleDependencyID>> bridgingHeaderDeps;
    if (modID.Kind == ModuleDependencyKind::CodiraSource)
      bridgingHeaderDeps = computeBridgingHeaderTransitiveDependencies(
          deps, moduleTransitiveClosures, cache);

    if (resolveExplicitModuleInputs(modID, dependencyClosure, scanner, cache,
                                    instance, bridgingHeaderDeps, tracker))
      return true;
  }

  return false;
}

static void
updateDependencyTracker(CompilerInstance &instance,
                        ModuleDependenciesCache &cache,
                        const std::vector<ModuleDependencyID> &allModules) {
  if (auto depTracker = instance.getDependencyTracker()) {
    for (auto module : allModules) {
      auto optionalDeps = cache.findDependency(module);
      if (!optionalDeps.has_value())
        continue;
      auto deps = optionalDeps.value();

      if (auto languageDeps = deps->getAsCodiraInterfaceModule()) {
        depTracker->addDependency(languageDeps->languageInterfaceFile,
                                  /*IsSystem=*/false);
        for (const auto &bridgingSourceFile :
             languageDeps->textualModuleDetails.bridgingSourceFiles)
          depTracker->addDependency(bridgingSourceFile, /*IsSystem=*/false);
      } else if (auto languageSourceDeps = deps->getAsCodiraSourceModule()) {
        for (const auto &sourceFile : languageSourceDeps->sourceFiles)
          depTracker->addDependency(sourceFile, /*IsSystem=*/false);
        for (const auto &bridgingSourceFile :
             languageSourceDeps->textualModuleDetails.bridgingSourceFiles)
          depTracker->addDependency(bridgingSourceFile, /*IsSystem=*/false);
      } else if (auto clangDeps = deps->getAsClangModule()) {
        if (!clangDeps->moduleMapFile.empty())
          depTracker->addDependency(clangDeps->moduleMapFile,
                                    /*IsSystem=*/false);
        for (const auto &sourceFile : clangDeps->fileDependencies)
          depTracker->addDependency(sourceFile, /*IsSystem=*/false);
      }
    }
  }
}

static void resolveImplicitLinkLibraries(const CompilerInstance &instance,
                                         ModuleDependenciesCache &cache) {
  auto langOpts = instance.getInvocation().getLangOptions();
  auto irGenOpts = instance.getInvocation().getIRGenOptions();
  auto mainModuleName = instance.getMainModule()->getNameStr();
  auto mainModuleID = ModuleDependencyID{mainModuleName.str(),
                                         ModuleDependencyKind::CodiraSource};
  auto mainModuleDepInfo = cache.findKnownDependency(mainModuleID);

  std::vector<LinkLibrary> linkLibraries;
  auto addLinkLibrary = [&linkLibraries](const LinkLibrary &ll) {
    linkLibraries.push_back(ll);
  };

  if (langOpts.EnableObjCInterop)
    addLinkLibrary(LinkLibrary{"objc", LibraryKind::Library, /*static=*/false});

  if (langOpts.EnableCXXInterop) {
    auto OptionalCxxDep = cache.findDependency(CXX_MODULE_NAME);
    auto OptionalCxxStdLibDep =
        cache.findDependency(instance.getASTContext().Id_CxxStdlib.str());
    bool hasStaticCxx =
        OptionalCxxDep.has_value() && OptionalCxxDep.value()->isStaticLibrary();
    bool hasStaticCxxStdlib = OptionalCxxStdLibDep.has_value() &&
                              OptionalCxxStdLibDep.value()->isStaticLibrary();
    registerCxxInteropLibraries(langOpts.Target, mainModuleName, hasStaticCxx,
                                hasStaticCxxStdlib, langOpts.CXXStdlib,
                                addLinkLibrary);
  }

  if (!irGenOpts.UseJIT && !langOpts.hasFeature(Feature::Embedded))
    registerBackDeployLibraries(irGenOpts, addLinkLibrary);

  mainModuleDepInfo.setLinkLibraries(linkLibraries);
  cache.updateDependency(mainModuleID, mainModuleDepInfo);
}

toolchain::ErrorOr<languagescan_dependency_graph_t>
language::dependencies::performModuleScan(
    CodiraDependencyScanningService &service, CompilerInstance &instance,
    ModuleDependenciesCache &cache,
    DependencyScanDiagnosticCollector *diagnosticCollector) {
  const ASTContext &ctx = instance.getASTContext();
  const FrontendOptions &opts = instance.getInvocation().getFrontendOptions();
  // Load the dependency cache if -reuse-dependency-scan-cache
  // is specified
  if (opts.ReuseDependencyScannerCache) {
    auto cachePath = opts.SerializedDependencyScannerCachePath;
    if (opts.EmitDependencyScannerCacheRemarks)
      ctx.Diags.diagnose(SourceLoc(), diag::remark_reuse_cache, cachePath);

    toolchain::sys::TimePoint<> serializedCacheTimeStamp;
    bool loadFailure =
        module_dependency_cache_serialization::readInterModuleDependenciesCache(
            cachePath, cache, serializedCacheTimeStamp);
    if (opts.EmitDependencyScannerCacheRemarks && loadFailure)
      ctx.Diags.diagnose(SourceLoc(), diag::warn_scanner_deserialize_failed,
                         cachePath);

    if (!loadFailure && opts.ValidatePriorDependencyScannerCache) {
      auto mainModuleID =
          ModuleDependencyID{instance.getMainModule()->getNameStr().str(),
                             ModuleDependencyKind::CodiraSource};
      incremental::validateInterModuleDependenciesCache(
          mainModuleID, cache, instance.getSharedCASInstance(),
          serializedCacheTimeStamp, *instance.getSourceMgr().getFileSystem(),
          ctx.Diags, opts.EmitDependencyScannerCacheRemarks);
    }
  }

  auto scanner = ModuleDependencyScanner(
      service, instance.getInvocation(), instance.getSILOptions(),
      instance.getASTContext(), *instance.getDependencyTracker(),
      instance.getSharedCASInstance(), instance.getSharedCacheInstance(),
      instance.getDiags(),
      instance.getInvocation().getFrontendOptions().ParallelDependencyScan);

  // Identify imports of the main module and add an entry for it
  // to the dependency graph.
  auto mainModuleName = instance.getMainModule()->getNameStr();
  auto mainModuleID = ModuleDependencyID{mainModuleName.str(),
                                         ModuleDependencyKind::CodiraSource};
  if (!cache.hasDependency(mainModuleID))
    cache.recordDependency(mainModuleName, *scanner.getMainModuleDependencyInfo(
                                               instance.getMainModule()));

  // Perform the full module scan starting at the main module.
  auto allModules = scanner.performDependencyScan(mainModuleID, cache);
  if (diagnoseCycle(instance, cache, mainModuleID))
    return std::make_error_code(std::errc::not_supported);

  auto topologicallySortedModuleList =
      computeTopologicalSortOfExplicitDependencies(allModules, cache);

  resolveDependencyCommandLineArguments(scanner, instance, cache,
                                        topologicallySortedModuleList);
  resolveImplicitLinkLibraries(instance, cache);
  updateDependencyTracker(instance, cache, allModules);

  if (ctx.Stats)
    ctx.Stats->getFrontendCounters().NumDepScanFilesystemLookups =
        scanner.getNumLookups();

  // Serialize the dependency cache if -serialize-dependency-scan-cache
  // is specified
  if (opts.SerializeDependencyScannerCache) {
    auto savePath = opts.SerializedDependencyScannerCachePath;
    module_dependency_cache_serialization::writeInterModuleDependenciesCache(
        ctx.Diags, instance.getOutputBackend(), savePath, cache);
    if (opts.EmitDependencyScannerCacheRemarks)
      ctx.Diags.diagnose(SourceLoc(), diag::remark_save_cache, savePath);
  }

  return generateFullDependencyGraph(instance, diagnosticCollector, cache,
                                     topologicallySortedModuleList);
}

toolchain::ErrorOr<languagescan_import_set_t> language::dependencies::performModulePrescan(
    CodiraDependencyScanningService &service, CompilerInstance &instance,
    ModuleDependenciesCache &cache,
    DependencyScanDiagnosticCollector *diagnosticCollector) {
  // Setup the scanner
  auto scanner = ModuleDependencyScanner(
      service, instance.getInvocation(), instance.getSILOptions(),
      instance.getASTContext(), *instance.getDependencyTracker(),
      instance.getSharedCASInstance(), instance.getSharedCacheInstance(),
      instance.getDiags(),
      instance.getInvocation().getFrontendOptions().ParallelDependencyScan);
  // Execute import prescan, and write JSON output to the output stream
  auto mainDependencies =
      scanner.getMainModuleDependencyInfo(instance.getMainModule());
  if (!mainDependencies)
    return mainDependencies.getError();
  auto *importSet = new languagescan_import_set_s;

  std::vector<std::string> importIdentifiers;
  importIdentifiers.reserve(mainDependencies->getModuleImports().size());
  toolchain::transform(mainDependencies->getModuleImports(),
                  std::back_inserter(importIdentifiers),
                  [](const auto &importInfo) -> std::string {
                    return importInfo.importIdentifier;
                  });
  importSet->imports = create_set(importIdentifiers);
  importSet->diagnostics =
      diagnosticCollector
          ? mapCollectedDiagnosticsForOutput(diagnosticCollector)
          : nullptr;
  importSet->diagnostics =
      diagnosticCollector
          ? mapCollectedDiagnosticsForOutput(diagnosticCollector)
          : nullptr;
  return importSet;
}

void language::dependencies::incremental::validateInterModuleDependenciesCache(
    const ModuleDependencyID &rootModuleID, ModuleDependenciesCache &cache,
    std::shared_ptr<toolchain::cas::ObjectStore> cas,
    const toolchain::sys::TimePoint<> &cacheTimeStamp, toolchain::vfs::FileSystem &fs,
    DiagnosticEngine &diags, bool emitRemarks) {
  ModuleDependencyIDSet visited;
  ModuleDependencyIDSet modulesRequiringRescan;
  outOfDateModuleScan(rootModuleID, cache, cas, cacheTimeStamp, fs, diags,
                      emitRemarks, visited, modulesRequiringRescan);
  for (const auto &outOfDateModID : modulesRequiringRescan)
    cache.removeDependency(outOfDateModID);

  // Regardless of invalidation, always re-scan main module.
  cache.removeDependency(rootModuleID);
}

void language::dependencies::incremental::outOfDateModuleScan(
    const ModuleDependencyID &moduleID, const ModuleDependenciesCache &cache,
    std::shared_ptr<toolchain::cas::ObjectStore> cas,
    const toolchain::sys::TimePoint<> &cacheTimeStamp, toolchain::vfs::FileSystem &fs,
    DiagnosticEngine &diags, bool emitRemarks, ModuleDependencyIDSet &visited,
    ModuleDependencyIDSet &modulesRequiringRescan) {
  // Visit the module's dependencies
  bool hasOutOfDateModuleDependency = false;
  for (const auto &depID : cache.getAllDependencies(moduleID)) {
    // If we have not already visited this module, recurse.
    if (visited.find(depID) == visited.end())
      outOfDateModuleScan(depID, cache, cas, cacheTimeStamp, fs, diags,
                          emitRemarks, visited, modulesRequiringRescan);

    // Even if we're not revisiting a dependency, we must check if it's
    // already known to be out of date.
    hasOutOfDateModuleDependency |=
        (modulesRequiringRescan.find(depID) != modulesRequiringRescan.end());
  }

  if (hasOutOfDateModuleDependency) {
    if (emitRemarks)
      diags.diagnose(SourceLoc(), diag::remark_scanner_invalidate_upstream,
                     moduleID.ModuleName);
    modulesRequiringRescan.insert(moduleID);
  } else if (!verifyModuleDependencyUpToDate(
                 moduleID, cache, cas, cacheTimeStamp, fs, diags, emitRemarks))
    modulesRequiringRescan.insert(moduleID);

  visited.insert(moduleID);
}

bool language::dependencies::incremental::verifyModuleDependencyUpToDate(
    const ModuleDependencyID &moduleID, const ModuleDependenciesCache &cache,
    std::shared_ptr<toolchain::cas::ObjectStore> cas,
    const toolchain::sys::TimePoint<> &cacheTimeStamp, toolchain::vfs::FileSystem &fs,
    DiagnosticEngine &diags, bool emitRemarks) {
  const auto &moduleInfo = cache.findKnownDependency(moduleID);
  auto verifyInputOlderThanCacheTimeStamp = [&cacheTimeStamp, &fs, &diags,
                                             emitRemarks](StringRef moduleName,
                                                          StringRef inputPath) {
    toolchain::sys::TimePoint<> inputModTime = toolchain::sys::TimePoint<>::max();
    if (auto Status = fs.status(inputPath))
      inputModTime = Status->getLastModificationTime();
    if (inputModTime > cacheTimeStamp) {
      if (emitRemarks)
        diags.diagnose(SourceLoc(),
                       diag::remark_scanner_stale_result_invalidate, moduleName,
                       inputPath);
      return false;
    }
    return true;
  };

  auto verifyCASID = [cas, &diags, emitRemarks](StringRef moduleName,
                                                     const std::string &casID) {
    if (!cas) {
      // If the wrong cache is passed.
      if (emitRemarks)
        diags.diagnose(SourceLoc(),
                       diag::remark_scanner_invalidate_configuration,
                       moduleName);
      return false;
    }
    auto ID = cas->parseID(casID);
    if (!ID) {
      if (emitRemarks)
        diags.diagnose(SourceLoc(), diag::remark_scanner_invalidate_cas_error,
                       moduleName, toString(ID.takeError()));
      return false;
    }
    if (!cas->getReference(*ID)) {
      if (emitRemarks)
        diags.diagnose(SourceLoc(), diag::remark_scanner_invalidate_missing_cas,
                       moduleName, casID);
      return false;
    }
    return true;
  };

  // Check CAS inputs exist
  if (const auto casID = moduleInfo.getClangIncludeTree())
    if (!verifyCASID(moduleID.ModuleName, *casID))
      return false;
  if (const auto casID = moduleInfo.getCASFSRootID())
    if (!verifyCASID(moduleID.ModuleName, *casID))
      return false;

  // Check interface file for Codira textual modules
  if (const auto &textualModuleDetails = moduleInfo.getAsCodiraInterfaceModule())
    if (!verifyInputOlderThanCacheTimeStamp(
            moduleID.ModuleName, textualModuleDetails->languageInterfaceFile))
      return false;

  // Check binary module file for Codira binary-only modules
  if (const auto &binaryModuleDetails = moduleInfo.getAsCodiraBinaryModule())
    if (!verifyInputOlderThanCacheTimeStamp(
            moduleID.ModuleName, binaryModuleDetails->compiledModulePath))
      return false;

  // Check header input source files (bridging header etc.)
  for (const auto &headerInput : moduleInfo.getHeaderInputSourceFiles())
    if (!verifyInputOlderThanCacheTimeStamp(moduleID.ModuleName, headerInput))
      return false;

  // Auxiliary files
  for (const auto &auxInput : moduleInfo.getAuxiliaryFiles())
    if (!verifyInputOlderThanCacheTimeStamp(moduleID.ModuleName, auxInput))
      return false;

  // Check header/modulemap source files for a Clang dependency
  if (const auto &clangModuleDetails = moduleInfo.getAsClangModule())
    for (const auto &fileInput : clangModuleDetails->fileDependencies)
      if (!verifyInputOlderThanCacheTimeStamp(moduleID.ModuleName, fileInput))
        return false;

  return true;
}
