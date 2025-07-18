//===--- OSSALifetimeCompletion.cpp ---------------------------------------===//
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
///
/// OSSA lifetime completion adds lifetime ending instructions to make
/// linear lifetimes complete.
///
/// Interior liveness handles the following cases naturally:
///
/// When completing the lifetime of the initial value, %v1, transitively
/// include all uses of dominated reborrows as, such as %phi1 in this example:
///
///     %v1 = ...
///     cond_br bb1, bb2
///   bb1:
///     %b1 = begin_borrow %v1
///     br bb3(%b1)
///   bb2:
///     %b2 = begin_borrow %v1
///     br bb3(%b2)
///   bb3(%phi1):
///     %u1 = %phi1
///     end_borrow %phi1
///     %k1 = destroy_value %v1 // must be below end_borrow %phi1
///
/// When completing the lifetime for a phi (%phi2) transitively include all
/// uses of inner adjacent reborrows, such as %phi1 in this example:
///
///   bb1:
///     %v1 = ...
///     %b1 = begin_borrow %v1
///     br bb3(%b1, %v1)
///   bb2:
///     %v2 = ...
///     %b2 = begin_borrow %v2
///     br bb3(%b2, %v2)
///   bb3(%phi1, %phi2):
///     %u1 = %phi1
///     end_borrow %phi1
///     %k1 = destroy_value %phi1
///
//===----------------------------------------------------------------------===//

#include "language/SIL/OSSALifetimeCompletion.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/AddressWalker.h"
#include "language/SIL/BasicBlockUtils.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/Test.h"
#include "toolchain/ADT/STLExtras.h"

using namespace language;

// FIXME: remove this option after fixing:
// rdar://145994924 (Mem2Reg calls lifetime completion without checking for
// pointer escapes)
toolchain::cl::opt<bool> VerifyLifetimeCompletion(
    "verify-lifetime-completion", toolchain::cl::init(false),
    toolchain::cl::desc("."));

static SILInstruction *endOSSALifetime(SILValue value,
                                       OSSALifetimeCompletion::LifetimeEnd end,
                                       SILBuilder &builder,
                                       DeadEndBlocks &deb) {
  auto loc =
    RegularLocation::getAutoGeneratedLocation(builder.getInsertionPointLoc());
  if (end == OSSALifetimeCompletion::LifetimeEnd::Loop) {
    return builder.createExtendLifetime(loc, value);
  }
  auto isDeadEnd = IsDeadEnd_t(deb.isDeadEnd(builder.getInsertionBB()));
  if (value->getOwnershipKind() == OwnershipKind::Owned) {
    if (value->getType().is<SILBoxType>()) {
      return builder.createDeallocBox(loc, value, isDeadEnd);
    }
    return builder.createDestroyValue(loc, value, DontPoisonRefs, isDeadEnd);
  }
  if (auto scopedAddress = ScopedAddressValue(value)) {
    return scopedAddress.createScopeEnd(builder.getInsertionPoint(), loc);
  }
  if (value->getOwnershipKind() == OwnershipKind::None) {
    return builder.createExtendLifetime(loc, value);
  }
  return builder.createEndBorrow(loc, lookThroughBorrowedFromUser(value));
}

static bool endLifetimeAtLivenessBoundary(SILValue value,
                                          const SSAPrunedLiveness &liveness,
                                          DeadEndBlocks &deb) {
  PrunedLivenessBoundary boundary;
  liveness.computeBoundary(boundary);

  bool changed = false;
  for (SILInstruction *lastUser : boundary.lastUsers) {
    if (liveness.isInterestingUser(lastUser)
        != PrunedLiveness::LifetimeEndingUse) {
      changed = true;
      SILBuilderWithScope::insertAfter(lastUser, [value,
                                                  &deb](SILBuilder &builder) {
        endOSSALifetime(value, OSSALifetimeCompletion::LifetimeEnd::Boundary,
                        builder, deb);
      });
    }
  }
  for (SILBasicBlock *edge : boundary.boundaryEdges) {
    changed = true;
    SILBuilderWithScope builder(edge->begin());
    endOSSALifetime(value, OSSALifetimeCompletion::LifetimeEnd::Boundary,
                    builder, deb);
  }
  for (SILNode *deadDef : boundary.deadDefs) {
    SILInstruction *next = nullptr;
    if (auto *deadInst = dyn_cast<SILInstruction>(deadDef)) {
      next = deadInst->getNextInstruction();
    } else {
      next = cast<ValueBase>(deadDef)->getNextInstruction();
    }
    changed = true;
    SILBuilderWithScope builder(next);
    endOSSALifetime(value, OSSALifetimeCompletion::LifetimeEnd::Boundary,
                    builder, deb);
  }
  return changed;
}

static void visitUsersOutsideLinearLivenessBoundary(
    SILValue value, const SSAPrunedLiveness &liveness,
    toolchain::function_ref<void(SILInstruction *)> visitor) {
  if (value->getOwnershipKind() == OwnershipKind::None) {
    return;
  }
  LinearLiveness linearLiveness(value);
  linearLiveness.compute();
  for (auto pair : liveness.getAllUsers()) {
    if (pair.second.isEnding() || isa<ExtendLifetimeInst>(pair.first)) {
      continue;
    }
    auto *user = pair.first;
    if (linearLiveness.getLiveness().isWithinBoundary(
            user, /*deadEndBlocks=*/nullptr)) {
      continue;
    }
    visitor(user);
  }
}

namespace language::test {
// Arguments:
// - SILValue: value
// Dumps:
// - the instructions outside the liveness boundary
static FunctionTest LivenessPartialBoundaryOutsideUsersTest(
    "liveness_partial_boundary_outside_users",
    [](auto &function, auto &arguments, auto &test) {
      SILValue value = arguments.takeValue();
      InteriorLiveness liveness(value);
      liveness.compute(test.getDominanceInfo());
      visitUsersOutsideLinearLivenessBoundary(
          value, liveness.getLiveness(),
          [](auto *inst) { inst->print(toolchain::outs()); });
    });
} // end namespace language::test

namespace {
/// Visits the latest instructions at which `value` is available.
///
/// Together with visitUsersOutsideLinearLivenessBoundary, implements
/// OSSALifetimeCompletion::visitAvailabilityBoundary.
///
/// Finding these positions is a three step process:
/// 1) computeRegion: Forward CFG walk from non-lifetime-ending boundary to find
///                   the dead-end region in which the value might be available.
/// 2) propagateAvailability: Forward iterative dataflow within the region to
///                           determine which blocks the value is available in.
/// 3) visitAvailabilityBoundary: Visits the final blocks in the region where
///                               the value is available--these are the blocks
///                               without successors or with at least one
///                               unavailable successor.
class AvailabilityBoundaryVisitor {
  /// The value whose dead-end block lifetime ends are to be visited.
  SILValue value;

  /// The non-lifetime-ending boundary of `value`.
  BasicBlockSet starts;
  /// The region between (inclusive) the `starts` and the unreachable blocks.
  BasicBlockSetVector region;

public:
  AvailabilityBoundaryVisitor(SILValue value)
      : value(value), starts(value->getFunction()),
        region(value->getFunction()) {}

  using Visit = toolchain::function_ref<void(SILInstruction *,
                                        OSSALifetimeCompletion::LifetimeEnd)>;

  struct Result;

  /// Do all three steps at once.
  void visit(const SSAPrunedLiveness &liveness, Result &result, Visit visit);

private:
  /// Region discovery.
  ///
  /// Forward CFG walk from non-lifetime-ending boundary to unreachable
  /// instructions.
  void computeRegion(const SSAPrunedLiveness &liveness);

  /// Iterative dataflow to determine availability for each block in `region`.
  void propagateAvailablity(Result &result);

  /// Visit the terminators of blocks on the boundary of availability.
  void visitAvailabilityBoundary(Result const &result, Visit visit);

  struct State {
    enum Value : uint8_t {
      Unavailable = 0,
      Available,
      Unknown,
    };
    Value value;

    State(Value value) : value(value){};
    operator Value() const { return value; }
    State meet(State const other) const {
      return *this < other ? *this : other;
    }
  };

public:
  struct Result {
    BasicBlockBitfield states;

    Result(SILFunction *function) : states(function, 2) {}

    State getState(SILBasicBlock *block) const {
      return {(State::Value)states.get(block)};
    }

    void setState(SILBasicBlock *block, State newState) {
      states.set(block, (unsigned)newState.value);
    }

    /// Propagate predecessors' state into `block`.
    ///
    /// states[block] ∧= state[predecessor_1] ∧ ... ∧ state[predecessor_n]
    bool updateState(SILBasicBlock *block) {
      auto oldState = getState(block);
      auto state = oldState;
      for (auto *predecessor : block->getPredecessorBlocks()) {
        state = state.meet(getState(predecessor));
      }
      setState(block, state);
      return state != oldState;
    }
  };
};

void AvailabilityBoundaryVisitor::visit(const SSAPrunedLiveness &liveness,
                                        Result &result, Visit visit) {
  computeRegion(liveness);

  propagateAvailablity(result);

  visitAvailabilityBoundary(result, visit);
}

void AvailabilityBoundaryVisitor::computeRegion(
    const SSAPrunedLiveness &liveness) {
  // (1) Compute the complete liveness boundary.
  PrunedLivenessBlockBoundary boundary;
  liveness.computeBoundary(boundary);

  BasicBlockSet consumingBlocks(value->getFunction());

  liveness.visitUsers(
      [&consumingBlocks](auto *instruction, auto lifetimeEnding) {
        if (lifetimeEnding.isEnding()) {
          consumingBlocks.insert(instruction->getParent());
        }
      });

  // Used in the forward walk below (3).
  BasicBlockWorklist regionWorklist(value->getFunction());

  // (2) Collect the non-lifetime-ending liveness boundary.  This is the
  //     portion of `boundary` consisting of:
  // - non-lifetime-ending instructions (their parent blocks)
  // - boundary edges
  // - dead defs (their parent blocks)
  auto collect = [&](SILBasicBlock *block) {
    // `region` consists of the non-lifetime-ending boundary and all its
    // iterative successors.
    region.insert(block);
    // `starts` just consists of the blocks in the non-lifetime-ending
    // boundary.
    starts.insert(block);
    // The forward walk begins from the non-lifetime-ending boundary.
    regionWorklist.push(block);
  };

  // Trivial values that correspond to local variables (as opposed to
  // ScopedAddresses) are available only up to their last extend_lifetime on
  // non-dead-end paths. They cannot be consumed, but are only "available" up to
  // the end of their scope.
  if (value->getOwnershipKind() != OwnershipKind::None
      || ScopedAddressValue(value)) {
    for (auto *endBlock : boundary.endBlocks) {
      if (!consumingBlocks.contains(endBlock)) {
        collect(endBlock);
      }
    }
  }
  for (SILBasicBlock *edge : boundary.boundaryEdges) {
    collect(edge);
  }

  // (3) Forward walk to find the region in which `value` might be available.
  while (auto *block = regionWorklist.pop()) {
    if (block->succ_empty()) {
      // This is a function-exiting block.
      //
      // In valid-but-lifetime-incomplete OSSA there must be a lifetime-ending
      // instruction on each path from the def that exits the function normally.
      // Thus finding a value available at the end of such a block means that
      // the block does _not_ must not exits the function normally; in other
      // words its terminator must be an UnreachableInst.
      assert(isa<UnreachableInst>(block->getTerminator()));
    }
    for (auto *successor : block->getSuccessorBlocks()) {
      regionWorklist.pushIfNotVisited(successor);
      region.insert(successor);
    }
  }
}

void AvailabilityBoundaryVisitor::propagateAvailablity(Result &result) {
  // Initialize per-block state.
  // - all blocks outside of the region are ::Unavailable (automatically
  //   initialized)
  // - non-initial in-region blocks are Unknown
  // - start blocks are ::Available
  for (auto *block : region) {
    if (starts.contains(block))
      result.setState(block, State::Available);
    else
      result.setState(block, State::Unknown);
  }

  BasicBlockWorklist worklist(value->getFunction());

  // Initialize worklist with all participating blocks.
  //
  // Only perform dataflow in the non-initial region.  Every initial block is
  // by definition ::Available.
  for (auto *block : region) {
    if (starts.contains(block))
      continue;
    worklist.push(block);
  }

  // Iterate over blocks which are successors of blocks whose state changed.
  while (auto *block = worklist.popAndForget()) {
    // Only propagate availability in non-initial, in-region blocks.
    if (!region.contains(block) || starts.contains(block))
      continue;
    auto changed = result.updateState(block);
    if (!changed) {
      continue;
    }
    // The state has changed.  Propagate the new state into successors.
    for (auto *successor : block->getSuccessorBlocks()) {
      worklist.pushIfNotVisited(successor);
    }
  }
}

void AvailabilityBoundaryVisitor::visitAvailabilityBoundary(
    Result const &result,
    toolchain::function_ref<void(SILInstruction *,
                            OSSALifetimeCompletion::LifetimeEnd end)>
        visit) {
  for (auto *block : region) {
    auto available = result.getState(block) == State::Available;
    if (!available) {
      continue;
    }
    auto hasUnavailableSuccessor = [&]() {
      // Use a lambda to avoid checking if possible.
      return toolchain::any_of(block->getSuccessorBlocks(), [&result](auto *block) {
        return result.getState(block) == State::Unavailable;
      });
    };
    if (!block->succ_empty() && !hasUnavailableSuccessor()) {
      continue;
    }
    assert(hasUnavailableSuccessor() ||
           isa<UnreachableInst>(block->getTerminator()));
    visit(block->getTerminator(),
          OSSALifetimeCompletion::LifetimeEnd::Boundary);
  }
}
} // end anonymous namespace

void OSSALifetimeCompletion::visitAvailabilityBoundary(
    SILValue value, const SSAPrunedLiveness &liveness,
    toolchain::function_ref<void(SILInstruction *, LifetimeEnd end)> visit) {

  AvailabilityBoundaryVisitor visitor(value);
  AvailabilityBoundaryVisitor::Result result(value->getFunction());
  visitor.visit(liveness, result, visit);

  visitUsersOutsideLinearLivenessBoundary(
      value, liveness, [&](auto *instruction) {
        instruction->visitSubsequentInstructions([&](auto *next) {
          visit(next, LifetimeEnd::Loop);
          return true;
        });
      });
}

static bool endLifetimeAtAvailabilityBoundary(SILValue value,
                                              const SSAPrunedLiveness &liveness,
                                              DeadEndBlocks &deb) {
  bool changed = false;
  OSSALifetimeCompletion::visitAvailabilityBoundary(
      value, liveness, [&](auto *unreachable, auto end) {
        SILBuilderWithScope builder(unreachable);
        endOSSALifetime(value, end, builder, deb);
        changed = true;
      });
  return changed;
}

static bool endLifetimeAtBoundary(SILValue value,
                                  SSAPrunedLiveness const &liveness,
                                  OSSALifetimeCompletion::Boundary boundary,
                                  DeadEndBlocks &deadEndBlocks) {
  bool changed = false;
  switch (boundary) {
  case OSSALifetimeCompletion::Boundary::Liveness:
    changed |= endLifetimeAtLivenessBoundary(value, liveness, deadEndBlocks);
    break;
  case OSSALifetimeCompletion::Boundary::Availability:
    changed |=
        endLifetimeAtAvailabilityBoundary(value, liveness, deadEndBlocks);
    break;
  }
  return changed;
}

bool OSSALifetimeCompletion::analyzeAndUpdateLifetime(
    ScopedAddressValue scopedAddress, Boundary boundary) {
  SmallVector<SILBasicBlock *, 8> discoveredBlocks;
  SSAPrunedLiveness liveness(scopedAddress->getFunction(), &discoveredBlocks);
  liveness.initializeDef(scopedAddress.value);

  struct Walker : TransitiveAddressWalker<Walker> {
    OSSALifetimeCompletion &completion;
    ScopedAddressValue scopedAddress;
    Boundary boundary;
    SSAPrunedLiveness &liveness;
    Walker(OSSALifetimeCompletion &completion, ScopedAddressValue scopedAddress,
           Boundary boundary, SSAPrunedLiveness &liveness)
        : completion(completion), scopedAddress(scopedAddress),
          boundary(boundary), liveness(liveness) {}
    bool visitUse(Operand *use) {
      auto *user = use->getUser();
      if (scopedAddress.isScopeEndingUse(use)) {
        liveness.updateForUse(user, /*lifetimeEnding=*/true);
        return true;
      }
      liveness.updateForUse(user, /*lifetimeEnding=*/false);
      for (auto result : user->getResults()) {
        auto shouldComplete =
            (bool)BorrowedValue(result) || (bool)ScopedAddressValue(result);
        if (!shouldComplete)
          continue;
        auto completed = completion.completeOSSALifetime(result, boundary);
        switch (completed) {
        case LifetimeCompletion::NoLifetime:
          break;
        case LifetimeCompletion::AlreadyComplete:
        case LifetimeCompletion::WasCompleted:
          for (auto *consume : result->getConsumingUses()) {
            liveness.updateForUse(consume->getUser(), /*lifetimeEnding=*/false);
          }
          break;
        }
      }
      return true;
    }
  };
  Walker walker(*this, scopedAddress, boundary, liveness);
  AddressUseKind result = walker.walk(scopedAddress.value);
  if ((VerifyLifetimeCompletion || ForceLivenessVerification) &&
      boundary != Boundary::Availability &&
      result != AddressUseKind::NonEscaping) {
    toolchain::errs() << "Incomplete liveness for:\n" << scopedAddress.value;
    if (auto *escapingUse = walker.getEscapingUse()) {
      toolchain::errs() << "  escapes at:\n";
      escapingUse->getUser()->printInContext(toolchain::errs());
    }
    ASSERT(false && "caller must check for pointer escapes");
  }
  return endLifetimeAtBoundary(scopedAddress.value, liveness, boundary,
                               deadEndBlocks);
}

/// End the lifetime of \p value at unreachable instructions.
///
/// Returns true if any new instructions were created to complete the lifetime.
bool OSSALifetimeCompletion::analyzeAndUpdateLifetime(SILValue value,
                                                      Boundary boundary) {
  if (auto scopedAddress = ScopedAddressValue(value)) {
    return analyzeAndUpdateLifetime(scopedAddress, boundary);
  }
  // Called for inner borrows, inner adjacent reborrows, inner reborrows, and
  // scoped addresses.
  auto handleInnerScope = [this, boundary](SILValue innerBorrowedValue) {
    completeOSSALifetime(innerBorrowedValue, boundary);
  };
  if (value->getOwnershipKind() == OwnershipKind::None) {
    // Trivial variable lifetimes are only relevant up to the extend_lifetime
    // instructions emitted by SILGen. Their other uses have no meaning with
    // respect to lifetime. The only purpose of "completing" their lifetime is
    // to insert extend_lifetime on dead-end blocks.
    LinearLiveness liveness(value);
    liveness.compute();
    return endLifetimeAtBoundary(value, liveness.getLiveness(), boundary,
                                 deadEndBlocks);
  }
  InteriorLiveness liveness(value);
  liveness.compute(domInfo, handleInnerScope);
  if (VerifyLifetimeCompletion && boundary != Boundary::Availability
      && liveness.getAddressUseKind() != AddressUseKind::NonEscaping) {
    toolchain::errs() << "Incomplete liveness for: " << value;
    if (auto *escapingUse = liveness.escapingUse) {
      toolchain::errs() << "  escapes at:\n";
      escapingUse->getUser()->printInContext(toolchain::errs());
    }
    ASSERT(false && "caller must check for pointer escapes");
  }
  return endLifetimeAtBoundary(value, liveness.getLiveness(), boundary,
                               deadEndBlocks);
}

namespace language::test {
// Arguments:
// - SILValue: value
// - string: either "liveness" or "availability"
// Dumps:
// - function
static FunctionTest OSSALifetimeCompletionTest(
    "ossa_lifetime_completion",
    [](auto &function, auto &arguments, auto &test) {
      SILValue value = arguments.takeValue();
      OSSALifetimeCompletion::Boundary kind =
          toolchain::StringSwitch<OSSALifetimeCompletion::Boundary>(
              arguments.takeString())
              .Case("liveness", OSSALifetimeCompletion::Boundary::Liveness)
              .Case("availability",
                    OSSALifetimeCompletion::Boundary::Availability);
      auto *deb = test.getDeadEndBlocks();
      toolchain::outs() << "OSSA lifetime completion on " << kind
                   << " boundary: " << value;
      OSSALifetimeCompletion completion(&function, /*domInfo*/ nullptr, *deb);
      completion.completeOSSALifetime(value, kind);
      function.print(toolchain::outs());
    });
} // end namespace language::test

// TODO: create a fast check for 'mayEndLifetime(SILInstruction *)'. Verify that
// it returns true for every instruction that has a lifetime-ending operand.
void UnreachableLifetimeCompletion::visitUnreachableInst(
    SILInstruction *instruction) {
  auto *block = instruction->getParent();
  bool inReachableBlock = !unreachableBlocks.contains(block);
  // If this instruction's block is already marked unreachable, and
  // updatingLifetimes is not yet set, then this instruction will be visited
  // again later when propagating unreachable blocks.
  if (!inReachableBlock && !updatingLifetimes)
    return;

  for (Operand &operand : instruction->getAllOperands()) {
    if (!operand.isLifetimeEnding())
      continue;

    SILValue value = operand.get();
    SILBasicBlock *defBlock = value->getParentBlock();
    if (unreachableBlocks.contains(defBlock))
      continue;

    auto *def = value->getDefiningInstruction();
    if (def && unreachableInsts.contains(def))
      continue;

    // The operand's definition is still reachable and its lifetime ends on a
    // newly unreachable path.
    //
    // Note: The arguments of a no-return try_apply may still appear reachable
    // here because the try_apply itself is never visited as unreachable, hence
    // its successor blocks are not marked . But it
    // seems harmless to recompute their lifetimes.

    // Insert this unreachable instruction in unreachableInsts if its parent
    // block is not already marked unreachable.
    if (inReachableBlock) {
      unreachableInsts.insert(instruction);
    }
    incompleteValues.insert(value);

    // Add unreachable successors to the forward traversal worklist.
    if (auto *term = dyn_cast<TermInst>(instruction)) {
      for (auto *succBlock : term->getSuccessorBlocks()) {
        if (toolchain::all_of(succBlock->getPredecessorBlocks(),
                         [&](SILBasicBlock *predBlock) {
                           if (predBlock == block)
                             return true;

                           return unreachableBlocks.contains(predBlock);
                         })) {
          unreachableBlocks.insert(succBlock);
        }
      }
    }
  }
}

bool UnreachableLifetimeCompletion::completeLifetimes() {
  assert(!updatingLifetimes && "don't call this more than once");
  updatingLifetimes = true;

  // Now that all unreachable terminator instructions have been visited,
  // propagate unreachable blocks.
  for (auto blockIt = unreachableBlocks.begin();
       blockIt != unreachableBlocks.end(); ++blockIt) {
    auto *block = *blockIt;
    for (auto &instruction : *block) {
      visitUnreachableInst(&instruction);
    }
  }

  OSSALifetimeCompletion completion(function, domInfo, deadEndBlocks);

  bool changed = false;
  for (auto value : incompleteValues) {
    if (completion.completeOSSALifetime(
            value, OSSALifetimeCompletion::Boundary::Availability) ==
        LifetimeCompletion::WasCompleted) {
      changed = true;
    }
  }
  return changed;
}
