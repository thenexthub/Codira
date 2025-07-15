//===--- ASTScopePrinting.cpp - Codira Object-Oriented AST Scope -----------===//
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
/// This file implements the printing functions of the ASTScopeImpl ontology.
///
//===----------------------------------------------------------------------===//
#include "language/AST/ASTScope.h"

#include "language/AST/ASTContext.h"
#include "language/AST/ASTWalker.h"
#include "language/AST/Decl.h"
#include "language/AST/Expr.h"
#include "language/AST/GenericParamList.h"
#include "language/AST/Initializer.h"
#include "language/AST/LazyResolver.h"
#include "language/AST/Module.h"
#include "language/AST/ParameterList.h"
#include "language/AST/Pattern.h"
#include "language/AST/SourceFile.h"
#include "language/AST/Stmt.h"
#include "language/AST/TypeRepr.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/PrettyStackTrace.h"
#include "language/Basic/STLExtras.h"
#include "toolchain/Support/Compiler.h"
#include <algorithm>

using namespace language;
using namespace ast_scope;

#pragma mark dumping

void ASTScopeImpl::dump() const { print(toolchain::errs(), 0, false); }

void ASTScopeImpl::dumpParents() const { printParents(toolchain::errs()); }

void ASTScopeImpl::dumpOneScopeMapLocation(
    std::pair<unsigned, unsigned> lineColumn) {
  auto bufferID = getSourceFile()->getBufferID();
  SourceLoc loc = getSourceManager().getLocForLineCol(
      bufferID, lineColumn.first, lineColumn.second);
  if (loc.isInvalid())
    return;

  toolchain::errs() << "***Scope at " << lineColumn.first << ":" << lineColumn.second
               << "***\n";
  auto *parentModule = getSourceFile()->getParentModule();
  auto *locScope = findInnermostEnclosingScope(parentModule, loc, &toolchain::errs());
  locScope->print(toolchain::errs(), 0, false, false);

  namelookup::ASTScopeDeclGatherer gatherer;
  // Print the local bindings introduced by this scope.
  locScope->lookupLocalsOrMembers(gatherer);
  if (!gatherer.getDecls().empty()) {
    toolchain::errs() << "Local bindings: ";
    toolchain::interleave(
        gatherer.getDecls().begin(), gatherer.getDecls().end(),
        [&](ValueDecl *value) { toolchain::errs() << value->getName(); },
        [&]() { toolchain::errs() << " "; });
    toolchain::errs() << "\n";
  }
}

void ASTScopeImpl::abortWithVerificationError(
    toolchain::function_ref<void(toolchain::raw_ostream &)> messageFn) const {
  ABORT([&](auto &out) {
    out << "ASTScopeImpl verification error in source file '"
        << getSourceFile()->getFilename() << "':\n";
    messageFn(out);
  });
}

#pragma mark printing

void ASTScopeImpl::print(toolchain::raw_ostream &out, unsigned level, bool lastChild,
                         bool printChildren) const {
  // Indent for levels 2+.
  if (level > 1)
    out.indent((level - 1) * 2);

  // Print child marker and leading '-' for levels 1+.
  if (level > 0)
    out << (lastChild ? '`' : '|') << '-';

  out << getClassName();
  if (auto *a = addressForPrinting().getPtrOrNull())
    out << " " << a;
  out << ", ";
  if (auto *d = getDeclIfAny().getPtrOrNull()) {
    if (d->isImplicit())
      out << "implicit ";
  }
  printRange(out);
  out << " ";
  printSpecifics(out);
  out << "\n";

  if (printChildren) {
    for (unsigned i : indices(getChildren())) {
      getChildren()[i]->print(out, level + 1,
                              /*lastChild=*/i == getChildren().size() - 1);
    }
  }
}

static void printSourceRange(toolchain::raw_ostream &out, const SourceRange range,
                             SourceManager &SM) {
  if (range.isInvalid()) {
    out << "[invalid source range]";
    return;
  }

  auto startLineAndCol = SM.getPresumedLineAndColumnForLoc(range.Start);
  auto endLineAndCol = SM.getPresumedLineAndColumnForLoc(range.End);

  out << "[" << startLineAndCol.first << ":" << startLineAndCol.second << " - "
      << endLineAndCol.first << ":" << endLineAndCol.second << "]";
}

void ASTScopeImpl::printRange(toolchain::raw_ostream &out) const {
  SourceRange range = getSourceRangeOfThisASTNode(/*omitAssertions=*/true);
  printSourceRange(out, range, getSourceManager());
}

void ASTScopeImpl::printParents(toolchain::raw_ostream &out) const {
  SmallVector<const ASTScopeImpl *, 8> nodes;
  const ASTScopeImpl *cursor = this;
  do {
    nodes.push_back(cursor);
    cursor = cursor->getParent().getPtrOrNull();
  } while (cursor);

  std::reverse(nodes.begin(), nodes.end());

  for (auto i : indices(nodes)) {
    nodes[i]->print(out, i, /*lastChild=*/true, /*printChildren=*/false);
  }
}

#pragma mark printSpecifics


void ASTSourceFileScope::printSpecifics(
    toolchain::raw_ostream &out) const {
  out << "'" << SF->getFilename() << "'";
}

NullablePtr<const void> ASTScopeImpl::addressForPrinting() const {
  if (auto *p = getDeclIfAny().getPtrOrNull())
    return p;
  if (auto *p = getStmtIfAny().getPtrOrNull())
    return p;
  if (auto *p = getExprIfAny().getPtrOrNull())
    return p;
  return nullptr;
}

void GenericTypeOrExtensionScope::printSpecifics(toolchain::raw_ostream &out) const {
  if (shouldHaveABody() && !doesDeclHaveABody())
    out << "<no body>";
  else if (auto *n = getCorrespondingNominalTypeDecl().getPtrOrNull())
    out << "'" << n->getName() << "'";
  else
    out << "<no extended nominal?!>";
}

void GenericParamScope::printSpecifics(toolchain::raw_ostream &out) const {
  out << "param " << index;
  auto *genericTypeParamDecl = paramList->getParams()[index];
  out << " '";
  genericTypeParamDecl->print(out);
  out << "'";
}

void AbstractFunctionDeclScope::printSpecifics(toolchain::raw_ostream &out) const {
  out << "'" << decl->getName() << "'";
}

void AbstractPatternEntryScope::printSpecifics(toolchain::raw_ostream &out) const {
  out << "entry " << patternEntryIndex;
  getPattern()->forEachVariable([&](VarDecl *vd) {
    out << " '" << vd->getName() << "'";
  });
}

void SubscriptDeclScope::printSpecifics(toolchain::raw_ostream &out) const {
  decl->dumpRef(out);
}

void MacroDeclScope::printSpecifics(toolchain::raw_ostream &out) const {
  decl->dumpRef(out);
}

void MacroExpansionDeclScope::printSpecifics(toolchain::raw_ostream &out) const {
  out << decl->getMacroName();
}

void ConditionalClausePatternUseScope::printSpecifics(
    toolchain::raw_ostream &out) const {
  sec.getPattern()->print(out);
}

bool GenericTypeOrExtensionScope::doesDeclHaveABody() const { return false; }

bool IterableTypeScope::doesDeclHaveABody() const {
  return getBraces().Start != getBraces().End;
}

void ast_scope::simple_display(toolchain::raw_ostream &out,
                               const ASTScopeImpl *scope) {
  // Cannot call scope->print(out) because printing an ASTFunctionBodyScope
  // gets the source range which can cause a request to parse it.
  // That in turn causes the request dependency printing code to blow up
  // as the AnyRequest ends up with a null.
  out << scope->getClassName() << "\n";
}
