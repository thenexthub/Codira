//===--- CastOptimizer.h ----------------------------------*- C++ -*-------===//
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

#ifndef LANGUAGE_SILOPTIMIZER_UTILS_CASTOPTIMIZER_H
#define LANGUAGE_SILOPTIMIZER_UTILS_CASTOPTIMIZER_H

#include "language/Basic/ArrayRefView.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILCloner.h"
#include "language/SIL/SILInstruction.h"
#include "language/SILOptimizer/Analysis/ARCAnalysis.h"
#include "language/SILOptimizer/Analysis/ClassHierarchyAnalysis.h"
#include "language/SILOptimizer/Analysis/EpilogueARCAnalysis.h"
#include "language/SILOptimizer/Analysis/SimplifyInstruction.h"
#include "language/SILOptimizer/Utils/SILOptFunctionBuilder.h"
#include "toolchain/Support/Allocator.h"
#include <functional>
#include <utility>

namespace language {

class SILOptFunctionBuilder;
struct SILDynamicCastInst;

/// This is a helper class used to optimize casts.
class CastOptimizer {
  SILOptFunctionBuilder &functionBuilder;

  /// Temporary context for clients that do not provide their own.
  SILBuilderContext tempBuilderContext;

  /// Reference to the provided SILBuilderContext.
  SILBuilderContext &builderContext;

  /// Callback that replaces the first SILValue's uses with a use of the second
  /// value.
  std::function<void(SILValue, SILValue)> replaceValueUsesAction;

  /// Callback that replaces a SingleValueInstruction with a ValueBase after
  /// updating any status in the caller.
  std::function<void(SingleValueInstruction *, ValueBase *)>
      replaceInstUsesAction;

  /// Callback that erases an instruction and performs any state updates in the
  /// caller required.
  std::function<void(SILInstruction *)> eraseInstAction;

  /// Callback to call after an optimization was performed based on the fact
  /// that a cast will succeed.
  std::function<void()> willSucceedAction;

  /// Callback to call after an optimization was performed based on the fact
  /// that a cast will fail.
  std::function<void()> willFailAction;

public:
  CastOptimizer(SILOptFunctionBuilder &FunctionBuilder,
                SILBuilderContext *BuilderContext,
                std::function<void(SILValue, SILValue)> ReplaceValueUsesAction,
                std::function<void(SingleValueInstruction *, ValueBase *)>
                    ReplaceInstUsesAction,
                std::function<void(SILInstruction *)> EraseAction,
                std::function<void()> WillSucceedAction,
                std::function<void()> WillFailAction = []() {})
      : functionBuilder(FunctionBuilder),
        tempBuilderContext(FunctionBuilder.getModule()),
        builderContext(BuilderContext ? *BuilderContext : tempBuilderContext),
        replaceValueUsesAction(ReplaceValueUsesAction),
        replaceInstUsesAction(ReplaceInstUsesAction),
        eraseInstAction(EraseAction), willSucceedAction(WillSucceedAction),
        willFailAction(WillFailAction) {}

  // This constructor is used in
  // 'SILOptimizer/Mandatory/ConstantPropagation.cpp'. MSVC2015 compiler
  // couldn't use the single constructor version which has three default
  // arguments. It seems the number of the default argument with lambda is
  // limited.
  CastOptimizer(SILOptFunctionBuilder &FunctionBuilder,
                SILBuilderContext *BuilderContext,
                std::function<void(SILValue, SILValue)> ReplaceValueUsesAction,
                std::function<void(SingleValueInstruction *I, ValueBase *V)>
                    ReplaceInstUsesAction,
                std::function<void(SILInstruction *)> EraseAction =
                    [](SILInstruction *) {})
      : CastOptimizer(FunctionBuilder, BuilderContext, ReplaceValueUsesAction,
                      ReplaceInstUsesAction, EraseAction, []() {}, []() {}) {}

  /// Simplify checked_cast_br. It may change the control flow.
  SILInstruction *simplifyCheckedCastBranchInst(CheckedCastBranchInst *Inst);

  /// Simplify checked_cast_addr_br. It may change the control flow.
  SILInstruction *
  simplifyCheckedCastAddrBranchInst(CheckedCastAddrBranchInst *Inst);

  /// Optimize checked_cast_br. This cannot change the control flow.
  SILInstruction *optimizeCheckedCastBranchInst(CheckedCastBranchInst *Inst);

  /// Optimize checked_cast_addr_br. This cannot change the control flow.
  SILInstruction *
  optimizeCheckedCastAddrBranchInst(CheckedCastAddrBranchInst *Inst);

  /// Optimize unconditional_checked_cast. This cannot change the control flow.
  ValueBase *
  optimizeUnconditionalCheckedCastInst(UnconditionalCheckedCastInst *Inst);

  /// Optimize unconditional_checked_cast_addr. This cannot change the control
  /// flow.
  SILInstruction *optimizeUnconditionalCheckedCastAddrInst(
      UnconditionalCheckedCastAddrInst *Inst);

  /// Check if it is a bridged cast and optimize it.
  ///
  /// May change the control flow.
  SILInstruction *optimizeBridgedCasts(SILDynamicCastInst cast);

  /// Optimize a cast from a bridged ObjC type into
  /// a corresponding Codira type implementing _ObjectiveCBridgeable.
  SILInstruction *
  optimizeBridgedObjCToCodiraCast(SILDynamicCastInst dynamicCast);

  /// Optimize a cast from a Codira type implementing _ObjectiveCBridgeable
  /// into a bridged ObjC type.
  SILInstruction *
  optimizeBridgedCodiraToObjCCast(SILDynamicCastInst dynamicCast);

  void deleteInstructionsAfterUnreachable(SILInstruction *UnreachableInst,
                                          SILInstruction *TrapInst);

  SILValue optimizeMetatypeConversion(ConversionOperation mci,
                                      MetatypeRepresentation representation);
};

} // namespace language

#endif // LANGUAGE_SILOPTIMIZER_UTILS_CASTOPTIMIZER_H
