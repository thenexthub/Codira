//===--- State.cpp - State chain for the VM and AST Walker ------*- C++ -*-===//
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

#include "State.h"
#include "Frame.h"
#include "Program.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/CXXInheritance.h"
#include "language/Core/AST/OptionalDiagnostic.h"

using namespace language::Core;
using namespace language::Core::interp;

State::~State() {}

OptionalDiagnostic State::FFDiag(SourceLocation Loc, diag::kind DiagId,
                                 unsigned ExtraNotes) {
  return diag(Loc, DiagId, ExtraNotes, false);
}

OptionalDiagnostic State::FFDiag(const Expr *E, diag::kind DiagId,
                                 unsigned ExtraNotes) {
  if (getEvalStatus().Diag)
    return diag(E->getExprLoc(), DiagId, ExtraNotes, false);
  setActiveDiagnostic(false);
  return OptionalDiagnostic();
}

OptionalDiagnostic State::FFDiag(const SourceInfo &SI, diag::kind DiagId,
                                 unsigned ExtraNotes) {
  if (getEvalStatus().Diag)
    return diag(SI.getLoc(), DiagId, ExtraNotes, false);
  setActiveDiagnostic(false);
  return OptionalDiagnostic();
}

OptionalDiagnostic State::CCEDiag(SourceLocation Loc, diag::kind DiagId,
                                  unsigned ExtraNotes) {
  // Don't override a previous diagnostic. Don't bother collecting
  // diagnostics if we're evaluating for overflow.
  if (!getEvalStatus().Diag || !getEvalStatus().Diag->empty()) {
    setActiveDiagnostic(false);
    return OptionalDiagnostic();
  }
  return diag(Loc, DiagId, ExtraNotes, true);
}

OptionalDiagnostic State::CCEDiag(const Expr *E, diag::kind DiagId,
                                  unsigned ExtraNotes) {
  return CCEDiag(E->getExprLoc(), DiagId, ExtraNotes);
}

OptionalDiagnostic State::CCEDiag(const SourceInfo &SI, diag::kind DiagId,
                                  unsigned ExtraNotes) {
  return CCEDiag(SI.getLoc(), DiagId, ExtraNotes);
}

OptionalDiagnostic State::Note(SourceLocation Loc, diag::kind DiagId) {
  if (!hasActiveDiagnostic())
    return OptionalDiagnostic();
  return OptionalDiagnostic(&addDiag(Loc, DiagId));
}

void State::addNotes(ArrayRef<PartialDiagnosticAt> Diags) {
  if (hasActiveDiagnostic())
    toolchain::append_range(*getEvalStatus().Diag, Diags);
}

DiagnosticBuilder State::report(SourceLocation Loc, diag::kind DiagId) {
  return getASTContext().getDiagnostics().Report(Loc, DiagId);
}

/// Add a diagnostic to the diagnostics list.
PartialDiagnostic &State::addDiag(SourceLocation Loc, diag::kind DiagId) {
  PartialDiagnostic PD(DiagId, getASTContext().getDiagAllocator());
  getEvalStatus().Diag->push_back(std::make_pair(Loc, PD));
  return getEvalStatus().Diag->back().second;
}

OptionalDiagnostic State::diag(SourceLocation Loc, diag::kind DiagId,
                               unsigned ExtraNotes, bool IsCCEDiag) {
  Expr::EvalStatus &EvalStatus = getEvalStatus();
  if (EvalStatus.Diag) {
    if (hasPriorDiagnostic()) {
      return OptionalDiagnostic();
    }

    unsigned CallStackNotes = getCallStackDepth() - 1;
    unsigned Limit =
        getASTContext().getDiagnostics().getConstexprBacktraceLimit();
    if (Limit)
      CallStackNotes = std::min(CallStackNotes, Limit + 1);
    if (checkingPotentialConstantExpression())
      CallStackNotes = 0;

    setActiveDiagnostic(true);
    setFoldFailureDiagnostic(!IsCCEDiag);
    EvalStatus.Diag->clear();
    EvalStatus.Diag->reserve(1 + ExtraNotes + CallStackNotes);
    addDiag(Loc, DiagId);
    if (!checkingPotentialConstantExpression()) {
      addCallStack(Limit);
    }
    return OptionalDiagnostic(&(*EvalStatus.Diag)[0].second);
  }
  setActiveDiagnostic(false);
  return OptionalDiagnostic();
}

const LangOptions &State::getLangOpts() const {
  return getASTContext().getLangOpts();
}

void State::addCallStack(unsigned Limit) {
  // Determine which calls to skip, if any.
  unsigned ActiveCalls = getCallStackDepth() - 1;
  unsigned SkipStart = ActiveCalls, SkipEnd = SkipStart;
  if (Limit && Limit < ActiveCalls) {
    SkipStart = Limit / 2 + Limit % 2;
    SkipEnd = ActiveCalls - Limit / 2;
  }

  // Walk the call stack and add the diagnostics.
  unsigned CallIdx = 0;
  const Frame *Top = getCurrentFrame();
  const Frame *Bottom = getBottomFrame();
  for (const Frame *F = Top; F != Bottom; F = F->getCaller(), ++CallIdx) {
    SourceRange CallRange = F->getCallRange();
    assert(CallRange.isValid());

    // Skip this call?
    if (CallIdx >= SkipStart && CallIdx < SkipEnd) {
      if (CallIdx == SkipStart) {
        // Note that we're skipping calls.
        addDiag(CallRange.getBegin(), diag::note_constexpr_calls_suppressed)
            << unsigned(ActiveCalls - Limit);
      }
      continue;
    }

    // Use a different note for an inheriting constructor, because from the
    // user's perspective it's not really a function at all.
    if (const auto *CD =
            dyn_cast_if_present<CXXConstructorDecl>(F->getCallee());
        CD && CD->isInheritingConstructor()) {
      addDiag(CallRange.getBegin(),
              diag::note_constexpr_inherited_ctor_call_here)
          << CD->getParent();
      continue;
    }

    SmallString<128> Buffer;
    toolchain::raw_svector_ostream Out(Buffer);
    F->describe(Out);
    if (!Buffer.empty())
      addDiag(CallRange.getBegin(), diag::note_constexpr_call_here)
          << Out.str() << CallRange;
  }
}
