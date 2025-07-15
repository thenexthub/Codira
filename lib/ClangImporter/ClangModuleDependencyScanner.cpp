//===--- ClangModuleDependencyScanner.cpp - Dependency Scanning -----------===//
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
// This file implements dependency scanning for Clang modules.
//
//===----------------------------------------------------------------------===//
#include "ImporterImpl.h"
#include "language/AST/DiagnosticsSema.h"
#include "language/AST/ModuleDependencies.h"
#include "language/Basic/SourceManager.h"
#include "language/ClangImporter/ClangImporter.h"
#include "language/Basic/Assertions.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/CAS/CASOptions.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendOptions.h"
#include "clang/Tooling/DependencyScanning/DependencyScanningService.h"
#include "clang/Tooling/DependencyScanning/DependencyScanningTool.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/Support/Allocator.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Signals.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/StringSaver.h"

using namespace language;

using namespace clang::tooling;
using namespace clang::tooling::dependencies;

static void addScannerPrefixMapperInvocationArguments(
    std::vector<std::string> &invocationArgStrs, ASTContext &ctx) {
  for (const auto &arg : ctx.SearchPathOpts.ScannerPrefixMapper) {
    invocationArgStrs.push_back("-fdepscan-prefix-map");
    invocationArgStrs.push_back(arg.first);
    invocationArgStrs.push_back(arg.second);
  }
}

/// Create the command line for Clang dependency scanning.
std::vector<std::string> ClangImporter::getClangDepScanningInvocationArguments(
    ASTContext &ctx) {
  std::vector<std::string> commandLineArgs = getClangDriverArguments(ctx);
  addScannerPrefixMapperInvocationArguments(commandLineArgs, ctx);

  // HACK! Drop the -fmodule-format= argument and the one that
  // precedes it.
  {
    auto moduleFormatPos = std::find_if(commandLineArgs.begin(),
                                        commandLineArgs.end(),
                                        [](StringRef arg) {
      return arg.starts_with("-fmodule-format=");
    });
    assert(moduleFormatPos != commandLineArgs.end());
    assert(moduleFormatPos != commandLineArgs.begin());
    commandLineArgs.erase(moduleFormatPos-1, moduleFormatPos+1);
  }

  // Use `-fsyntax-only` to do dependency scanning and assert if not there.
  assert(std::find(commandLineArgs.begin(), commandLineArgs.end(),
                   "-fsyntax-only") != commandLineArgs.end() &&
         "missing -fsyntax-only option");

  // The Clang modules produced by ClangImporter are always embedded in an
  // ObjectFilePCHContainer and contain -gmodules debug info.
  commandLineArgs.push_back("-gmodules");

  // To use -gmodules we need to have a real path for the PCH; this option has
  // no effect if caching is disabled.
  commandLineArgs.push_back("-Xclang");
  commandLineArgs.push_back("-finclude-tree-preserve-pch-path");

  return commandLineArgs;
}

ModuleDependencyVector ClangImporter::bridgeClangModuleDependencies(
    const ASTContext &ctx,
    clang::tooling::dependencies::DependencyScanningTool &clangScanningTool,
    clang::tooling::dependencies::ModuleDepsGraph &clangDependencies,
    LookupModuleOutputCallback lookupModuleOutput,
    RemapPathCallback callback) {
  ModuleDependencyVector result;

  auto remapPath = [&](StringRef path) {
    if (callback)
      return callback(path);
    return path.str();
  };

  for (auto &clangModuleDep : clangDependencies) {
    // File dependencies for this module.
    std::vector<std::string> fileDeps;
    clangModuleDep.forEachFileDep(
        [&fileDeps](StringRef fileDep) { fileDeps.emplace_back(fileDep); });

    std::vector<std::string> languageArgs;
    auto addClangArg = [&](Twine arg) {
      languageArgs.push_back("-Xcc");
      languageArgs.push_back(arg.str());
    };

    // We are using Codira frontend mode.
    languageArgs.push_back("-frontend");

    // Codira frontend action: -emit-pcm
    languageArgs.push_back("-emit-pcm");
    languageArgs.push_back("-module-name");
    languageArgs.push_back(clangModuleDep.ID.ModuleName);

    auto pcmPath = lookupModuleOutput(clangModuleDep,
                                      ModuleOutputKind::ModuleFile);
    languageArgs.push_back("-o");
    languageArgs.push_back(pcmPath);

    // Ensure that the resulting PCM build invocation uses Clang frontend
    // directly
    languageArgs.push_back("-direct-clang-cc1-module-build");

    // Codira frontend option for input file path (Foo.modulemap).
    languageArgs.push_back(remapPath(clangModuleDep.ClangModuleMapFile));

    auto invocation = clangModuleDep.getUnderlyingCompilerInvocation();
    // Clear some options from clang scanner.
    invocation.getMutFrontendOpts().ModuleCacheKeys.clear();
    invocation.getMutFrontendOpts().PathPrefixMappings.clear();
    invocation.getMutFrontendOpts().OutputFile.clear();

    // Reset CASOptions since that should be coming from language.
    invocation.getMutCASOpts() = clang::CASOptions();
    invocation.getMutFrontendOpts().CASIncludeTreeID.clear();

    // FIXME: workaround for rdar://105684525: find the -ivfsoverlay option
    // from clang scanner and pass to language.
    if (!ctx.CASOpts.EnableCaching) {
      auto &overlayFiles = invocation.getMutHeaderSearchOpts().VFSOverlayFiles;
      for (auto overlay : overlayFiles) {
        languageArgs.push_back("-vfsoverlay");
        languageArgs.push_back(overlay);
      }
    }

    // Add args reported by the scanner.
    auto clangArgs = invocation.getCC1CommandLine();
    toolchain::for_each(clangArgs, addClangArg);

    // CASFileSystemRootID.
    std::string RootID = clangModuleDep.CASFileSystemRootID
                             ? clangModuleDep.CASFileSystemRootID->toString()
                             : "";

    std::string IncludeTree =
        clangModuleDep.IncludeTreeID ? *clangModuleDep.IncludeTreeID : "";

    ctx.CASOpts.enumerateCASConfigurationFlags(
        [&](StringRef Arg) { languageArgs.push_back(Arg.str()); });

    if (!IncludeTree.empty()) {
      languageArgs.push_back("-clang-include-tree-root");
      languageArgs.push_back(IncludeTree);
    }
    std::string mappedPCMPath = remapPath(pcmPath);

    std::vector<LinkLibrary> LinkLibraries;
    for (const auto &ll : clangModuleDep.LinkLibraries)
      LinkLibraries.emplace_back(
          ll.Library,
          ll.IsFramework ? LibraryKind::Framework : LibraryKind::Library,
          /*static=*/false);

    // Module-level dependencies.
    toolchain::StringSet<> alreadyAddedModules;
    auto dependencies = ModuleDependencyInfo::forClangModule(
        pcmPath, mappedPCMPath, clangModuleDep.ClangModuleMapFile,
        clangModuleDep.ID.ContextHash, languageArgs, fileDeps,
        LinkLibraries, RootID, IncludeTree, /*module-cache-key*/ "",
        clangModuleDep.IsSystem);

    std::vector<ModuleDependencyID> directDependencyIDs;
    for (const auto &moduleName : clangModuleDep.ClangModuleDeps) {
      // FIXME: This assumes, conservatively, that all Clang module imports
      // are exported. We need to fix this once the clang scanner gains the appropriate
      // API to query this.
      dependencies.addModuleImport(moduleName.ModuleName, /* isExported */ true,
                                   AccessLevel::Public, &alreadyAddedModules);
      // It is safe to assume that all dependencies of a Clang module are Clang modules.
      directDependencyIDs.push_back({moduleName.ModuleName, ModuleDependencyKind::Clang});
    }
    dependencies.setImportedClangDependencies(directDependencyIDs);
    result.push_back(std::make_pair(ModuleDependencyID{clangModuleDep.ID.ModuleName,
                                                       ModuleDependencyKind::Clang},
                                    dependencies));
  }
  return result;
}

void ClangImporter::getBridgingHeaderOptions(
    const ASTContext &ctx,
    const clang::tooling::dependencies::TranslationUnitDeps &deps,
    std::vector<std::string> &languageArgs) {
  auto addClangArg = [&](Twine arg) {
    languageArgs.push_back("-Xcc");
    languageArgs.push_back(arg.str());
  };

  // We are using Codira frontend mode.
  languageArgs.push_back("-frontend");

  // Codira frontend action: -emit-pcm
  languageArgs.push_back("-emit-pch");

  // Ensure that the resulting PCM build invocation uses Clang frontend
  // directly
  languageArgs.push_back("-direct-clang-cc1-module-build");

  // Add args reported by the scanner.

  // Round-trip clang args to canonicalize and clear the options that language
  // compiler doesn't need.
  clang::CompilerInvocation depsInvocation;
  clang::DiagnosticsEngine clangDiags(new clang::DiagnosticIDs(),
                                      new clang::DiagnosticOptions(),
                                      new clang::IgnoringDiagConsumer());

  toolchain::SmallVector<const char *> clangArgs;
  toolchain::for_each(deps.Commands[0].Arguments, [&](const std::string &Arg) {
    clangArgs.push_back(Arg.c_str());
  });

  bool success = clang::CompilerInvocation::CreateFromArgs(
      depsInvocation, clangArgs, clangDiags);
  (void)success;
  assert(success && "clang option from dep scanner round trip failed");

  // Clear the cache key for module. The module key is computed from clang
  // invocation, not language invocation.
  depsInvocation.getFrontendOpts().ProgramAction =
      clang::frontend::ActionKind::GeneratePCH;
  depsInvocation.getFrontendOpts().ModuleCacheKeys.clear();
  depsInvocation.getFrontendOpts().PathPrefixMappings.clear();
  depsInvocation.getFrontendOpts().OutputFile.clear();

  toolchain::BumpPtrAllocator allocator;
  toolchain::StringSaver saver(allocator);
  clangArgs.clear();
  depsInvocation.generateCC1CommandLine(
      clangArgs,
      [&saver](const toolchain::Twine &T) { return saver.save(T).data(); });

  toolchain::for_each(clangArgs, addClangArg);

  ctx.CASOpts.enumerateCASConfigurationFlags(
      [&](StringRef Arg) { languageArgs.push_back(Arg.str()); });

  if (auto Tree = deps.IncludeTreeID) {
    languageArgs.push_back("-clang-include-tree-root");
    languageArgs.push_back(*Tree);
  }
}
