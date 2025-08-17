//===--------------------------------------------------------------*- C++ -*--//
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

#include "NoOwnershipChangeVisitor.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugReporterVisitors.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ExplodedGraph.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState_Fwd.h"
#include "toolchain/ADT/SetOperations.h"

using namespace language::Core;
using namespace ento;
using OwnerSet = NoOwnershipChangeVisitor::OwnerSet;

namespace {
// Collect which entities point to the allocated memory, and could be
// responsible for deallocating it.
class OwnershipBindingsHandler : public StoreManager::BindingsHandler {
  SymbolRef Sym;
  OwnerSet &Owners;

public:
  OwnershipBindingsHandler(SymbolRef Sym, OwnerSet &Owners)
      : Sym(Sym), Owners(Owners) {}

  bool HandleBinding(StoreManager &SMgr, Store Store, const MemRegion *Region,
                     SVal Val) override {
    if (Val.getAsSymbol() == Sym)
      Owners.insert(Region);
    return true;
  }

  LLVM_DUMP_METHOD void dump() const { dumpToStream(toolchain::errs()); }
  LLVM_DUMP_METHOD void dumpToStream(toolchain::raw_ostream &out) const {
    out << "Owners: {\n";
    for (const MemRegion *Owner : Owners) {
      out << "  ";
      Owner->dumpToStream(out);
      out << ",\n";
    }
    out << "}\n";
  }
};
} // namespace

OwnerSet NoOwnershipChangeVisitor::getOwnersAtNode(const ExplodedNode *N) {
  OwnerSet Ret;

  ProgramStateRef State = N->getState();
  OwnershipBindingsHandler Handler{Sym, Ret};
  State->getStateManager().getStoreManager().iterBindings(State->getStore(),
                                                          Handler);
  return Ret;
}

LLVM_DUMP_METHOD std::string
NoOwnershipChangeVisitor::getFunctionName(const ExplodedNode *CallEnterN) {
  if (const CallExpr *CE = toolchain::dyn_cast_or_null<CallExpr>(
          CallEnterN->getLocationAs<CallEnter>()->getCallExpr()))
    if (const FunctionDecl *FD = CE->getDirectCallee())
      return FD->getQualifiedNameAsString();
  return "";
}

bool NoOwnershipChangeVisitor::wasModifiedInFunction(
    const ExplodedNode *CallEnterN, const ExplodedNode *CallExitEndN) {
  const Decl *Callee =
      CallExitEndN->getFirstPred()->getLocationContext()->getDecl();
  if (!doesFnIntendToHandleOwnership(
          Callee,
          CallExitEndN->getState()->getAnalysisManager().getASTContext()))
    return true;

  if (hasResourceStateChanged(CallEnterN->getState(), CallExitEndN->getState()))
    return true;

  OwnerSet CurrOwners = getOwnersAtNode(CallEnterN);
  OwnerSet ExitOwners = getOwnersAtNode(CallExitEndN);

  // Owners in the current set may be purged from the analyzer later on.
  // If a variable is dead (is not referenced directly or indirectly after
  // some point), it will be removed from the Store before the end of its
  // actual lifetime.
  // This means that if the ownership status didn't change, CurrOwners
  // must be a superset of, but not necessarily equal to ExitOwners.
  return !toolchain::set_is_subset(ExitOwners, CurrOwners);
}

PathDiagnosticPieceRef NoOwnershipChangeVisitor::maybeEmitNoteForParameters(
    PathSensitiveBugReport &R, const CallEvent &Call, const ExplodedNode *N) {
  // TODO: Factor the logic of "what constitutes as an entity being passed
  // into a function call" out by reusing the code in
  // NoStoreFuncVisitor::maybeEmitNoteForParameters, maybe by incorporating
  // the printing technology in UninitializedObject's FieldChainInfo.
  ArrayRef<ParmVarDecl *> Parameters = Call.parameters();
  for (unsigned I = 0; I < Call.getNumArgs() && I < Parameters.size(); ++I) {
    SVal V = Call.getArgSVal(I);
    if (V.getAsSymbol() == Sym)
      return emitNote(N);
  }
  return nullptr;
}
