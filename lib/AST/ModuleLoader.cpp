//===--- ModuleLoader.cpp - Codira Language Module Implementation ----------===//
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
//  This file implements the ModuleLoader class and/or any helpers.
//
//===----------------------------------------------------------------------===//

#include "language/AST/ASTContext.h"
#include "language/AST/DiagnosticsCommon.h"
#include "language/AST/FileUnit.h"
#include "language/AST/ModuleLoader.h"
#include "language/AST/ModuleDependencies.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/FileTypes.h"
#include "language/Basic/Platform.h"
#include "language/Basic/SourceManager.h"
#include "clang/Frontend/Utils.h"
#include "language/ClangImporter/ClangImporter.h"

namespace toolchain {
class FileCollectorBase;
}

namespace language {

DependencyTracker::DependencyTracker(
    IntermoduleDepTrackingMode Mode,
    std::shared_ptr<toolchain::FileCollectorBase> FileCollector)
    // NB: The ClangImporter believes it's responsible for the construction of
    // this instance, and it static_cast<>s the instance pointer to its own
    // subclass based on that belief. If you change this to be some other
    // instance, you will need to change ClangImporter's code to handle the
    // difference.
    : clangCollector(
          ClangImporter::createDependencyCollector(Mode, FileCollector)) {}

void
DependencyTracker::addDependency(StringRef File, bool IsSystem) {
  // DependencyTracker exposes an interface that (intentionally) does not talk
  // about clang at all, nor about missing deps. It does expose an IsSystem
  // dimension, which we accept and pass along to the clang DependencyCollector.
  clangCollector->maybeAddDependency(File, /*FromModule=*/false,
                                     IsSystem, /*IsModuleFile=*/false,
                                     /*IsMissing=*/false);
}

void DependencyTracker::addIncrementalDependency(StringRef File,
                                                 Fingerprint FP) {
  if (incrementalDepsUniquer.insert(File).second) {
    incrementalDeps.emplace_back(File.str(), FP);
  }
}

void DependencyTracker::addMacroPluginDependency(StringRef File,
                                                 Identifier ModuleName) {
  macroPluginDeps.insert({ModuleName, std::string(File)});
}

ArrayRef<std::string>
DependencyTracker::getDependencies() const {
  return clangCollector->getDependencies();
}

ArrayRef<DependencyTracker::IncrementalDependency>
DependencyTracker::getIncrementalDependencies() const {
  return incrementalDeps;
}

ArrayRef<DependencyTracker::MacroPluginDependency>
DependencyTracker::getMacroPluginDependencies() const {
  return macroPluginDeps.getArrayRef();
}

std::shared_ptr<clang::DependencyCollector>
DependencyTracker::getClangCollector() {
  return clangCollector;
}

static bool findOverlayFilesInDirectory(ASTContext &ctx, StringRef path,
                                        StringRef moduleName,
                                        SourceLoc diagLoc,
                                        toolchain::function_ref<void(StringRef)> callback) {
  using namespace toolchain::sys;
  using namespace file_types;

  auto fs = ctx.SourceMgr.getFileSystem();

  std::error_code error;
  for (auto dir = fs->dir_begin(path, error);
       !error && dir != toolchain::vfs::directory_iterator();
       dir.increment(error)) {
    StringRef file = dir->path();
    if (lookupTypeForExtension(path::extension(file)) != TY_CodiraOverlayFile)
      continue;

    callback(file);
  }

  // A CAS file list returns operation not permitted on directory iterations.
  if (error && error != std::errc::no_such_file_or_directory &&
      error != std::errc::operation_not_permitted) {
    ctx.Diags.diagnose(diagLoc, diag::cannot_list_languagecrossimport_dir,
                       moduleName, error.message(), path);
  }
  return !error;
}

static void findOverlayFilesInternal(ASTContext &ctx, StringRef moduleDefiningPath,
                             StringRef moduleName,
                             SourceLoc diagLoc,
                             toolchain::function_ref<void(StringRef)> callback) {
  using namespace toolchain::sys;
  using namespace file_types;
  auto &langOpts = ctx.LangOpts;
  // This method constructs several paths to directories near the module and
  // scans them for .codeoverlay files. These paths can be in various
  // directories and have a few different filenames at the end, but I'll
  // illustrate the path transformations by showing examples for a module
  // defined by a languageinterface at:
  //
  // /usr/lib/language/FooKit.codemodule/x86_64-apple-macos.codeinterface

  // dirPath = /usr/lib/language/FooKit.codemodule
  SmallString<64> dirPath{moduleDefiningPath};

  // dirPath = /usr/lib/language/
  path::remove_filename(dirPath);

  // dirPath = /usr/lib/language/FooKit.codecrossimport
  path::append(dirPath, moduleName);
  path::replace_extension(dirPath, getExtension(TY_CodiraCrossImportDir));

  // Search for languageoverlays that apply to all platforms.
  if (!findOverlayFilesInDirectory(ctx, dirPath, moduleName, diagLoc, callback))
    // If we diagnosed an error, or we didn't find the directory at all, don't
    // bother trying the target-specific directories.
    return;

  // dirPath = /usr/lib/language/FooKit.codecrossimport/x86_64-apple-macos
  auto moduleTriple = getTargetSpecificModuleTriple(langOpts.Target);
  path::append(dirPath, moduleTriple.str());

  // Search for languageoverlays specific to the target triple's platform.
  findOverlayFilesInDirectory(ctx, dirPath, moduleName, diagLoc, callback);

  // The rest of this handles target variant triples, which are only used for
  // certain MacCatalyst builds.
  if (!langOpts.TargetVariant)
    return;

  // dirPath = /usr/lib/language/FooKit.codecrossimport/x86_64-apple-ios-macabi
  path::remove_filename(dirPath);
  auto moduleVariantTriple =
      getTargetSpecificModuleTriple(*langOpts.TargetVariant);
  path::append(dirPath, moduleVariantTriple.str());

  // Search for languageoverlays specific to the target variant's platform.
  findOverlayFilesInDirectory(ctx, dirPath, moduleName, diagLoc, callback);
}

void ModuleLoader::findOverlayFiles(SourceLoc diagLoc, ModuleDecl *module,
                                    FileUnit *file) {
  using namespace toolchain::sys;
  using namespace file_types;

  // If cross import information is passed on command-line, prefer use that.
  auto &crossImports = module->getASTContext().SearchPathOpts.CrossImportInfo;
  auto overlays = crossImports.find(module->getNameStr());
  if (overlays != crossImports.end()) {
    for (auto entry : overlays->second) {
      module->addCrossImportOverlayFile(entry);
      if (dependencyTracker)
        dependencyTracker->addDependency(entry, module->isSystemModule());
    }
  }
  if (module->getASTContext().SearchPathOpts.DisableCrossImportOverlaySearch)
    return;

  if (file->getModuleDefiningPath().empty())
    return;
  findOverlayFilesInternal(module->getASTContext(),
                           file->getModuleDefiningPath(),
                           module->getName().str(),
                           diagLoc,
                           [&](StringRef file) {
    module->addCrossImportOverlayFile(file);
    // FIXME: Better to add it only if we load it.
    if (dependencyTracker)
      dependencyTracker->addDependency(file, module->isSystemModule());
  });
}

toolchain::StringMap<toolchain::SmallSetVector<Identifier, 4>>
ModuleDependencyInfo::collectCrossImportOverlayNames(
    ASTContext &ctx, StringRef moduleName,
    std::set<std::pair<std::string, std::string>> &overlayFiles) const {
  using namespace toolchain::sys;
  using namespace file_types;
  std::optional<std::string> modulePath;
  // A map from secondary module name to a vector of overlay names.
  toolchain::StringMap<toolchain::SmallSetVector<Identifier, 4>> result;

  switch (getKind()) {
    case language::ModuleDependencyKind::CodiraInterface: {
      auto *languageDep = getAsCodiraInterfaceModule();
      // Prefer interface path to binary module path if we have it.
      modulePath = languageDep->languageInterfaceFile;
      assert(modulePath.has_value());
      StringRef parentDir = toolchain::sys::path::parent_path(*modulePath);
      if (toolchain::sys::path::extension(parentDir) == ".codemodule") {
        modulePath = parentDir.str();
      }
      break;
    }
    case language::ModuleDependencyKind::CodiraBinary: {
      auto *languageBinaryDep = getAsCodiraBinaryModule();
      modulePath = languageBinaryDep->getDefiningModulePath();
      assert(modulePath.has_value());
      StringRef parentDir = toolchain::sys::path::parent_path(*modulePath);
      if (toolchain::sys::path::extension(parentDir) == ".codemodule") {
        modulePath = parentDir.str();
      }
      break;
    }
    case language::ModuleDependencyKind::Clang: {
      auto *clangDep = getAsClangModule();
      modulePath = clangDep->moduleMapFile;
      assert(modulePath.has_value());
      break;
    }
    case language::ModuleDependencyKind::CodiraSource: {
      return result;
    }
    case language::ModuleDependencyKind::LastKind:
      toolchain_unreachable("Unhandled dependency kind.");
  }
  // Mimic getModuleDefiningPath() for Codira and Clang module.
  findOverlayFilesInternal(ctx, *modulePath, moduleName, SourceLoc(),
                           [&](StringRef file) {
    StringRef bystandingModule;
    auto overlayNames =
      ModuleDecl::collectCrossImportOverlay(ctx, file, moduleName,
                                            bystandingModule);
    result[bystandingModule] = std::move(overlayNames);
    overlayFiles.insert({moduleName.str(), file.str()});
  });
  return result;
}
} // namespace language
