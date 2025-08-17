//===--- RecursiveSymbolVisitor.h - Clang refactoring library -------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// A wrapper class around \c RecursiveASTVisitor that visits each
/// occurrences of a named symbol.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_RECURSIVESYMBOLVISITOR_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_RECURSIVESYMBOLVISITOR_H

#include "language/Core/AST/AST.h"
#include "language/Core/AST/RecursiveASTVisitor.h"
#include "language/Core/Lex/Lexer.h"

namespace language::Core {
namespace tooling {

/// Traverses the AST and visits the occurrence of each named symbol in the
/// given nodes.
template <typename T>
class RecursiveSymbolVisitor
    : public RecursiveASTVisitor<RecursiveSymbolVisitor<T>> {
  using BaseType = RecursiveASTVisitor<RecursiveSymbolVisitor<T>>;

public:
  RecursiveSymbolVisitor(const SourceManager &SM, const LangOptions &LangOpts)
      : SM(SM), LangOpts(LangOpts) {}

  bool visitSymbolOccurrence(const NamedDecl *ND,
                             ArrayRef<SourceRange> NameRanges) {
    return true;
  }

  // Declaration visitors:

  bool VisitNamedDecl(const NamedDecl *D) {
    return isa<CXXConversionDecl>(D) ? true : visit(D, D->getLocation());
  }

  bool VisitCXXConstructorDecl(const CXXConstructorDecl *CD) {
    for (const auto *Initializer : CD->inits()) {
      // Ignore implicit initializers.
      if (!Initializer->isWritten())
        continue;
      if (const FieldDecl *FD = Initializer->getMember()) {
        if (!visit(FD, Initializer->getSourceLocation(),
                   Lexer::getLocForEndOfToken(Initializer->getSourceLocation(),
                                              0, SM, LangOpts)))
          return false;
      }
    }
    return true;
  }

  // Expression visitors:

  bool VisitDeclRefExpr(const DeclRefExpr *Expr) {
    return visit(Expr->getFoundDecl(), Expr->getLocation());
  }

  bool VisitMemberExpr(const MemberExpr *Expr) {
    return visit(Expr->getFoundDecl().getDecl(), Expr->getMemberLoc());
  }

  bool VisitOffsetOfExpr(const OffsetOfExpr *S) {
    for (unsigned I = 0, E = S->getNumComponents(); I != E; ++I) {
      const OffsetOfNode &Component = S->getComponent(I);
      if (Component.getKind() == OffsetOfNode::Field) {
        if (!visit(Component.getField(), Component.getEndLoc()))
          return false;
      }
      // FIXME: Try to resolve dependent field references.
    }
    return true;
  }

  // Other visitors:

  bool VisitTypeLoc(const TypeLoc Loc) {
    const SourceLocation TypeBeginLoc = Loc.getBeginLoc();
    const SourceLocation TypeEndLoc =
        Lexer::getLocForEndOfToken(TypeBeginLoc, 0, SM, LangOpts);
    if (const auto *TemplateTypeParm =
            dyn_cast<TemplateTypeParmType>(Loc.getType())) {
      if (!visit(TemplateTypeParm->getDecl(), TypeBeginLoc, TypeEndLoc))
        return false;
    }
    if (const auto *TemplateSpecType =
            dyn_cast<TemplateSpecializationType>(Loc.getType())) {
      if (!visit(TemplateSpecType->getTemplateName().getAsTemplateDecl(),
                 TypeBeginLoc, TypeEndLoc))
        return false;
    }
    if (const Type *TP = Loc.getTypePtr()) {
      if (TP->getTypeClass() == language::Core::Type::Record)
        return visit(TP->getAsCXXRecordDecl(), TypeBeginLoc, TypeEndLoc);
    }
    return true;
  }

  bool VisitTypedefTypeLoc(TypedefTypeLoc TL) {
    const SourceLocation TypeEndLoc =
        Lexer::getLocForEndOfToken(TL.getBeginLoc(), 0, SM, LangOpts);
    return visit(TL.getDecl(), TL.getBeginLoc(), TypeEndLoc);
  }

  bool TraverseNestedNameSpecifierLoc(NestedNameSpecifierLoc QualifierLoc) {
    // The base visitor will visit NNSL prefixes, so we should only look at
    // the current NNS.
    if (NestedNameSpecifier Qualifier = QualifierLoc.getNestedNameSpecifier();
        Qualifier.getKind() == NestedNameSpecifier::Kind::Namespace) {
      const auto *ND = dyn_cast<NamespaceDecl>(
          Qualifier.getAsNamespaceAndPrefix().Namespace);
      if (!visit(ND, QualifierLoc.getLocalBeginLoc(),
                 QualifierLoc.getLocalEndLoc()))
        return false;
    }
    return BaseType::TraverseNestedNameSpecifierLoc(QualifierLoc);
  }

  bool VisitDesignatedInitExpr(const DesignatedInitExpr *E) {
    for (const DesignatedInitExpr::Designator &D : E->designators()) {
      if (D.isFieldDesignator()) {
        if (const FieldDecl *Decl = D.getFieldDecl()) {
          if (!visit(Decl, D.getFieldLoc(), D.getFieldLoc()))
            return false;
        }
      }
    }
    return true;
  }

private:
  const SourceManager &SM;
  const LangOptions &LangOpts;

  bool visit(const NamedDecl *ND, SourceLocation BeginLoc,
             SourceLocation EndLoc) {
    return static_cast<T *>(this)->visitSymbolOccurrence(
        ND, SourceRange(BeginLoc, EndLoc));
  }
  bool visit(const NamedDecl *ND, SourceLocation Loc) {
    return visit(ND, Loc, Lexer::getLocForEndOfToken(Loc, 0, SM, LangOpts));
  }
};

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_RECURSIVESYMBOLVISITOR_H
