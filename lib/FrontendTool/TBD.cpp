//===--- TBD.cpp -- generates and validates TBD files ---------------------===//
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

#include "TBD.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Decl.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/FileSystem.h"
#include "language/AST/FileUnit.h"
#include "language/AST/Module.h"
#include "language/AST/TBDGenRequests.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Toolchain.h"
#include "language/Demangling/Demangle.h"
#include "language/Frontend/FrontendOptions.h"
#include "language/IRGen/TBDGen.h"

#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/StringSet.h"
#include "toolchain/IR/Module.h"
#include "toolchain/IR/Mangler.h"
#include "toolchain/IR/ValueSymbolTable.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/VirtualOutputBackend.h"
#include <vector>

using namespace language;

static std::vector<StringRef> sortSymbols(toolchain::StringSet<> &symbols) {
  std::vector<StringRef> sorted;
  for (auto &symbol : symbols)
    sorted.push_back(symbol.getKey());
  std::sort(sorted.begin(), sorted.end());
  return sorted;
}

bool language::writeTBD(ModuleDecl *M, StringRef OutputFilename,
                     toolchain::vfs::OutputBackend &Backend,
                     const TBDGenOptions &Opts) {
  return withOutputPath(M->getDiags(), Backend, OutputFilename,
                        [&](raw_ostream &OS) -> bool {
                          writeTBDFile(M, OS, Opts);
                          return false;
                        });
}

static bool validateSymbols(DiagnosticEngine &diags,
                            const std::vector<std::string> &symbols,
                            const toolchain::Module &IRModule,
                            bool diagnoseExtraSymbolsInTBD) {
  toolchain::StringSet<> symbolSet;
  symbolSet.insert(symbols.begin(), symbols.end());

  auto error = false;

  // Diff the two sets of symbols, flagging anything outside their intersection.

  // Delay the emission of errors for things in the IR but not TBD, so we can
  // sort them to get a stable order.
  std::vector<StringRef> irNotTBD;

  for (auto &nameValue : IRModule.getValueSymbolTable()) {
    // TBDGen inserts mangled names (usually with a leading '_') into its
    // symbol table, so make sure to mangle IRGen names before comparing them
    // with what TBDGen created.
    auto unmangledName = nameValue.getKey();

    SmallString<128> name;
    toolchain::Mangler::getNameWithPrefix(name, unmangledName,
                                     IRModule.getDataLayout());

    auto value = nameValue.getValue();
    if (auto GV = dyn_cast<toolchain::GlobalValue>(value)) {
      // Is this a symbol that should be listed?
      auto externallyVisible =
          (GV->hasExternalLinkage() || GV->hasCommonLinkage())
        && !GV->hasHiddenVisibility();
      if (!GV->isDeclaration() && externallyVisible) {
        // Is it listed?
        if (!symbolSet.erase(name))
          // Note: Add the unmangled name to the irNotTBD list, which is owned
          //       by the IRModule, instead of the mangled name.
          irNotTBD.push_back(unmangledName);
      }
    } else {
      assert(!symbolSet.contains(name) &&
             "non-global value in value symbol table");
    }
  }

  std::sort(irNotTBD.begin(), irNotTBD.end());
  for (auto &name : irNotTBD) {
    diags.diagnose(SourceLoc(), diag::symbol_in_ir_not_in_tbd, name,
                   Demangle::demangleSymbolAsString(name));
    error = true;
  }

  if (diagnoseExtraSymbolsInTBD) {
    // Look for any extra symbols.
    for (auto &name : sortSymbols(symbolSet)) {
      diags.diagnose(SourceLoc(), diag::symbol_in_tbd_not_in_ir, name,
                     Demangle::demangleSymbolAsString(name));
      error = true;
    }
  }

  if (error) {
    diags.diagnose(SourceLoc(), diag::tbd_validation_failure);
  }

  return error;
}

bool language::validateTBD(ModuleDecl *M,
                        const toolchain::Module &IRModule,
                        const TBDGenOptions &opts,
                        bool diagnoseExtraSymbolsInTBD) {
  auto symbols = getPublicSymbols(TBDGenDescriptor::forModule(M, opts));
  return validateSymbols(M->getASTContext().Diags, symbols, IRModule,
                         diagnoseExtraSymbolsInTBD);
}

bool language::validateTBD(FileUnit *file,
                        const toolchain::Module &IRModule,
                        const TBDGenOptions &opts,
                        bool diagnoseExtraSymbolsInTBD) {
  auto symbols = getPublicSymbols(TBDGenDescriptor::forFile(file, opts));
  return validateSymbols(file->getParentModule()->getASTContext().Diags,
                         symbols, IRModule, diagnoseExtraSymbolsInTBD);
}
