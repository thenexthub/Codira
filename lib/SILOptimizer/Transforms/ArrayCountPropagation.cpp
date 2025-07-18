//===--- ArrayCountPropagation.cpp - Propagate the count of arrays --------===//
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
#define DEBUG_TYPE "array-count-propagation"

#include "toolchain/ADT/SetVector.h"
#include "toolchain/ADT/SmallVector.h"
#include "language/Basic/Assertions.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Analysis/ArraySemantic.h"
#include "language/SIL/DebugUtils.h"

using namespace language;

/// Propagate the count of array values to calls of the array's count method.
///
/// Array literal construction and array initialization of array values
/// associates a count with the array value. This count can be propagated to the
/// count method if we can prove that the array value has not changed until
/// reading the array value's count.

/// Propagation of the count of one array allocation.
///
/// We propagate the count parameter to calls of
///
/// * Array.init(count:repeatedValue:)
/// * Array._adoptStorage(storage:count:)
/// * Array._allocateUninitialized(count:)
///
/// To Array.count users of the returned array value if we can prove that the
/// array value is not modified.
///
/// Currently, we recursively look at all array value uses and if any use could
/// escape or change the array value we give up. This is flow insensitive.
namespace {
class ArrayAllocation {
  /// The array allocation call.
  ApplyInst *Alloc;
  /// The array value returned by the allocation call.
  SILValue ArrayValue;
  /// The count of the allocated array.
  SILValue ArrayCount;
  // The calls to Array.count that use this array allocation.
  toolchain::SmallSetVector<ApplyInst *, 16> CountCalls;
  // Array count calls that are dead as a consequence of propagating the count
  // value.
  toolchain::SmallVectorImpl<ApplyInst *> &DeadArrayCountCalls;

  ArrayAllocation(ApplyInst *AI, toolchain::SmallVectorImpl<ApplyInst *> &DeadCalls)
      : Alloc(AI), DeadArrayCountCalls(DeadCalls) {}

  bool propagate();
  bool isInitializationWithKnownCount();
  bool analyzeArrayValueUses();
  bool recursivelyCollectUses(ValueBase *Def);
  bool propagateCountToUsers();

public:
  static bool tryPropagate(ApplyInst *Inst,
                           toolchain::SmallVectorImpl<ApplyInst *> &DeadCalls) {
    return ArrayAllocation(Inst, DeadCalls).propagate();
  }
};
} // end anonymous namespace

/// Propagate the count of an array created to count method calls on the same
/// array.
///
/// We have to prove that the size of the array value is not changed in between
/// the creation and the method call to count.
bool ArrayAllocation::propagate() {
  if (!isInitializationWithKnownCount())
    return false;

  // The array value was stored or has escaped.
  if (!analyzeArrayValueUses())
    return false;

  // No count users.
  if (CountCalls.empty())
    return false;

  return propagateCountToUsers();
}

/// Check that we have an array initialization call with a known count.
///
/// The returned array value is known not to be aliased since it was just
/// allocated.
bool ArrayAllocation::isInitializationWithKnownCount() {
  ArraySemanticsCall Uninitialized(Alloc, "array.uninitialized");
  if (Uninitialized &&
      (ArrayCount = Uninitialized.getInitializationCount()) &&
      (ArrayValue = Uninitialized.getArrayValue()))
    return true;

  ArraySemanticsCall Init(Alloc, "array.init", /*matchPartialName*/true);
  if (Init &&
      (ArrayCount = Init.getInitializationCount()) &&
      (ArrayValue = Init.getArrayValue()))
    return true;

  return false;
}

/// Collect all getCount users and check that there are no escapes or uses that
/// could change the array value.
bool ArrayAllocation::analyzeArrayValueUses() {
  return recursivelyCollectUses(ArrayValue);
}

/// Recursively look at all uses of this definition. Abort if the array value
/// could escape or be changed. Collect all uses that are calls to array.count.
bool ArrayAllocation::recursivelyCollectUses(ValueBase *Def) {
  for (auto *Opd : Def->getUses()) {
    auto *User = Opd->getUser();
    // Ignore reference counting and debug instructions.
    if (isa<RefCountingInst>(User) || isa<DestroyValueInst>(User) ||
        isa<DebugValueInst>(User))
      continue;

    if (auto mdi = MarkDependenceInstruction(User)) {
      if (Def == mdi.getBase()) {
        continue;
      }
    }

    // Array value projection.
    if (auto *SEI = dyn_cast<StructExtractInst>(User)) {
      if (!recursivelyCollectUses(SEI))
        return false;
      continue;
    }

    // Check array semantic calls.
    if (auto *apply = dyn_cast<ApplyInst>(User)) {
      ArraySemanticsCall ArrayOp(apply);
      switch (ArrayOp.getKind()) {
        case ArrayCallKind::kNone:
          return false;
        case ArrayCallKind::kGetCount:
          CountCalls.insert(ArrayOp);
          break;
        case ArrayCallKind::kArrayFinalizeIntrinsic:
          if (!recursivelyCollectUses(apply))
            return false;
          break;
        default:
          if (!ArrayOp.doesNotChangeArray())
            return false;
          break;
      }
      continue;
    }

    // An operation that escapes or modifies the array value.
    return false;
  }
  return true;
}

bool ArrayAllocation::propagateCountToUsers() {
  bool HasChanged = false;
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Propagating count from " << *Alloc);
  for (auto *Count : CountCalls) {
    assert(ArraySemanticsCall(Count).getKind() == ArrayCallKind::kGetCount &&
           "Expecting a call to count");

    SmallVector<Operand *, 16> Uses;
    for (auto *Op : Count->getUses()) {
      if (Op->get()->getType() == ArrayCount->getType()) {
        Uses.push_back(Op);
      }
    }

    for (auto *Use : Uses) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "  to user " << *Use->getUser());
      Use->set(ArrayCount);
      HasChanged = true;
    }

    if (HasChanged && onlyHaveDebugUses(Count))
      DeadArrayCountCalls.push_back(Count);
  }
  return HasChanged;
}

namespace {

class ArrayCountPropagation : public SILFunctionTransform {
public:
  ArrayCountPropagation() {}

  void run() override {
    auto &Fn = *getFunction();
    bool Changed = false;
    SmallVector<ApplyInst *, 16> DeadArrayCountCalls;
    // Propagate the count of array allocations to array.count users.
    for (auto &BB :Fn) {
      for (auto &Inst : BB) {
        if (auto *Apply = dyn_cast<ApplyInst>(&Inst))
          Changed |= ArrayAllocation::tryPropagate(Apply, DeadArrayCountCalls);
      }
    }
    // Remove dead array.count calls.
    for (auto *DeadCall : DeadArrayCountCalls) {
      ArraySemanticsCall GetCount(DeadCall);
      assert(GetCount.getKind() == ArrayCallKind::kGetCount);
      deleteAllDebugUses(GetCount);
      GetCount.removeCall();
    }

    if (Changed) {
      PM->invalidateAnalysis(&Fn,
                           SILAnalysis::InvalidationKind::CallsAndInstructions);
    }
  }
};

} // end anonymous namespace

SILTransform *language::createArrayCountPropagation() {
  return new ArrayCountPropagation();
}
