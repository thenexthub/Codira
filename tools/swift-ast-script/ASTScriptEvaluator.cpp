//===--- ASTScriptEvaluator.cpp -------------------------------------------===//
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
/// AST script evaluation.
///
//===----------------------------------------------------------------------===//

#include "ASTScript.h"
#include "ASTScriptConfiguration.h"

#include "language/AST/ASTMangler.h"
#include "language/AST/ASTWalker.h"
#include "language/AST/Decl.h"
#include "language/AST/NameLookup.h"
#include "language/AST/NameLookupRequests.h"
#include "language/Frontend/Frontend.h"

using namespace language;
using namespace scripting;

namespace {

class ASTScriptWalker : public ASTWalker {
  const ASTScript &Script;
  ProtocolDecl *ViewProtocol;

public:
  ASTScriptWalker(const ASTScript &script, ProtocolDecl *viewProtocol)
    : Script(script), ViewProtocol(viewProtocol) {}

  MacroWalking getMacroWalkingBehavior() const override {
    return MacroWalking::Expansion;
  }

  PreWalkAction walkToDeclPre(Decl *D) override {
    visit(D);
    return Action::Continue();
  }

  void visit(const Decl *D) {
    auto fn = dyn_cast<AbstractFunctionDecl>(D);
    if (!fn) return;

    // Suppress warnings.
    (void) Script;

    for (auto param : *fn->getParameters()) {
      // The parameter must have function type.
      auto paramType = param->getInterfaceType();
      auto paramFnType = paramType->getAs<FunctionType>();
      if (!paramFnType) continue;

      // The parameter function must return a type parameter that
      // conforms to CodiraUI.View.
      auto paramResultType = paramFnType->getResult();
      if (!paramResultType->isTypeParameter()) continue;
      auto sig = fn->getGenericSignature();
      if (!sig->requiresProtocol(paramResultType, ViewProtocol)) continue;

      // The parameter must not be a @ViewBuilder parameter.
      if (param->getResultBuilderType()) continue;

      // Print the function.
      printDecl(fn);
    }
  }

  void printDecl(const ValueDecl *decl) {
    // FIXME: there's got to be some better way to print an exact reference
    // to a declaration, including its context.
    printDecl(toolchain::outs(), decl);
    toolchain::outs() << "\n";
  }

  void printDecl(toolchain::raw_ostream &out, const ValueDecl *decl) {
    if (auto accessor = dyn_cast<AccessorDecl>(decl)) {
      printDecl(out, accessor->getStorage());
      out << ".(accessor)";
    } else {
      printDeclContext(out, decl->getDeclContext());
      out << decl->getName();
    }
  }

  void printDeclContext(toolchain::raw_ostream &out, const DeclContext *dc) {
    if (!dc) return;
    if (auto module = dyn_cast<ModuleDecl>(dc)) {
      out << module->getName() << ".";
    } else if (auto extension = dyn_cast<ExtensionDecl>(dc)) {
      auto *extended = extension->getExtendedNominal();
      if (extended) {
        printDecl(out, extended);
        out << ".";
      }
    } else if (auto decl = dyn_cast_or_null<ValueDecl>(dc->getAsDecl())) {
      printDecl(out, decl);
      out << ".";
    } else {
      printDeclContext(out, dc->getParent());
    }
  }
};

}

bool ASTScript::execute() const {
  // Hardcode the actual query we want to execute here.

  auto &ctx = Config.Compiler.getASTContext();
  auto languageUI = ctx.getLoadedModule(ctx.getIdentifier("CodiraUI"));
  if (!languageUI) {
    toolchain::errs() << "error: CodiraUI module not loaded\n";
    return true;
  }

  auto descriptor =
      UnqualifiedLookupDescriptor(DeclNameRef(ctx.getIdentifier("View")),
                                  languageUI);
  auto viewLookup = evaluateOrDefault(ctx.evaluator,
                                      UnqualifiedLookupRequest{descriptor}, {});
  auto viewProtocol =
    dyn_cast_or_null<ProtocolDecl>(viewLookup.getSingleTypeResult());
  if (!viewProtocol) {
    toolchain::errs() << "error: couldn't find CodiraUI.View protocol\n";
    return true;
  }

  SmallVector<Decl*, 128> topLevelDecls;
  languageUI->getTopLevelDecls(topLevelDecls);

  toolchain::errs() << "found " << topLevelDecls.size() << " top-level declarations\n";

  ASTScriptWalker walker(*this, viewProtocol);
  for (auto decl : topLevelDecls) {
    decl->walk(walker);
  }

  return false;
}
