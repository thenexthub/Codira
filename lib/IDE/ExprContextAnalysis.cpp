//===--- ExprContextAnalysis.cpp - Expession context analysis -------------===//
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

#include "ExprContextAnalysis.h"
#include "language/AST/ASTContext.h"
#include "language/AST/ASTVisitor.h"
#include "language/AST/ASTWalker.h"
#include "language/AST/Decl.h"
#include "language/AST/DeclContext.h"
#include "language/AST/Expr.h"
#include "language/AST/GenericSignature.h"
#include "language/AST/Initializer.h"
#include "language/AST/LazyResolver.h"
#include "language/AST/Module.h"
#include "language/AST/NameLookup.h"
#include "language/AST/ParameterList.h"
#include "language/AST/Pattern.h"
#include "language/AST/SourceFile.h"
#include "language/AST/Stmt.h"
#include "language/AST/Type.h"
#include "language/AST/Types.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/SourceManager.h"
#include "language/IDE/CodeCompletionResult.h"
#include "language/Sema/IDETypeChecking.h"
#include "language/Subsystems.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "toolchain/ADT/SmallSet.h"

using namespace language;
using namespace ide;

//===----------------------------------------------------------------------===//
// typeCheckContextAt(DeclContext, SourceLoc)
//===----------------------------------------------------------------------===//

void language::ide::typeCheckContextAt(TypeCheckASTNodeAtLocContext TypeCheckCtx,
                                    SourceLoc Loc) {
  // Make sure the extension has been bound.
  auto DC = TypeCheckCtx.getDeclContext();
  // Even if the extension is invalid (e.g. nested in a function or another
  // type), we want to know the "intended nominal" of the extension so that
  // we can know the type of 'Self'.
  SmallVector<ExtensionDecl *, 1> extensions;
  for (auto typeCtx = DC->getInnermostTypeContext(); typeCtx != nullptr;
       typeCtx = typeCtx->getParent()->getInnermostTypeContext()) {
    if (auto *ext = dyn_cast<ExtensionDecl>(typeCtx))
      extensions.push_back(ext);
  }
  while (!extensions.empty()) {
    extensions.back()->computeExtendedNominal();
    extensions.pop_back();
  }

  // If the completion happens in the inheritance clause of the extension,
  // 'DC' is the parent of the extension. We need to iterate the top level
  // decls to find it. In theory, we don't need the extended nominal in the
  // inheritance clause, but ASTScope lookup requires that. We don't care
  // unless 'DC' is not 'SourceFile' because non-toplevel extensions are
  // 'canNeverBeBound()' anyway.
  if (auto *SF = dyn_cast<SourceFile>(DC)) {
    auto &SM = DC->getASTContext().SourceMgr;
    for (auto *decl : SF->getTopLevelDecls())
      if (auto *ext = dyn_cast<ExtensionDecl>(decl))
        if (SM.rangeContainsTokenLoc(ext->getSourceRange(), Loc))
          ext->computeExtendedNominal();
  }

  language::typeCheckASTNodeAtLoc(TypeCheckCtx, Loc);
}

//===----------------------------------------------------------------------===//
// findParsedExpr(DeclContext, Expr)
//===----------------------------------------------------------------------===//

namespace {
class ExprFinder : public ASTWalker {
  SourceManager &SM;
  SourceRange TargetRange;
  Expr *FoundExpr = nullptr;

  template <typename NodeType> bool isInterstingRange(NodeType *Node) {
    return SM.rangeContains(Node->getSourceRange(), TargetRange);
  }

  bool shouldIgnore(Expr *E) {
    // E.g. instanceOfDerived.methodInBaseReturningSelf().#^HERE^#'
    // When calling a method in a base class returning 'Self', the call
    // expression itself has the type of the base class. That is wrapped with
    // CovariantReturnConversionExpr which downcasts it to the derived class.
    if (isa<CovariantReturnConversionExpr>(E))
      return false;

    // E.g. TypeName(#^HERE^#
    // In this case, we want the type expression instead of a reference to the
    // initializer.
    if (isa<ConstructorRefCallExpr>(E))
      return true;

    // Ignore other implicit expression.
    if (E->isImplicit())
      return true;

    return false;
  }

public:
  ExprFinder(SourceManager &SM, SourceRange TargetRange)
      : SM(SM), TargetRange(TargetRange) {}

  Expr *get() const { return FoundExpr; }

  MacroWalking getMacroWalkingBehavior() const override {
    return MacroWalking::ArgumentsAndExpansion;
  }

  PreWalkResult<Expr *> walkToExprPre(Expr *E) override {
    if (TargetRange == E->getSourceRange() && !shouldIgnore(E)) {
      assert(!FoundExpr && "non-nullptr for found expr");
      FoundExpr = E;
      return Action::Stop();
    }
    return Action::VisitNodeIf(isInterstingRange(E), E);
  }

  PreWalkResult<Pattern *> walkToPatternPre(Pattern *P) override {
    return Action::VisitNodeIf(isInterstingRange(P), P);
  }

  PreWalkResult<Stmt *> walkToStmtPre(Stmt *S) override {
    return Action::VisitNodeIf(isInterstingRange(S), S);
  }

  PreWalkAction walkToTypeReprPre(TypeRepr *T) override {
    return Action::SkipNode();
  }
};
} // anonymous namespace

Expr *language::ide::findParsedExpr(const DeclContext *DC,
                                 SourceRange TargetRange) {
  ExprFinder finder(DC->getASTContext().SourceMgr, TargetRange);
  const_cast<DeclContext *>(DC)->walkContext(finder);
  return finder.get();
}
