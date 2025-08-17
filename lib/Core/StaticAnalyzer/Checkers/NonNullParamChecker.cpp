//===--- NonNullParamChecker.cpp - Undefined arguments checker -*- C++ -*--===//
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
// This defines NonNullParamChecker, which checks for arguments expected not to
// be null due to:
//   - the corresponding parameters being declared to have nonnull attribute
//   - the corresponding parameters being references; since the call would form
//     a reference to a null pointer
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/Attr.h"
#include "language/Core/Analysis/AnyCall.h"
#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "toolchain/ADT/StringExtras.h"

using namespace language::Core;
using namespace ento;

namespace {
class NonNullParamChecker
    : public Checker<check::PreCall, check::BeginFunction,
                     EventDispatcher<ImplicitNullDerefEvent>> {
  const BugType BTAttrNonNull{
      this, "Argument with 'nonnull' attribute passed null", "API"};
  const BugType BTNullRefArg{this, "Dereference of null pointer"};

public:
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  void checkBeginFunction(CheckerContext &C) const;

  std::unique_ptr<PathSensitiveBugReport>
  genReportNullAttrNonNull(const ExplodedNode *ErrorN, const Expr *ArgE,
                           unsigned IdxOfArg) const;
  std::unique_ptr<PathSensitiveBugReport>
  genReportReferenceToNullPointer(const ExplodedNode *ErrorN,
                                  const Expr *ArgE) const;
};

template <class CallType>
void setBitsAccordingToFunctionAttributes(const CallType &Call,
                                          toolchain::SmallBitVector &AttrNonNull) {
  const Decl *FD = Call.getDecl();

  for (const auto *NonNull : FD->specific_attrs<NonNullAttr>()) {
    if (!NonNull->args_size()) {
      // Lack of attribute parameters means that all of the parameters are
      // implicitly marked as non-null.
      AttrNonNull.set();
      break;
    }

    for (const ParamIdx &Idx : NonNull->args()) {
      // 'nonnull' attribute's parameters are 1-based and should be adjusted to
      // match actual AST parameter/argument indices.
      unsigned IdxAST = Idx.getASTIndex();
      if (IdxAST >= AttrNonNull.size())
        continue;
      AttrNonNull.set(IdxAST);
    }
  }
}

template <class CallType>
void setBitsAccordingToParameterAttributes(const CallType &Call,
                                           toolchain::SmallBitVector &AttrNonNull) {
  for (const ParmVarDecl *Parameter : Call.parameters()) {
    unsigned ParameterIndex = Parameter->getFunctionScopeIndex();
    if (ParameterIndex == AttrNonNull.size())
      break;

    if (Parameter->hasAttr<NonNullAttr>())
      AttrNonNull.set(ParameterIndex);
  }
}

template <class CallType>
toolchain::SmallBitVector getNonNullAttrsImpl(const CallType &Call,
                                         unsigned ExpectedSize) {
  toolchain::SmallBitVector AttrNonNull(ExpectedSize);

  setBitsAccordingToFunctionAttributes(Call, AttrNonNull);
  setBitsAccordingToParameterAttributes(Call, AttrNonNull);

  return AttrNonNull;
}

/// \return Bitvector marking non-null attributes.
toolchain::SmallBitVector getNonNullAttrs(const CallEvent &Call) {
  return getNonNullAttrsImpl(Call, Call.getNumArgs());
}

/// \return Bitvector marking non-null attributes.
toolchain::SmallBitVector getNonNullAttrs(const AnyCall &Call) {
  return getNonNullAttrsImpl(Call, Call.param_size());
}
} // end anonymous namespace

void NonNullParamChecker::checkPreCall(const CallEvent &Call,
                                       CheckerContext &C) const {
  if (!Call.getDecl())
    return;

  toolchain::SmallBitVector AttrNonNull = getNonNullAttrs(Call);
  unsigned NumArgs = Call.getNumArgs();

  ProgramStateRef state = C.getState();
  ArrayRef<ParmVarDecl *> parms = Call.parameters();

  for (unsigned idx = 0; idx < NumArgs; ++idx) {
    // For vararg functions, a corresponding parameter decl may not exist.
    bool HasParam = idx < parms.size();

    // Check if the parameter is a reference. We want to report when reference
    // to a null pointer is passed as a parameter.
    bool HasRefTypeParam =
        HasParam ? parms[idx]->getType()->isReferenceType() : false;
    bool ExpectedToBeNonNull = AttrNonNull.test(idx);

    if (!ExpectedToBeNonNull && !HasRefTypeParam)
      continue;

    // If the value is unknown or undefined, we can't perform this check.
    const Expr *ArgE = Call.getArgExpr(idx);
    SVal V = Call.getArgSVal(idx);
    auto DV = V.getAs<DefinedSVal>();
    if (!DV)
      continue;

    assert(!HasRefTypeParam || isa<Loc>(*DV));

    // Process the case when the argument is not a location.
    if (ExpectedToBeNonNull && !isa<Loc>(*DV)) {
      // If the argument is a union type, we want to handle a potential
      // transparent_union GCC extension.
      if (!ArgE)
        continue;

      QualType T = ArgE->getType();
      const RecordType *UT = T->getAsUnionType();
      if (!UT || !UT->getOriginalDecl()
                      ->getMostRecentDecl()
                      ->hasAttr<TransparentUnionAttr>())
        continue;

      auto CSV = DV->getAs<nonloc::CompoundVal>();

      // FIXME: Handle LazyCompoundVals?
      if (!CSV)
        continue;

      V = *(CSV->begin());
      DV = V.getAs<DefinedSVal>();
      assert(++CSV->begin() == CSV->end());
      // FIXME: Handle (some_union){ some_other_union_val }, which turns into
      // a LazyCompoundVal inside a CompoundVal.
      if (!isa<Loc>(V))
        continue;

      // Retrieve the corresponding expression.
      if (const auto *CE = dyn_cast<CompoundLiteralExpr>(ArgE))
        if (const auto *IE = dyn_cast<InitListExpr>(CE->getInitializer()))
          ArgE = dyn_cast<Expr>(*(IE->begin()));
    }

    ConstraintManager &CM = C.getConstraintManager();
    ProgramStateRef stateNotNull, stateNull;
    std::tie(stateNotNull, stateNull) = CM.assumeDual(state, *DV);

    // Generate an error node.  Check for a null node in case
    // we cache out.
    if (stateNull && !stateNotNull) {
      if (ExplodedNode *errorNode = C.generateErrorNode(stateNull)) {

        std::unique_ptr<BugReport> R;
        if (ExpectedToBeNonNull)
          R = genReportNullAttrNonNull(errorNode, ArgE, idx + 1);
        else if (HasRefTypeParam)
          R = genReportReferenceToNullPointer(errorNode, ArgE);

        // Highlight the range of the argument that was null.
        R->addRange(Call.getArgSourceRange(idx));

        // Emit the bug report.
        C.emitReport(std::move(R));
      }

      // Always return.  Either we cached out or we just emitted an error.
      return;
    }

    if (stateNull) {
      if (ExplodedNode *N = C.generateSink(stateNull, C.getPredecessor())) {
        ImplicitNullDerefEvent event = {
            V, false, N, &C.getBugReporter(),
            /*IsDirectDereference=*/HasRefTypeParam};
        dispatchEvent(event);
      }
    }

    // If a pointer value passed the check we should assume that it is
    // indeed not null from this point forward.
    state = stateNotNull;
  }

  // If we reach here all of the arguments passed the nonnull check.
  // If 'state' has been updated generated a new node.
  C.addTransition(state);
}

/// We want to trust developer annotations and consider all 'nonnull' parameters
/// as non-null indeed. Each marked parameter will get a corresponding
/// constraint.
///
/// This approach will not only help us to get rid of some false positives, but
/// remove duplicates and shorten warning traces as well.
///
/// \code
///   void foo(int *x) [[gnu::nonnull]] {
///     // . . .
///     *x = 42;    // we don't want to consider this as an error...
///     // . . .
///   }
///
///   foo(nullptr); // ...and report here instead
/// \endcode
void NonNullParamChecker::checkBeginFunction(CheckerContext &Context) const {
  // Planned assumption makes sense only for top-level functions.
  // Inlined functions will get similar constraints as part of 'checkPreCall'.
  if (!Context.inTopFrame())
    return;

  const LocationContext *LocContext = Context.getLocationContext();

  const Decl *FD = LocContext->getDecl();
  // AnyCall helps us here to avoid checking for FunctionDecl and ObjCMethodDecl
  // separately and aggregates interfaces of these classes.
  auto AbstractCall = AnyCall::forDecl(FD);
  if (!AbstractCall)
    return;

  ProgramStateRef State = Context.getState();
  toolchain::SmallBitVector ParameterNonNullMarks = getNonNullAttrs(*AbstractCall);

  for (const ParmVarDecl *Parameter : AbstractCall->parameters()) {
    // 1. Check parameter if it is annotated as non-null
    if (!ParameterNonNullMarks.test(Parameter->getFunctionScopeIndex()))
      continue;

    // 2. Check that parameter is a pointer.
    //    Nonnull attribute can be applied to non-pointers (by default
    //    __attribute__(nonnull) implies "all parameters").
    if (!Parameter->getType()->isPointerType())
      continue;

    Loc ParameterLoc = State->getLValue(Parameter, LocContext);
    // We never consider top-level function parameters undefined.
    auto StoredVal =
        State->getSVal(ParameterLoc).castAs<DefinedOrUnknownSVal>();

    // 3. Assume that it is indeed non-null
    if (ProgramStateRef NewState = State->assume(StoredVal, true)) {
      State = NewState;
    }
  }

  Context.addTransition(State);
}

std::unique_ptr<PathSensitiveBugReport>
NonNullParamChecker::genReportNullAttrNonNull(const ExplodedNode *ErrorNode,
                                              const Expr *ArgE,
                                              unsigned IdxOfArg) const {
  toolchain::SmallString<256> SBuf;
  toolchain::raw_svector_ostream OS(SBuf);
  OS << "Null pointer passed to "
     << IdxOfArg << toolchain::getOrdinalSuffix(IdxOfArg)
     << " parameter expecting 'nonnull'";

  auto R =
      std::make_unique<PathSensitiveBugReport>(BTAttrNonNull, SBuf, ErrorNode);
  if (ArgE)
    bugreporter::trackExpressionValue(ErrorNode, ArgE, *R);

  return R;
}

std::unique_ptr<PathSensitiveBugReport>
NonNullParamChecker::genReportReferenceToNullPointer(
    const ExplodedNode *ErrorNode, const Expr *ArgE) const {
  auto R = std::make_unique<PathSensitiveBugReport>(
      BTNullRefArg, "Forming reference to null pointer", ErrorNode);
  if (ArgE) {
    const Expr *ArgEDeref = bugreporter::getDerefExpr(ArgE);
    if (!ArgEDeref)
      ArgEDeref = ArgE;
    bugreporter::trackExpressionValue(ErrorNode, ArgEDeref, *R);
  }
  return R;
}

void ento::registerNonNullParamChecker(CheckerManager &mgr) {
  mgr.registerChecker<NonNullParamChecker>();
}

bool ento::shouldRegisterNonNullParamChecker(const CheckerManager &mgr) {
  return true;
}
