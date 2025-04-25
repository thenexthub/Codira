//===--- Context.h --------------------------------------------------------===//
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

#ifndef SWIFT_SILOPTIMIZER_SEMANTICARC_CONTEXT_H
#define SWIFT_SILOPTIMIZER_SEMANTICARC_CONTEXT_H

#include "OwnershipLiveRange.h"
#include "SemanticARCOpts.h"

#include "language/Basic/BlotSetVector.h"
#include "language/Basic/FrozenMultiMap.h"
#include "language/Basic/MultiMapCache.h"
#include "language/SIL/BasicBlockUtils.h"
#include "language/SIL/OwnershipUtils.h"
#include "language/SIL/SILVisitor.h"
#include "language/SILOptimizer/Utils/InstOptUtils.h"
#include "language/SILOptimizer/Utils/ValueLifetime.h"
#include "llvm/Support/Compiler.h"

namespace language {
namespace semanticarc {

struct LLVM_LIBRARY_VISIBILITY Context {
  SILFunction &fn;
  SILPassManager *pm = nullptr;
  ARCTransformKind transformKind = ARCTransformKind::All;
  DeadEndBlocks &deadEndBlocks;
  ValueLifetimeAnalysis::Frontier lifetimeFrontier;
  SmallMultiMapCache<SILValue, Operand *> addressToExhaustiveWriteListCache;

  /// Are we assuming that we reached a fix point and are re-processing to
  /// prepare to use the phiToIncomingValueMultiMap.
  bool assumingAtFixedPoint = false;

  /// A map from a value that acts as a "joined owned introducer" in the def-use
  /// graph.
  ///
  /// A "joined owned introducer" is a value with owned ownership whose
  /// ownership is derived from multiple non-trivial owned operands of a related
  /// instruction. Some examples are phi arguments, tuples, structs. Naturally,
  /// all of these instructions must be non-unary instructions and only have
  /// this property if they have multiple operands that are non-trivial.
  ///
  /// In such a case, we can not just treat them like normal forwarding concepts
  /// since we can only eliminate optimize such a value if we are able to reason
  /// about all of its operands together jointly. This is not amenable to a
  /// small peephole analysis.
  ///
  /// Instead, as we perform the peephole analysis, using the multimap, we map
  /// each joined owned value introducer to the set of its @owned operands that
  /// we thought we could convert to guaranteed only if we could do the same to
  /// the joined owned value introducer. Then once we finish performing
  /// peepholes, we iterate through the map and see if any of our joined phi
  /// ranges had all of their operand's marked with this property by iterating
  /// over the multimap. Since we are dealing with owned values and we know that
  /// our LiveRange can not see through joined live ranges, we know that we
  /// should only be able to have a single owned value introducer for each
  /// consumed operand.
  ///
  /// NOTE: To work around potential invalidation of our consuming operands when
  /// adding values to edges on the CFG, we store our Operands as a
  /// SILBasicBlock and an operand number. We only add values to edges and never
  /// remove/modify edges so the operand number should be safe.
  struct ConsumingOperandState {
    PointerUnion<SILBasicBlock *, SILInstruction *> parent;
    unsigned operandNumber;

    ConsumingOperandState() : parent(nullptr), operandNumber(UINT_MAX) {}

    ConsumingOperandState(Operand *op)
        : parent(), operandNumber(op->getOperandNumber()) {
      if (auto *ti = dyn_cast<TermInst>(op->getUser())) {
        parent = ti->getParent();
      } else {
        parent = op->getUser();
      }
    }

    ConsumingOperandState(const ConsumingOperandState &other) :
        parent(other.parent), operandNumber(other.operandNumber) {}

    ConsumingOperandState &operator=(const ConsumingOperandState &other) {
      parent = other.parent;
      operandNumber = other.operandNumber;
      return *this;
    }

    ~ConsumingOperandState() = default;

    operator bool() const {
      return bool(parent) && operandNumber != UINT_MAX;
    }
  };

  FrozenMultiMap<SILValue, ConsumingOperandState>
      joinedOwnedIntroducerToConsumedOperands;

  /// If set to true, then we should only run cheap optimizations that do not
  /// build up data structures or analyze code in depth.
  ///
  /// As an example, we do not do load [copy] optimizations here since they
  /// generally involve more complex analysis, but simple peepholes of
  /// copy_values we /do/ allow.
  bool onlyMandatoryOpts;

  /// Callbacks that we must use to remove or RAUW values.
  InstModCallbacks instModCallbacks;

  using FrozenMultiMapRange =
      decltype(joinedOwnedIntroducerToConsumedOperands)::PairToSecondEltRange;

  DeadEndBlocks &getDeadEndBlocks() { return deadEndBlocks; }

  Context(SILFunction &fn, SILPassManager *pm, DeadEndBlocks &deBlocks, bool onlyMandatoryOpts,
          InstModCallbacks callbacks)
      : fn(fn), pm(pm), deadEndBlocks(deBlocks), lifetimeFrontier(),
        addressToExhaustiveWriteListCache(constructCacheValue),
        onlyMandatoryOpts(onlyMandatoryOpts), instModCallbacks(callbacks) {}

  void verify() const;

  bool shouldPerform(ARCTransformKind testKind) const {
    // When asserts are enabled, we allow for specific arc transforms to be
    // turned on/off via LLVM args. So check that if we have asserts, perform
    // all optimizations otherwise.
#ifndef NDEBUG
    if (transformKind == ARCTransformKind::Invalid)
      return false;
    return bool(testKind & transformKind);
#else
    return true;
#endif
  }

  void reset() {
    lifetimeFrontier.clear();
    addressToExhaustiveWriteListCache.clear();
    joinedOwnedIntroducerToConsumedOperands.reset();
  }

private:
  static bool
  constructCacheValue(SILValue initialValue,
                      SmallVectorImpl<Operand *> &wellBehavedWriteAccumulator);
};

} // namespace semanticarc
} // namespace language

#endif
