//===--- SourceLoader.cpp - Import .code files as modules ----------------===//
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
///
/// \file
/// A simple module loader that loads .code source files.
///
//===----------------------------------------------------------------------===//

#include "language/Sema/SourceLoader.h"
#include "language/Subsystems.h"
#include "language/AST/ASTContext.h"
#include "language/AST/DiagnosticsSema.h"
#include "language/AST/Module.h"
#include "language/AST/ModuleDependencies.h"
#include "language/AST/SourceFile.h"
#include "language/Parse/PersistentParserState.h"
#include "language/Basic/SourceManager.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/PrefixMapper.h"
#include "toolchain/Support/SaveAndRestore.h"
#include <system_error>

using namespace language;

// FIXME: Basically the same as SerializedModuleLoader.
using FileOrError = toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>>;

static FileOrError findModule(ASTContext &ctx, Identifier moduleID,
                              SourceLoc importLoc) {
  toolchain::SmallString<128> inputFilename;
  // Find a module with an actual, physical name on disk, in case
  // -module-alias is used (otherwise same).
  //
  // For example, if '-module-alias Foo=Bar' is passed in to the frontend,
  // and a source file has 'import Foo', a module called Bar (real name)
  // should be searched.
  StringRef moduleNameRef = ctx.getRealModuleName(moduleID).str();

  for (const auto &Path : ctx.SearchPathOpts.getImportSearchPaths()) {
    inputFilename = Path.Path;
    toolchain::sys::path::append(inputFilename, moduleNameRef);
    inputFilename.append(".code");
    toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> FileBufOrErr =
      ctx.SourceMgr.getFileSystem()->getBufferForFile(inputFilename.str());

    // Return if we loaded a file
    if (FileBufOrErr)
      return FileBufOrErr;
    // Or if we get any error other than the file not existing
    auto err = FileBufOrErr.getError();
    if (err != std::errc::no_such_file_or_directory)
      return FileBufOrErr;
  }

  return make_error_code(std::errc::no_such_file_or_directory);
}

void SourceLoader::collectVisibleTopLevelModuleNames(
    SmallVectorImpl<Identifier> &names) const {
  // TODO: Implement?
}

bool SourceLoader::canImportModule(ImportPath::Module path, SourceLoc loc,
                                   ModuleVersionInfo *versionInfo,
                                   bool isTestableDependencyLookup) {
  // FIXME: Codira submodules?
  if (path.hasSubmodule())
    return false;

  auto ID = path[0];
  // Search the memory buffers to see if we can find this file on disk.
  FileOrError inputFileOrError = findModule(Ctx, ID.Item,
                                            ID.Loc);
  if (!inputFileOrError) {
    auto err = inputFileOrError.getError();
    if (err != std::errc::no_such_file_or_directory) {
      Ctx.Diags.diagnose(ID.Loc, diag::sema_opening_import,
                         ID.Item, err.message());
    }

    return false;
  }

  return true;
}

ModuleDecl *SourceLoader::loadModule(SourceLoc importLoc,
                                     ImportPath::Module path,
                                     bool AllowMemoryCache) {
  // FIXME: Codira submodules?
  if (path.size() > 1)
    return nullptr;

  auto moduleID = path[0];

  FileOrError inputFileOrError = findModule(Ctx, moduleID.Item,
                                            moduleID.Loc);
  if (!inputFileOrError) {
    auto err = inputFileOrError.getError();
    if (err != std::errc::no_such_file_or_directory) {
      Ctx.Diags.diagnose(moduleID.Loc, diag::sema_opening_import,
                         moduleID.Item, err.message());
    }

    return nullptr;
  }
  std::unique_ptr<toolchain::MemoryBuffer> inputFile =
    std::move(inputFileOrError.get());

  if (dependencyTracker)
    dependencyTracker->addDependency(inputFile->getBufferIdentifier(),
                                     /*isSystem=*/false);

  unsigned bufferID;
  if (auto BufID =
       Ctx.SourceMgr.getIDForBufferIdentifier(inputFile->getBufferIdentifier()))
    bufferID = BufID.value();
  else
    bufferID = Ctx.SourceMgr.addNewSourceBuffer(std::move(inputFile));

  ImplicitImportInfo importInfo;
  importInfo.StdlibKind = Ctx.getStdlibModule() ? ImplicitStdlibKind::Stdlib
                                                : ImplicitStdlibKind::None;

  auto *importMod = ModuleDecl::create(
      moduleID.Item, Ctx, importInfo, [&](ModuleDecl *importMod, auto addFile) {
    auto opts = SourceFile::getDefaultParsingOptions(Ctx.LangOpts);
    addFile(new (Ctx) SourceFile(*importMod, SourceFileKind::Library, bufferID,
                                 opts));
  });
  if (EnableLibraryEvolution)
    importMod->setResilienceStrategy(ResilienceStrategy::Resilient);
  Ctx.addLoadedModule(importMod);

  performImportResolution(importMod);
  bindExtensions(*importMod);
  return importMod;
}

void SourceLoader::loadExtensions(NominalTypeDecl *nominal,
                                  unsigned previousGeneration) {
  // Type-checking the source automatically loads all extensions; there's
  // nothing to do here.
}
