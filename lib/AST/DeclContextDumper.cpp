//===--- DeclContextDumper.cpp --------------------------------------------===//
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

#include "language/AST/ASTContext.h"
#include "language/AST/ASTWalker.h"
#include "language/AST/Attr.h"
#include "language/AST/Decl.h"
#include "language/AST/DeclContext.h"
#include "language/AST/Expr.h"
#include "language/AST/Pattern.h"
#include "language/AST/SourceFile.h"
#include "language/AST/Stmt.h"
#include "language/AST/TypeRepr.h"
#include "language/Subsystems.h"
#include "llvm/ADT/SetVector.h"

using namespace language;

namespace {

/// Collect 'DeclContext' hierarchy from AST.
class DeclContextHierarchyCollector : public ASTWalker {
  llvm::DenseMap<DeclContext *, llvm::SetVector<DeclContext *>> &Map;

public:
  DeclContextHierarchyCollector(
      llvm::DenseMap<DeclContext *, llvm::SetVector<DeclContext *>> &Map)
      : Map(Map) {}

  // Insert DC and its ascendants into the map.
  void handle(DeclContext *DC) {
    if (!DC)
      return;

    auto *parentDC = DC->getParent();
    if (Map[parentDC].insert(DC))
      handle(parentDC);
  }

  PreWalkAction walkToDeclPre(Decl *D) override {
    handle(D->getDeclContext());

    if (auto *dc = dyn_cast<DeclContext>(D)) {
      handle(dc);
    }

    if (auto *PBD = dyn_cast<PatternBindingDecl>(D)) {
      for (unsigned i = 0, e = PBD->getNumPatternEntries(); i != e; ++i)
        handle(PBD->getInitContext(i));
    }

    for (auto *attr : D->getAttrs().getAttributes<CustomAttr>()) {
      handle(attr->getInitContext());
    }

    return Action::Continue();
  }

  PreWalkResult<Expr *> walkToExprPre(Expr *E) override {
    if (auto *dc = dyn_cast<AbstractClosureExpr>(E)) {
      handle(dc);
    }
    switch (E->getKind()) {
    case ExprKind::SingleValueStmt:
      handle(cast<SingleValueStmtExpr>(E)->getDeclContext());
      break;
    case ExprKind::MacroExpansion:
      handle(cast<MacroExpansionExpr>(E)->getDeclContext());
      break;
    default:
      break;
    }
    return Action::Continue(E);
  }

  PreWalkResult<Stmt *> walkToStmtPre(Stmt *S) override {
    switch (S->getKind()) {
    case StmtKind::DoCatch:
      handle(cast<DoCatchStmt>(S)->getDeclContext());
      break;
    case StmtKind::Fallthrough:
      handle(cast<FallthroughStmt>(S)->getDeclContext());
      break;
    case StmtKind::Break:
      handle(cast<BreakStmt>(S)->getDeclContext());
      break;
    case StmtKind::Continue:
      handle(cast<ContinueStmt>(S)->getDeclContext());
      break;
    default:
      break;
    }
    return Action::Continue(S);
  }

  PreWalkResult<Pattern *> walkToPatternPre(Pattern *P) override {
    switch (P->getKind()) {
    case PatternKind::EnumElement:
      handle(cast<EnumElementPattern>(P)->getDeclContext());
      break;
    case PatternKind::Expr:
      handle(cast<ExprPattern>(P)->getDeclContext());
      break;
    default:
      break;
    }
    return Action::Continue(P);
  }

  PreWalkAction walkToTypeReprPre(TypeRepr *T) override {
    switch (T->getKind()) {
    case TypeReprKind::QualifiedIdent:
    case TypeReprKind::UnqualifiedIdent: {
      auto *tyR = cast<DeclRefTypeRepr>(T);
      if (tyR->isBound())
        handle(tyR->getDeclContext());
      break;
    }
    default:
      break;
    }
    return Action::Continue();
  }
};
} // namespace

void swift::dumpDeclContextHierarchy(llvm::raw_ostream &OS,
                                     SourceFile &SF) {
  llvm::DenseMap<DeclContext *, llvm::SetVector<DeclContext *>> map;

  DeclContextHierarchyCollector collector(map);
  SF.walk(collector);

  std::function<void(DeclContext *, size_t)> printChildrenDC =
      [&](DeclContext *parentDC, size_t indent) -> void {
    for (auto *DC : map[parentDC]) {
      DC->printContext(OS, indent, /*onlyAPartialLine=*/true);
      OS << "\n";
      printChildrenDC(DC, indent + 2);
    }
  };

  printChildrenDC(nullptr, 0);
}
