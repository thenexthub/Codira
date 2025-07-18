//===--- ResultPlan.h -------------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_SILGEN_RESULTPLAN_H
#define LANGUAGE_SILGEN_RESULTPLAN_H

#include "Callee.h"
#include "ExecutorBreadcrumb.h"
#include "Initialization.h"
#include "ManagedValue.h"
#include "language/AST/Types.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Toolchain.h"
#include "language/SIL/SILLocation.h"
#include <memory>

namespace language {

class CanType;
class SILValue;

namespace Lowering {

class AbstractionPattern;
class Initialization;
class RValue;
class SILGenFunction;
class SGFContext;
class CalleeTypeInfo;

/// An abstract class for working with results.of applies.
class ResultPlan {
public:
  virtual RValue finish(SILGenFunction &SGF, SILLocation loc,
                        ArrayRef<ManagedValue> &directResults,
                        SILValue bridgedForeignError) = 0;

  virtual void finishAndAddTo(SILGenFunction &SGF, SILLocation loc,
                              ArrayRef<ManagedValue> &directResults,
                              SILValue bridgedForeignError,
                              RValue &result);

  virtual ~ResultPlan() = default;

  /// Defers the emission of the given breadcrumb until \p finish is invoked.
  virtual void deferExecutorBreadcrumb(ExecutorBreadcrumb &&breadcrumb) {
    toolchain_unreachable("this ResultPlan does not handle deferred breadcrumbs!");
  }

  virtual void
  gatherIndirectResultAddrs(SILGenFunction &SGF, SILLocation loc,
                            SmallVectorImpl<SILValue> &outList) const = 0;

  virtual std::optional<std::pair<ManagedValue, ManagedValue>>
  emitForeignErrorArgument(SILGenFunction &SGF, SILLocation loc) {
    return std::nullopt;
  }

  virtual ManagedValue
  emitForeignAsyncCompletionHandler(SILGenFunction &SGF,
                                    AbstractionPattern origFormalType,
                                    ManagedValue self, SILLocation loc) {
    return {};
  }
};

using ResultPlanPtr = std::unique_ptr<ResultPlan>;

/// The class for building result plans.
struct ResultPlanBuilder {
  SILGenFunction &SGF;
  SILLocation loc;
  const CalleeTypeInfo &calleeTypeInfo;

  /// A list of all of the results that we are tracking in reverse order. The
  /// reason that it is in reverse order is to allow us to simply traverse the
  /// list by popping values off the back.
  SmallVector<SILResultInfo, 8> allResults;

  ResultPlanBuilder(SILGenFunction &SGF, SILLocation loc,
                    const CalleeTypeInfo &calleeTypeInfo)
      : SGF(SGF), loc(loc), calleeTypeInfo(calleeTypeInfo),
        // We reverse the order so we can pop values off the back.
        allResults(toolchain::reverse(calleeTypeInfo.substFnType->getResults())) {}

  ResultPlanPtr build(Initialization *emitInto, AbstractionPattern origType,
                      CanType substType);
  ResultPlanPtr buildForScalar(Initialization *emitInto,
                               AbstractionPattern origType,
                               CanType substType,
                               SILResultInfo result);
  ResultPlanPtr buildForTuple(Initialization *emitInto,
                              AbstractionPattern origType,
                              CanType substType);
  ResultPlanPtr
  buildForPackExpansion(std::optional<ArrayRef<Initialization *>> inits,
                        AbstractionPattern origExpansionType,
                        CanTupleEltTypeArrayRef substTypes);
  ResultPlanPtr buildPackExpansionIntoPack(SILValue packAddr,
                                           CanPackType formalPackType,
                                           unsigned componentIndex,
                                           Initialization *init,
                                           AbstractionPattern origType);
  ResultPlanPtr buildScalarIntoPack(SILValue packAddr,
                                    CanPackType formalPackType,
                                    unsigned componentIndex,
                                    Initialization *init,
                                    AbstractionPattern origType);

  static ResultPlanPtr computeResultPlan(SILGenFunction &SGF,
                                         const CalleeTypeInfo &calleeTypeInfo,
                                         SILLocation loc,
                                         SGFContext evalContext);

  ~ResultPlanBuilder() {
    assert(allResults.empty() && "didn't consume all results!");
  }

private:
  ResultPlanPtr buildTopLevelResult(Initialization *init, SILLocation loc);
};

} // end namespace Lowering
} // end namespace language

#endif
