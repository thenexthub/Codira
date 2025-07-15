//===--- SymbolGraphGen.cpp - Symbol Graph Generator Entry Point ----------===//
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

#include "language/SymbolGraphGen/SymbolGraphGen.h"
#include "language/AST/ASTContext.h"
#include "language/AST/FileSystem.h"
#include "language/AST/Import.h"
#include "language/AST/Module.h"
#include "language/AST/NameLookup.h"
#include "language/Sema/IDETypeChecking.h"
#include "clang/Basic/Module.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/Support/JSON.h"
#include "toolchain/Support/Path.h"

#include "SymbolGraphASTWalker.h"

using namespace language;
using namespace symbolgraphgen;

namespace {
int serializeSymbolGraph(SymbolGraph &SG,
                         const SymbolGraphOptions &Options) {
  SmallString<256> FileName;
  FileName.append(getFullModuleName(&SG.M));
  if (SG.ExtendedModule.has_value()) {
    FileName.push_back('@');
//    FileName.append(SG.ExtendedModule.value()->getNameStr());
    FileName.append(getFullModuleName(SG.ExtendedModule.value()));
  } else if (SG.DeclaringModule.has_value()) {
    // Treat cross-import overlay modules as "extensions" of their declaring module
    FileName.push_back('@');
//    FileName.append(SG.DeclaringModule.value()->getNameStr());
    FileName.append(getFullModuleName(SG.DeclaringModule.value()));
  }
  FileName.append(".symbols.json");

  SmallString<1024> OutputPath(Options.OutputDir);
  toolchain::sys::path::append(OutputPath, FileName);

  return withOutputPath(
      SG.M.getASTContext().Diags, SG.M.getASTContext().getOutputBackend(),
      OutputPath, [&](raw_ostream &OS) {
        toolchain::json::OStream J(OS, Options.PrettyPrint ? 2 : 0);
        SG.serialize(J);
        return false;
      });
}

} // end anonymous namespace

// MARK: - Main Entry Point

/// Emit a symbol graph JSON file for a `ModuleDecl`.
int symbolgraphgen::emitSymbolGraphForModule(
    ModuleDecl *M, const SymbolGraphOptions &Options) {
  ModuleDecl::ImportCollector importCollector(Options.MinimumAccessLevel);

  SmallPtrSet<const clang::Module *, 2> ExportedClangModules = {};
  SmallPtrSet<const clang::Module *, 2> WildcardExportClangModules = {};
  if (const auto *ClangModule = M->findUnderlyingClangModule()) {
    // Scan through the Clang module's exports and collect them for later
    // handling
    for (auto ClangExport : ClangModule->Exports) {
      if (ClangExport.getInt()) {
        // Blanket exports are represented as a true boolean tag
        if (const auto *ExportParent = ClangExport.getPointer()) {
          // If a pointer is present, this is a scoped blanket export, like
          // `export Submodule.*`
          WildcardExportClangModules.insert(ExportParent);
        } else {
          // Otherwise it represents a full blanket `export *`
          WildcardExportClangModules.insert(ClangModule);
        }
      } else if (!ClangExport.getInt() && ClangExport.getPointer()) {
        // This is an explicit `export Submodule`
        ExportedClangModules.insert(ClangExport.getPointer());
      }
    }

    if (ExportedClangModules.empty() && WildcardExportClangModules.empty()) {
      // HACK: In the absence of an explicit export declaration, export all of the submodules.
      WildcardExportClangModules.insert(ClangModule);
    }
  }

  auto importFilter = [&Options, &WildcardExportClangModules,
                       &ExportedClangModules](const ModuleDecl *module) {
    if (!module)
      return false;

    if (const auto *ClangModule = module->findUnderlyingClangModule()) {
      if (ExportedClangModules.contains(ClangModule)) {
        return true;
      }

      for (const auto *ClangParent : WildcardExportClangModules) {
        if (ClangModule->isSubModuleOf(ClangParent))
          return true;
      }
    }

    if (Options.AllowedReexportedModules.has_value())
      for (const auto &allowedModuleName : *Options.AllowedReexportedModules)
        if (allowedModuleName == module->getNameStr())
          return true;

    return false;
  };

  if (Options.AllowedReexportedModules.has_value() ||
      !WildcardExportClangModules.empty() || !ExportedClangModules.empty())
    importCollector.importFilter = std::move(importFilter);

  SmallVector<Decl *, 64> ModuleDecls;
  language::getTopLevelDeclsForDisplay(
      M, ModuleDecls, [&importCollector](ModuleDecl *M, SmallVectorImpl<Decl *> &results) {
        M->getDisplayDeclsRecursivelyAndImports(results, importCollector);
      });

  if (Options.PrintMessages)
    toolchain::errs() << ModuleDecls.size()
        << " top-level declarations in this module.\n";

  SymbolGraphASTWalker Walker(*M, importCollector.imports,
                              importCollector.qualifiedImports, Options);

  for (auto *Decl : ModuleDecls) {
    Walker.walk(Decl);
  }

  if (Options.PrintMessages)
    toolchain::errs()
      << "Found " << Walker.MainGraph.Nodes.size() << " symbols and "
      << Walker.MainGraph.Edges.size() << " relationships.\n";

  int Success = EXIT_SUCCESS;

  Success |= serializeSymbolGraph(Walker.MainGraph, Options);

  for (const auto &Entry : Walker.ExtendedModuleGraphs) {
    if (Entry.getValue()->empty()) {
      continue;
    }
    Success |= serializeSymbolGraph(*Entry.getValue(), Options);
  }

  return Success;
}

int symbolgraphgen::
printSymbolGraphForDecl(const ValueDecl *D, Type BaseTy,
                        bool InSynthesizedExtension,
                        const SymbolGraphOptions &Options,
                        toolchain::raw_ostream &OS,
                        SmallVectorImpl<PathComponent> &ParentContexts,
                        SmallVectorImpl<FragmentInfo> &FragmentInfo) {
  if (!Symbol::supportsKind(D->getKind()))
    return EXIT_FAILURE;

  toolchain::json::OStream JOS(OS, Options.PrettyPrint ? 2 : 0);
  ModuleDecl *MD = D->getModuleContext();
  SymbolGraphASTWalker Walker(*MD, Options);
  markup::MarkupContext MarkupCtx;
  SymbolGraph Graph(Walker, *MD, std::nullopt, MarkupCtx, std::nullopt,
                    /*IsForSingleNode=*/true);
  NominalTypeDecl *NTD = InSynthesizedExtension
      ? BaseTy->getAnyNominal()
      : nullptr;

  Symbol MySym(&Graph, D, NTD, BaseTy);
  MySym.getPathComponents(ParentContexts);
  MySym.getFragmentInfo(FragmentInfo);
  Graph.recordNode(MySym);
  Graph.serialize(JOS);
  return EXIT_SUCCESS;
}
