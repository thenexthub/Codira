//===--- PrettyStackTrace.cpp - Codira-specific PrettyStackTraceEntries ----===//
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
//  This file implements several Codira-specific implementations of
//  PrettyStackTraceEntry.
//
//===----------------------------------------------------------------------===//

#include "language/AST/ASTContext.h"
#include "language/AST/Decl.h"
#include "language/AST/Expr.h"
#include "language/AST/GenericSignature.h"
#include "language/AST/Module.h"
#include "language/AST/Pattern.h"
#include "language/AST/ProtocolConformance.h"
#include "language/AST/Stmt.h"
#include "language/AST/PrettyStackTrace.h"
#include "language/AST/TypeRepr.h"
#include "language/AST/TypeVisitor.h"
#include "language/Basic/SourceManager.h"
#include "clang/AST/Type.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/Support/MemoryBuffer.h"

using namespace language;

void PrettyStackTraceDecl::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << ' ';
  printDeclDescription(out, TheDecl);
}

void PrettyStackTraceDeclAndSubst::print(toolchain::raw_ostream &out) const {
  out << "While " << action << ' ';
  printDeclDescription(out, decl);

  out << "with substitution map: ";
  subst.dump(out);
}


void language::printDeclDescription(toolchain::raw_ostream &out, const Decl *D,
                                 bool addNewline) {
  if (!D) {
    out << "NULL declaration!";
    if (addNewline) out << '\n';
    return;
  }
  SourceLoc loc = D->getStartLoc();
  bool hasPrintedName = false;
  if (auto *named = dyn_cast<ValueDecl>(D)) {
    if (named->hasName()) {
      out << '\'' << named->getName() << '\'';
      hasPrintedName = true;
    } else if (auto *accessor = dyn_cast<AccessorDecl>(named)) {
      auto ASD = accessor->getStorage();
      if (ASD->hasName()) {
        switch (accessor->getAccessorKind()) {
        case AccessorKind::Get:
          out << "getter";
          break;
        case AccessorKind::DistributedGet:
          out << "_distributed_getter";
          break;
        case AccessorKind::Set:
          out << "setter";
          break;
        case AccessorKind::WillSet:
          out << "willset";
          break;
        case AccessorKind::DidSet:
          out << "didset";
          break;
        case AccessorKind::Address:
          out << "addressor";
          break;
        case AccessorKind::MutableAddress:
          out << "mutableAddressor";
          break;
        case AccessorKind::Read:
          out << "read";
          break;
        case AccessorKind::Modify:
          out << "modify";
          break;
        case AccessorKind::Init:
          out << "init";
          break;
        case AccessorKind::Modify2:
          out << "modify2";
          break;
        case AccessorKind::Read2:
          out << "read2";
          break;
        }

        out << " for " << ASD->getName();
        hasPrintedName = true;
        loc = ASD->getStartLoc();
      }
    }
  } else if (auto *extension = dyn_cast<ExtensionDecl>(D)) {
    Type extendedTy = extension->getExtendedType();
    if (extendedTy) {
      out << "extension of " << extendedTy;
      hasPrintedName = true;
    }
  }

  if (!hasPrintedName)
    out << "declaration " << (const void *)D;

  if (loc.isValid()) {
    out << " (at ";
    loc.print(out, D->getASTContext().SourceMgr);
    out << ')';
  } else {
    out << " (in module '" << D->getModuleContext()->getName() << "')";
  }
  if (addNewline) out << '\n';
}

void PrettyStackTraceAnyFunctionRef::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << ' ';
  if (auto *AFD = TheRef.getAbstractFunctionDecl()) {
    printDeclDescription(out, AFD);
  } else {
    auto *ACE = TheRef.getAbstractClosureExpr();
    printExprDescription(out, ACE, ACE->getASTContext());
  }
}

void PrettyStackTraceFreestandingMacroExpansion::print(
    toolchain::raw_ostream &out) const {
  out << "While " << Action << ' ';
  switch (Expansion->getFreestandingMacroKind()) {
  case FreestandingMacroKind::Expr: {
    auto &Context = Expansion->getDeclContext()->getASTContext();
    printExprDescription(out, cast<MacroExpansionExpr>(Expansion), Context);
    break;
  }
  case FreestandingMacroKind::Decl:
    printDeclDescription(out, cast<MacroExpansionDecl>(Expansion));
    break;
  }
}

void PrettyStackTraceExpr::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << ' ';
  if (!TheExpr) {
    out << "NULL expression!\n";
    return;
  }
  printExprDescription(out, TheExpr, Context);
}

void language::printExprDescription(toolchain::raw_ostream &out, const Expr *E,
                                 const ASTContext &Context, bool addNewline) {
  out << "expression at ";
  E->getSourceRange().print(out, Context.SourceMgr);
  if (addNewline) out << '\n';
}

void PrettyStackTraceStmt::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << ' ';
  if (!TheStmt) {
    out << "NULL statement!\n";
    return;
  }
  printStmtDescription(out, TheStmt, Context);
}

void language::printStmtDescription(toolchain::raw_ostream &out, Stmt *S,
                                 const ASTContext &Context, bool addNewline) {
  out << "statement at ";
  S->getSourceRange().print(out, Context.SourceMgr);
  if (addNewline) out << '\n';
}

void PrettyStackTracePattern::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << ' ';
  if (!ThePattern) {
    out << "NULL pattern!\n";
    return;
  }
  printPatternDescription(out, ThePattern, Context);
}

void language::printPatternDescription(toolchain::raw_ostream &out, Pattern *P,
                                    const ASTContext &Context,
                                    bool addNewline) {
  out << "pattern at ";
  P->getSourceRange().print(out, Context.SourceMgr);
  if (addNewline) out << '\n';
}

namespace {
  /// Map a Type to an interesting declaration whose source range we
  /// should print.
  struct InterestingDeclForType
      : TypeVisitor<InterestingDeclForType, Decl*> {
    Decl *visitType(TypeBase *type) {
      return nullptr;
    }
    Decl *visitUnboundGenericType(UnboundGenericType *type) {
      return type->getDecl();
    }
    Decl *visitBoundGenericType(BoundGenericType *type) {
      return type->getDecl();
    }
    Decl *visitNominalType(NominalType *type) {
      return type->getDecl();
    }
    Decl *visitTypeAliasType(TypeAliasType *type) {
      return type->getDecl();
    }
  };
} // end anonymous namespace

void PrettyStackTraceType::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << ' ';
  if (TheType.isNull()) {
    out << "NULL type!\n";
    return;
  }
  printTypeDescription(out, TheType, Context);
}

void language::printTypeDescription(toolchain::raw_ostream &out, Type type,
                                 const ASTContext &Context, bool addNewline) {
  out << "type '" << type << '\'';
  if (Decl *decl = InterestingDeclForType().visit(type)) {
    if (decl->getSourceRange().isValid()) {
      out << " (declared at ";
      decl->getSourceRange().print(out, Context.SourceMgr);
      out << ')';
    }
  }
  if (addNewline) out << '\n';
}

void PrettyStackTraceClangType::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << ' ';
  if (TheType == nullptr) {
    out << "NULL clang type!\n";
    return;
  }
  TheType->dump(out, Context);
}

void PrettyStackTraceTypeRepr::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << " type ";
  TheType->print(out);
  if (TheType && TheType->getSourceRange().isValid()) {
    out << " at ";
    TheType->getSourceRange().print(out, Context.SourceMgr);
  }
  out << '\n';
}

void PrettyStackTraceConformance::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << ' ';
  auto &Context = Conformance->getDeclContext()->getASTContext();
  printConformanceDescription(out, Conformance, Context);
}

void language::printConformanceDescription(toolchain::raw_ostream &out,
                                        const ProtocolConformance *conformance,
                                        const ASTContext &ctxt,
                                        bool addNewline) {
  if (!conformance) {
    out << "NULL protocol conformance!";
    if (addNewline) out << '\n';
    return;
  }

  out << "protocol conformance "
      << conformance->getType() << ": "
      << conformance->getProtocol()->getName() << " at ";
  auto *decl = conformance->getDeclContext()->getInnermostDeclarationDeclContext();
  printDeclDescription(out, decl, addNewline);
}

void language::printSourceLocDescription(toolchain::raw_ostream &out,
                                      SourceLoc loc, const ASTContext &ctx,
                                      bool addNewline) {
  loc.print(out, ctx.SourceMgr);
  if (addNewline) out << '\n';
}

void PrettyStackTraceLocation::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << " starting at ";
  printSourceLocDescription(out, Loc, Context);
}

void PrettyStackTraceGenericSignature::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << " generic signature ";
  GenericSig->print(out);
  if (Requirement) {
    out << " in requirement #" << *Requirement;
  }
  out << '\n';
}

void PrettyStackTraceSelector::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << " '" << Selector << "'";
}

void PrettyStackTraceDifferentiabilityWitness::print(
    toolchain::raw_ostream &out) const {
  out << "While " << Action << ' ';
  printDifferentiabilityWitnessDescription(out, Key);
}

void language::printDifferentiabilityWitnessDescription(
    toolchain::raw_ostream &out, const SILDifferentiabilityWitnessKey key,
    bool addNewline) {
  key.print(out);
  if (addNewline)
    out << '\n';
}

void PrettyStackTraceDeclContext::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << " in decl context:\n";
  out << "    ---\n";
  DC->printContext(out, /*indent=*/4);
  out << "    ---\n";
}
