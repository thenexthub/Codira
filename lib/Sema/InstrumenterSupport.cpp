//===--- InstrumenterSupport.cpp - Instrumenter Support -------------------===//
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
//  This file implements the supporting functions for writing instrumenters of
//  the Codira AST.
//
//===----------------------------------------------------------------------===//

#include "InstrumenterSupport.h"
#include "language/AST/DiagnosticSuppression.h"
#include "language/AST/SourceFile.h"
#include "language/Demangling/Punycode.h"
#include "toolchain/Support/Path.h"

using namespace language;
using namespace language::instrumenter_support;

namespace {

class ErrorGatherer : public DiagnosticConsumer {
private:
  bool error = false;
  DiagnosticEngine &diags;

public:
  ErrorGatherer(DiagnosticEngine &diags) : diags(diags) {
    diags.addConsumer(*this);
  }
  ~ErrorGatherer() override { diags.takeConsumers(); }
  void handleDiagnostic(SourceManager &SM,
                        const DiagnosticInfo &Info) override {
    if (Info.Kind == language::DiagnosticKind::Error) {
      error = true;
    }
    DiagnosticEngine::formatDiagnosticText(toolchain::errs(), Info.FormatString,
                                           Info.FormatArgs);
    toolchain::errs() << "\n";
  }
  bool hadError() { return error; }
};


class ErrorFinder : public ASTWalker {
  bool error = false;

public:
  ErrorFinder() {}

  MacroWalking getMacroWalkingBehavior() const override {
    return MacroWalking::Expansion;
  }

  PreWalkResult<Expr *> walkToExprPre(Expr *E) override {
    if (isa<ErrorExpr>(E) || !E->getType() || E->getType()->hasError())
      error = true;

    return Action::StopIf(error, E);
  }
  PreWalkAction walkToDeclPre(Decl *D) override {
    if (auto *VD = dyn_cast<ValueDecl>(D)) {
      if (!VD->hasInterfaceType() || VD->getInterfaceType()->hasError())
        error = true;
    }
    return Action::StopIf(error);
  }
  bool hadError() { return error; }
};
} // end anonymous namespace

InstrumenterBase::InstrumenterBase(ASTContext &C, DeclContext *DC)
    : Context(C), TypeCheckDC(DC), CF(*this) {
  // Prefixes for module and file vars
  const std::string builtinPrefix = "__builtin";
  const std::string modulePrefix = "_pg_module_";
  const std::string filePrefix = "_pg_file_";

  // Setup Module identifier
  std::string moduleName =
      std::string(TypeCheckDC->getParentModule()->getName());
  Identifier moduleIdentifier =
      Context.getIdentifier(builtinPrefix + modulePrefix + moduleName);

  SmallVector<ValueDecl *, 1> results;
  TypeCheckDC->getParentModule()->lookupValue(
      moduleIdentifier, NLKind::UnqualifiedLookup, results);

  if (results.size() == 1)
    ModuleIdentifier = results.front()->createNameRef();

  // Setup File identifier
  StringRef filePath = TypeCheckDC->getParentSourceFile()->getFilename();
  StringRef fileName = toolchain::sys::path::stem(filePath);

  std::string filePunycodeName;
  Punycode::encodePunycodeUTF8(fileName, filePunycodeName, true);
  Identifier fileIdentifier =
      Context.getIdentifier(builtinPrefix + modulePrefix + moduleName +
                            filePrefix + filePunycodeName);

  results.clear();
  TypeCheckDC->getParentModule()->lookupValue(
      fileIdentifier, NLKind::UnqualifiedLookup, results);

  if (results.size() == 1)
    FileIdentifier = results.front()->createNameRef();
}

void InstrumenterBase::anchor() {}

bool InstrumenterBase::doTypeCheckImpl(ASTContext &Ctx, DeclContext *DC,
                                       Expr * &parsedExpr) {
  DiagnosticSuppression suppression(Ctx.Diags);
  ErrorGatherer errorGatherer(Ctx.Diags);

  TypeChecker::typeCheckExpression(parsedExpr, DC, /*contextualInfo=*/{});

  if (parsedExpr) {
    ErrorFinder errorFinder;
    parsedExpr->walk(errorFinder);
    if (!errorFinder.hadError() && !errorGatherer.hadError()) {
      return true;
    }
  }

  return false;
}

Expr *InstrumenterBase::buildIDArgumentExpr(std::optional<DeclNameRef> name,
                                            SourceRange SR) {
  if (!name)
    return IntegerLiteralExpr::createFromUnsigned(Context, 0, SR.End);

  return new (Context) UnresolvedDeclRefExpr(*name, DeclRefKind::Ordinary,
                                             DeclNameLoc(SR.End));
}
