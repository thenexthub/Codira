//===--- ImportedModules.cpp -- generates the list of imported modules ----===//
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

#include "Dependencies.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Decl.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/Module.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/STLExtras.h"
#include "language/ClangImporter/ClangImporter.h"
#include "language/Frontend/FrontendOptions.h"
#include "clang/Basic/Module.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/VirtualOutputBackend.h"

using namespace language;

static StringRef getTopLevelName(const clang::Module *module) {
  return module->getTopLevelModule()->Name;
}

static void findAllClangImports(const clang::Module *module,
                                toolchain::SetVector<StringRef> &modules) {
  for (auto imported : module->Imports) {
    modules.insert(getTopLevelName(imported));
  }

  for (auto sub : module->submodules()) {
    findAllClangImports(sub, modules);
  }
}

bool language::emitImportedModules(ModuleDecl *mainModule,
                                const FrontendOptions &opts,
                                toolchain::vfs::OutputBackend &backend) {
  auto &Context = mainModule->getASTContext();
  std::string path = opts.InputsAndOutputs.getSingleOutputFilename();
  auto &diags = Context.Diags;
  auto out = backend.createFile(path);
  if (!out) {
    diags.diagnose(SourceLoc(), diag::error_opening_output,
                   path, toString(out.takeError()));
    return true;
  }

  toolchain::SetVector<StringRef> Modules;

  // Find the imports in the main Codira code.
  // We don't need `getTopLevelDeclsForDisplay()` here because we only care
  // about `ImportDecl`s.
  toolchain::SmallVector<Decl *, 32> Decls;
  mainModule->getDisplayDecls(Decls);
  for (auto D : Decls) {
    auto ID = dyn_cast<ImportDecl>(D);
    if (!ID)
      continue;

    ImportPath::Builder scratch;
    auto modulePath = ID->getRealModulePath(scratch);
    // only the top-level name is needed (i.e. A in A.B.C)
    Modules.insert(modulePath[0].Item.str());
  }

  // And now look in the C code we're possibly using.
  auto clangImporter =
      static_cast<ClangImporter *>(Context.getClangModuleLoader());

  StringRef implicitHeaderPath = opts.ImplicitObjCHeaderPath;
  if (!implicitHeaderPath.empty()) {
    if (!clangImporter->importBridgingHeader(implicitHeaderPath, mainModule)) {
      SmallVector<ImportedModule, 16> imported;
      clangImporter->getImportedHeaderModule()->getImportedModules(
          imported, ModuleDecl::getImportFilterLocal());

      for (auto IM : imported) {
        if (auto clangModule = IM.importedModule->findUnderlyingClangModule())
          Modules.insert(getTopLevelName(clangModule));
        else
          assert(IM.importedModule->isStdlibModule() &&
                 "unexpected non-stdlib language module");
      }
    }
  }

  if (opts.ImportUnderlyingModule) {
    auto underlyingModule = clangImporter->loadModule(SourceLoc(),
      ImportPath::Module::Builder(mainModule->getName()).get());
    if (!underlyingModule) {
      Context.Diags.diagnose(SourceLoc(),
                             diag::error_underlying_module_not_found,
                             mainModule->getName());
      return true;
    }
    auto clangModule = underlyingModule->findUnderlyingClangModule();

    findAllClangImports(clangModule, Modules);
  }

  for (auto name : Modules) {
    *out << name << "\n";
  }

  if (auto error = out->keep()) {
    diags.diagnose(SourceLoc(), diag::error_closing_output,
                   path, toString(std::move(error)));
    return true;
  }

  return false;
}
