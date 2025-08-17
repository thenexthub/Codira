//===- Environment.cpp - Map from Stmt* to Locations/Values ---------------===//
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
//
//  This file defined the Environment and EnvironmentManager classes.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Core/PathSensitive/Environment.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/AST/ExprCXX.h"
#include "language/Core/AST/PrettyPrinter.h"
#include "language/Core/AST/Stmt.h"
#include "language/Core/AST/StmtObjC.h"
#include "language/Core/Analysis/AnalysisDeclContext.h"
#include "language/Core/Basic/JsonSupport.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SValBuilder.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SymExpr.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SymbolManager.h"
#include "toolchain/ADT/ImmutableMap.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/raw_ostream.h"
#include <cassert>

using namespace language::Core;
using namespace ento;

static const Expr *ignoreTransparentExprs(const Expr *E) {
  E = E->IgnoreParens();

  switch (E->getStmtClass()) {
  case Stmt::OpaqueValueExprClass:
    if (const Expr *SE = cast<OpaqueValueExpr>(E)->getSourceExpr()) {
      E = SE;
      break;
    }
    return E;
  case Stmt::ExprWithCleanupsClass:
    E = cast<ExprWithCleanups>(E)->getSubExpr();
    break;
  case Stmt::ConstantExprClass:
    E = cast<ConstantExpr>(E)->getSubExpr();
    break;
  case Stmt::CXXBindTemporaryExprClass:
    E = cast<CXXBindTemporaryExpr>(E)->getSubExpr();
    break;
  case Stmt::SubstNonTypeTemplateParmExprClass:
    E = cast<SubstNonTypeTemplateParmExpr>(E)->getReplacement();
    break;
  default:
    // This is the base case: we can't look through more than we already have.
    return E;
  }

  return ignoreTransparentExprs(E);
}

static const Stmt *ignoreTransparentExprs(const Stmt *S) {
  if (const auto *E = dyn_cast<Expr>(S))
    return ignoreTransparentExprs(E);
  return S;
}

EnvironmentEntry::EnvironmentEntry(const Stmt *S, const LocationContext *L)
    : std::pair<const Stmt *,
                const StackFrameContext *>(ignoreTransparentExprs(S),
                                           L ? L->getStackFrame()
                                             : nullptr) {}

SVal Environment::lookupExpr(const EnvironmentEntry &E) const {
  const SVal* X = ExprBindings.lookup(E);
  if (X) {
    SVal V = *X;
    return V;
  }
  return UnknownVal();
}

SVal Environment::getSVal(const EnvironmentEntry &Entry,
                          SValBuilder& svalBuilder) const {
  const Stmt *S = Entry.getStmt();
  assert(!isa<ObjCForCollectionStmt>(S) &&
         "Use ExprEngine::hasMoreIteration()!");
  assert((isa<Expr, ReturnStmt>(S)) &&
         "Environment can only argue about Exprs, since only they express "
         "a value! Any non-expression statement stored in Environment is a "
         "result of a hack!");
  const LocationContext *LCtx = Entry.getLocationContext();

  switch (S->getStmtClass()) {
  case Stmt::CXXBindTemporaryExprClass:
  case Stmt::ExprWithCleanupsClass:
  case Stmt::GenericSelectionExprClass:
  case Stmt::ConstantExprClass:
  case Stmt::ParenExprClass:
  case Stmt::SubstNonTypeTemplateParmExprClass:
    toolchain_unreachable("Should have been handled by ignoreTransparentExprs");

  case Stmt::AddrLabelExprClass:
  case Stmt::CharacterLiteralClass:
  case Stmt::CXXBoolLiteralExprClass:
  case Stmt::CXXScalarValueInitExprClass:
  case Stmt::ImplicitValueInitExprClass:
  case Stmt::IntegerLiteralClass:
  case Stmt::ObjCBoolLiteralExprClass:
  case Stmt::CXXNullPtrLiteralExprClass:
  case Stmt::ObjCStringLiteralClass:
  case Stmt::StringLiteralClass:
  case Stmt::TypeTraitExprClass:
  case Stmt::SizeOfPackExprClass:
  case Stmt::PredefinedExprClass:
    // Known constants; defer to SValBuilder.
    return *svalBuilder.getConstantVal(cast<Expr>(S));

  case Stmt::ReturnStmtClass: {
    const auto *RS = cast<ReturnStmt>(S);
    if (const Expr *RE = RS->getRetValue())
      return getSVal(EnvironmentEntry(RE, LCtx), svalBuilder);
    return UndefinedVal();
  }

  // Handle all other Stmt* using a lookup.
  default:
    return lookupExpr(EnvironmentEntry(S, LCtx));
  }
}

Environment EnvironmentManager::bindExpr(Environment Env,
                                         const EnvironmentEntry &E,
                                         SVal V,
                                         bool Invalidate) {
  if (V.isUnknown()) {
    if (Invalidate)
      return Environment(F.remove(Env.ExprBindings, E));
    else
      return Env;
  }
  return Environment(F.add(Env.ExprBindings, E, V));
}

namespace {

class MarkLiveCallback final : public SymbolVisitor {
  SymbolReaper &SymReaper;

public:
  MarkLiveCallback(SymbolReaper &symreaper) : SymReaper(symreaper) {}

  bool VisitSymbol(SymbolRef sym) override {
    SymReaper.markLive(sym);
    return true;
  }

  bool VisitMemRegion(const MemRegion *R) override {
    SymReaper.markLive(R);
    return true;
  }
};

} // namespace

// removeDeadBindings:
//  - Remove subexpression bindings.
//  - Remove dead block expression bindings.
//  - Keep live block expression bindings:
//   - Mark their reachable symbols live in SymbolReaper,
//     see ScanReachableSymbols.
//   - Mark the region in DRoots if the binding is a loc::MemRegionVal.
Environment
EnvironmentManager::removeDeadBindings(Environment Env,
                                       SymbolReaper &SymReaper,
                                       ProgramStateRef ST) {
  // We construct a new Environment object entirely, as this is cheaper than
  // individually removing all the subexpression bindings (which will greatly
  // outnumber block-level expression bindings).
  Environment NewEnv = getInitialEnvironment();

  MarkLiveCallback CB(SymReaper);
  ScanReachableSymbols RSScaner(ST, CB);

  toolchain::ImmutableMapRef<EnvironmentEntry, SVal>
    EBMapRef(NewEnv.ExprBindings.getRootWithoutRetain(),
             F.getTreeFactory());

  // Iterate over the block-expr bindings.
  for (Environment::iterator I = Env.begin(), End = Env.end(); I != End; ++I) {
    const EnvironmentEntry &BlkExpr = I.getKey();
    SVal X = I.getData();

    const Expr *E = dyn_cast<Expr>(BlkExpr.getStmt());
    if (!E)
      continue;

    if (SymReaper.isLive(E, BlkExpr.getLocationContext())) {
      // Copy the binding to the new map.
      EBMapRef = EBMapRef.add(BlkExpr, X);

      // Mark all symbols in the block expr's value live.
      RSScaner.scan(X);
    }
  }

  NewEnv.ExprBindings = EBMapRef.asImmutableMap();
  return NewEnv;
}

void Environment::printJson(raw_ostream &Out, const ASTContext &Ctx,
                            const LocationContext *LCtx, const char *NL,
                            unsigned int Space, bool IsDot) const {
  Indent(Out, Space, IsDot) << "\"environment\": ";

  if (ExprBindings.isEmpty()) {
    Out << "null," << NL;
    return;
  }

  ++Space;
  if (!LCtx) {
    // Find the freshest location context.
    toolchain::SmallPtrSet<const LocationContext *, 16> FoundContexts;
    for (const auto &I : *this) {
      const LocationContext *LC = I.first.getLocationContext();
      if (FoundContexts.count(LC) == 0) {
        // This context is fresher than all other contexts so far.
        LCtx = LC;
        for (const LocationContext *LCI = LC; LCI; LCI = LCI->getParent())
          FoundContexts.insert(LCI);
      }
    }
  }

  assert(LCtx);

  Out << "{ \"pointer\": \"" << (const void *)LCtx->getStackFrame()
      << "\", \"items\": [" << NL;
  PrintingPolicy PP = Ctx.getPrintingPolicy();

  LCtx->printJson(Out, NL, Space, IsDot, [&](const LocationContext *LC) {
    // LCtx items begin
    bool HasItem = false;
    unsigned int InnerSpace = Space + 1;

    // Store the last ExprBinding which we will print.
    BindingsTy::iterator LastI = ExprBindings.end();
    for (BindingsTy::iterator I = ExprBindings.begin(); I != ExprBindings.end();
         ++I) {
      if (I->first.getLocationContext() != LC)
        continue;

      if (!HasItem) {
        HasItem = true;
        Out << '[' << NL;
      }

      const Stmt *S = I->first.getStmt();
      (void)S;
      assert(S != nullptr && "Expected non-null Stmt");

      LastI = I;
    }

    for (BindingsTy::iterator I = ExprBindings.begin(); I != ExprBindings.end();
         ++I) {
      if (I->first.getLocationContext() != LC)
        continue;

      const Stmt *S = I->first.getStmt();
      Indent(Out, InnerSpace, IsDot)
          << "{ \"stmt_id\": " << S->getID(Ctx) << ", \"kind\": \""
          << S->getStmtClassName() << "\", \"pretty\": ";
      S->printJson(Out, nullptr, PP, /*AddQuotes=*/true);

      Out << ", \"value\": ";
      I->second.printJson(Out, /*AddQuotes=*/true);

      Out << " }";

      if (I != LastI)
        Out << ',';
      Out << NL;
    }

    if (HasItem)
      Indent(Out, --InnerSpace, IsDot) << ']';
    else
      Out << "null ";
  });

  Indent(Out, --Space, IsDot) << "]}," << NL;
}
