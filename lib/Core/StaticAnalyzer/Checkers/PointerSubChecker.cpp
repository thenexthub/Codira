//=== PointerSubChecker.cpp - Pointer subtraction checker ------*- C++ -*--===//
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
// This files defines PointerSubChecker, a builtin checker that checks for
// pointer subtractions on two pointers pointing to different memory chunks.
// This check corresponds to CWE-469.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/FormatVariadic.h"

using namespace language::Core;
using namespace ento;

namespace {
class PointerSubChecker
  : public Checker< check::PreStmt<BinaryOperator> > {
  const BugType BT{this, "Pointer subtraction"};
  const toolchain::StringLiteral Msg_MemRegionDifferent =
      "Subtraction of two pointers that do not point into the same array "
      "is undefined behavior.";

public:
  void checkPreStmt(const BinaryOperator *B, CheckerContext &C) const;
};
}

void PointerSubChecker::checkPreStmt(const BinaryOperator *B,
                                     CheckerContext &C) const {
  // When doing pointer subtraction, if the two pointers do not point to the
  // same array, emit a warning.
  if (B->getOpcode() != BO_Sub)
    return;

  SVal LV = C.getSVal(B->getLHS());
  SVal RV = C.getSVal(B->getRHS());

  const MemRegion *LR = LV.getAsRegion();
  const MemRegion *RR = RV.getAsRegion();
  if (!LR || !RR)
    return;

  // Allow subtraction of identical pointers.
  if (LR == RR)
    return;

  // No warning if one operand is unknown or resides in a region that could be
  // equal to the other.
  if (LR->getSymbolicBase() || RR->getSymbolicBase())
    return;

  if (!B->getLHS()->getType()->isPointerType() ||
      !B->getRHS()->getType()->isPointerType())
    return;

  const auto *ElemLR = dyn_cast<ElementRegion>(LR);
  const auto *ElemRR = dyn_cast<ElementRegion>(RR);

  // Allow cases like "(&x + 1) - &x".
  if (ElemLR && ElemLR->getSuperRegion() == RR)
    return;
  // Allow cases like "&x - (&x + 1)".
  if (ElemRR && ElemRR->getSuperRegion() == LR)
    return;

  const ValueDecl *DiffDeclL = nullptr;
  const ValueDecl *DiffDeclR = nullptr;

  if (ElemLR && ElemRR) {
    const MemRegion *SuperLR = ElemLR->getSuperRegion();
    const MemRegion *SuperRR = ElemRR->getSuperRegion();
    if (SuperLR == SuperRR)
      return;
    // Allow arithmetic on different symbolic regions.
    if (isa<SymbolicRegion>(SuperLR) || isa<SymbolicRegion>(SuperRR))
      return;
    if (const auto *SuperDLR = dyn_cast<DeclRegion>(SuperLR))
      DiffDeclL = SuperDLR->getDecl();
    if (const auto *SuperDRR = dyn_cast<DeclRegion>(SuperRR))
      DiffDeclR = SuperDRR->getDecl();
  }

  if (ExplodedNode *N = C.generateNonFatalErrorNode()) {
    auto R =
        std::make_unique<PathSensitiveBugReport>(BT, Msg_MemRegionDifferent, N);
    R->addRange(B->getSourceRange());
    // The declarations may be identical even if the regions are different:
    //   struct { int array[10]; } a, b;
    //   do_something(&a.array[5] - &b.array[5]);
    // In this case don't emit notes.
    if (DiffDeclL != DiffDeclR) {
      auto AddNote = [&R, &C](const ValueDecl *D, StringRef SideStr) {
        if (D) {
          std::string Msg = toolchain::formatv(
              "{0} at the {1}-hand side of subtraction",
              D->getType()->isArrayType() ? "Array" : "Object", SideStr);
          R->addNote(Msg, {D, C.getSourceManager()});
        }
      };
      AddNote(DiffDeclL, "left");
      AddNote(DiffDeclR, "right");
    }
    C.emitReport(std::move(R));
  }
}

void ento::registerPointerSubChecker(CheckerManager &mgr) {
  mgr.registerChecker<PointerSubChecker>();
}

bool ento::shouldRegisterPointerSubChecker(const CheckerManager &mgr) {
  return true;
}
