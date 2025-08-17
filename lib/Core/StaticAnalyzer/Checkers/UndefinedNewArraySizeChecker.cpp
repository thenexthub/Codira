//===--- UndefinedNewArraySizeChecker.cpp -----------------------*- C++ -*--==//
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
// This defines UndefinedNewArraySizeChecker, a builtin check in ExprEngine
// that checks if the size of the array in a new[] expression is undefined.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace language::Core;
using namespace ento;

namespace {
class UndefinedNewArraySizeChecker : public Checker<check::PreCall> {

private:
  BugType BT{this, "Undefined array element count in new[]",
             categories::LogicError};

public:
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  void HandleUndefinedArrayElementCount(CheckerContext &C, SVal ArgVal,
                                        const Expr *Init,
                                        SourceRange Range) const;
};
} // namespace

void UndefinedNewArraySizeChecker::checkPreCall(const CallEvent &Call,
                                                CheckerContext &C) const {
  if (const auto *AC = dyn_cast<CXXAllocatorCall>(&Call)) {
    if (!AC->isArray())
      return;

    auto *SizeEx = *AC->getArraySizeExpr();
    auto SizeVal = AC->getArraySizeVal();

    if (SizeVal.isUndef())
      HandleUndefinedArrayElementCount(C, SizeVal, SizeEx,
                                       SizeEx->getSourceRange());
  }
}

void UndefinedNewArraySizeChecker::HandleUndefinedArrayElementCount(
    CheckerContext &C, SVal ArgVal, const Expr *Init, SourceRange Range) const {

  if (ExplodedNode *N = C.generateErrorNode()) {

    SmallString<100> buf;
    toolchain::raw_svector_ostream os(buf);

    os << "Element count in new[] is a garbage value";

    auto R = std::make_unique<PathSensitiveBugReport>(BT, os.str(), N);
    R->markInteresting(ArgVal);
    R->addRange(Range);
    bugreporter::trackExpressionValue(N, Init, *R);

    C.emitReport(std::move(R));
  }
}

void ento::registerUndefinedNewArraySizeChecker(CheckerManager &mgr) {
  mgr.registerChecker<UndefinedNewArraySizeChecker>();
}

bool ento::shouldRegisterUndefinedNewArraySizeChecker(
    const CheckerManager &mgr) {
  return mgr.getLangOpts().CPlusPlus;
}
