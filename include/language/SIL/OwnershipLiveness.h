//===--- OwnershipLiveness.h ---------------------------------*- C++ -*----===//
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
/// Terminology:
///
/// Linear lifetime: All paths through the value's definition to the
/// function exit pass through exactly one lifetime-ending operation.
///
/// Complete OSSA: All owned values and borrow introducers have linear lifetime.
///
/// Borrow introducer: defines a guaranteed SILValue and an associated explicit
/// borrow scope: begin_borrow, load_borrow, store_borrow, and reborrow.  A
/// "reborrow" is a guaranteed phi for which `isGuaranteedForwarding` returns
/// false. (Eventually, the phi will know whether it is a reborrow based on a
/// flag or subclass). Guaranteed function arguments do conceptually introduce
/// borrow scopes, but their scopes are implied by the function entry and
/// return points.
///
/// Completing an OSSA lifetime is the process of giving an owned value or a
/// borrow introducer a linear lifetime. This process may require phi creation
/// whenever a nested scope involves a guaranteed phi that is not dominated by
/// the outer scope's definition. A new phi will be created that ends the outer
/// lifetime and begins a new lifetime in the successor block. This new phi will
/// either forward an owned value or reborrow the outer borrow scope. The new
/// phi's lifetime will then be recursively extended over the lifetime of the
/// original guaranteed phi, which we refer to as its inner adjacent phi.
///
//===----------------------------------------------------------------------===//
///
/// Linear SSA liveness:
///
/// - Liveness for a single "defining" owned value or borrow introducing value
///   assuming the value's lifetime is already linear.
///
/// - The definition dominates all use points (phis do not extend the lifetime)
///
/// - Only lifetime-ending operations generate liveness
///
/// - Oblivious to pointer escapes within the lifetime
///
///
/// Interior SSA liveness:
///
/// - Liveness for a single "defining" value with any ownership.
///
/// - The definition dominates all use points (phis do not extend the lifetime)
///
/// - Does not assume the current lifetime is linear. Transitively follows
///   guaranteed forwarding and address uses within the current scope.
///
/// - Liveness cannot extend beyond lifetime-ending operations
///   (a.k.a. affine lifetimes).
///
/// - Assumes inner scopes *are* linear, including borrow and address scopes
///   (e.g. begin_borrow, load_borrow, begin_apply, store_borrow, begin_access)
///   A callback may be used to complete inner scopes before updating liveness.
///
/// - Only returns AddressUseKind::PointerEscape if one of the uses of the
///   outer value has OperandOwnership::PointerEscape or
///   OperandOwnership::BitwiseEscape. An inner scope's guaranteed value may
///   escape without causing the outer scope's value to escape.
///
/// - Insulates outer scopes from inner scope details. Maintains the
///   invariant that inlining cannot pessimize optimization.
///
/// - Interior SSA liveness is used to complete (linearize) an OSSA lifetime
///
/// Interior liveness example:
///
///     %struct = struct ...
///     %f = struct_extract %s     // defines a guaranteed value (%f)
///     %b = begin_borrow %field
///     %a = ref_element_addr %b
///     _  = address_to_pointer %a
///     end_borrow %b              // the only interior use of %f
///
/// When computing interior liveness for %f, %b is an inner scope. Because inner
/// scopes are complete, the only relevant use is end_borrow %b. Despite the
/// address_to_pointer instruction, %f does not escape any dependent address.
///
/// Transitive SSA liveness
///
/// - Similar to Interior SSA liveness, but does not assume that any lifetimes
///   are linear. Transitively follows uses within inner scopes, recursively
///   through nested scopes, including forwarding operations and address uses.
///
/// - Much more likely to return AddressUseKind::PointerEscape
///
/// Transitive liveness example:
///
///     %struct = struct ...
///     %f = struct_extract %s     // defines a guaranteed value (%f)
///     %b = begin_borrow %field
///     %a = ref_element_addr %b
///     _  = address_to_pointer %a // a transitive use of %f escapes
///
/// When computing transitive liveness for %f, %b is an inner scope. Liveness
/// does not assume that an end_borrow exists. Instead it transitively considers
/// all uses of %b. As a result, %f escapes.
///
/// - This header provides no interface for transitive liveness because it is no
///   longer needed with complete OSSA lifetimes
///
/// Extended liveness
///
/// - Liveness of a single "defining" value extended beyond its lifetime-ending
///   operations.
///
/// - May refer to copy-extension, phi-extension, or extension over barriers
///   such as access markers.
///
/// - For phi-extension, the definition no longer dominates the use
///   points. MultiDefPrunedLiveness must be used. Each phi is added as a new
///   definition.
///
/// Extended linear liveness
///
/// - Extended liveness that only considers lifetime-ending operations. Assumes
///   the definition already has a linear lifetime and that any phis that end
///   the current lifetime also have linear lifetimes.
///
/// Extended interior liveness
///
/// - Like interior SSA liveness, does not assume the current lifetime is
///   linear. Transitively follows guaranteed forwarding and address uses within
///   the current scope. Assumes inner scopes *are* linear.
///
/// - Interior copy-extension is used to canonicalize an OSSA lifetime
///
/// - This header provides no interface for extended interior liveness because
///   it is highly specific to a particular task.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_OWNERSHIPLIVENESS_H
#define LANGUAGE_SIL_OWNERSHIPLIVENESS_H

#include "language/Basic/Debug.h"
#include "language/Basic/Toolchain.h"
#include "language/SIL/PrunedLiveness.h"
#include "language/SIL/OwnershipUseVisitor.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILValue.h"
#include "toolchain/ADT/SmallVector.h"

namespace language {

/// Analyze SSA liveness of values that introduce an OSSA live range:
///
/// 1. Owned non-phi values
/// 2. Owned phi values
/// 3. Borrow scope introducers: begin_borrow/load_borrow
/// 4. Reborrows: guaranteed phis that end their operands' borrow scopes and
///    require their own post-dominating end_borrows.
///
/// Used for OSSA lifetime completion.
class OSSALiveness {
protected:
  SILValue ownershipDef;

  // MARK: Results.

  SmallVector<SILBasicBlock *, 8> discoveredBlocks;
  SSAPrunedLiveness liveness;

  OSSALiveness(const OSSALiveness &) = delete;
  OSSALiveness &operator=(const OSSALiveness &) = delete;

public:
  OSSALiveness(SILValue def): ownershipDef(def),
                              liveness(def->getFunction(), &discoveredBlocks) {}

  const SSAPrunedLiveness &getLiveness() const { return liveness; }

  ArrayRef<SILBasicBlock *> getDiscoveredBlocks() const {
    return discoveredBlocks;
  }

  void print(toolchain::raw_ostream &OS) const;
  void dump() const;
};

// Internal implementation
struct LinearLivenessVisitor;

/// Compute ownershipDef's lifetime based on it's lifetime-ending uses, assuming
/// it is already complete/linear. ownershipDef must be either an owned value or
/// a local borrow scope introduced (begin_borrow, load_borrow, or
/// store_borrow).
///
/// This is the simplest OSSA liveness analysis, but is not appropriate for
/// fixing OSSA lifetimes after a transformation and cannot tell you whether
/// pointer escapes occur.
class LinearLiveness : public OSSALiveness {
  friend LinearLivenessVisitor;

public:
  /// Whether extend_lifetime instructions should be added to the boundary.
  /// Used to verify extend_lifetime instructions.
  enum IncludeExtensions_t {
    DoNotIncludeExtensions = false,
    IncludeExtensions = true,
  };
  LinearLiveness(SILValue def,
                 IncludeExtensions_t includeExtensions = IncludeExtensions);

  void compute();

private:
  const IncludeExtensions_t includeExtensions;
};

// Internal implementation
struct InteriorLivenessVisitor;

/// Compute ownershipDef's lifetime based on all its uses (except those
/// already enclosed by inner scopes). Returns AddressUseKind::PointerEscape
/// if a pointer to ownershipDef escapes (and is not already enclosed by an
/// inner scope).
class InteriorLiveness : public OSSALiveness {
  friend InteriorLivenessVisitor;

  // Handle inner scopes. Called for inner reborrows, inner adjacent reborrows,
  // and address scopes.
  //
  // This may add uses to the inner scope, but it may not modify a use-list
  // in any outer scopes.
  using InnerScopeHandlerRef = toolchain::function_ref<void(SILValue)>;

public:
  // Summarize address uses
  AddressUseKind addressUseKind = AddressUseKind::Unknown;
  Operand *escapingUse = nullptr;

public:
  InteriorLiveness(SILValue def): OSSALiveness(def) {}

  void compute(const DominanceInfo *domInfo,
               InnerScopeHandlerRef handleInnerScope = InnerScopeHandlerRef());

  /// Compute the boundary from the blocks discovered during liveness analysis.
  void computeBoundary(PrunedLivenessBoundary &boundary) const {
    liveness.computeBoundary(boundary);
  }

  AddressUseKind getAddressUseKind() const { return addressUseKind; }

  void print(toolchain::raw_ostream &OS) const;
  void dump() const;
};

// Internal implementation
struct ExtendedLinearLivenessVisitor;

/// Analyze liveness of values that introduce an OSSA live range. This computes
/// the phi-extended live range for these four categories of live range
/// introducing values:
///
/// 1. Owned non-phi values
/// 2. Owned phi values
/// 3. Borrow scope introducers: begin_borrow/load_borrow
/// 4. Reborrows: guaranteed phis that end their incoming borrow scopes and
///    begin a new borrow scope
class ExtendedLinearLiveness {
  friend ExtendedLinearLivenessVisitor;

  SILValue ownershipDef;

  // MARK: Results.

  // Because of reborrows, the ssa def may not dominate all
  // uses. Consider the reborrows to be separate defs.
  SmallVector<SILBasicBlock *, 8> discoveredBlocks;
  MultiDefPrunedLiveness liveness;

  ExtendedLinearLiveness(const ExtendedLinearLiveness &) = delete;
  ExtendedLinearLiveness &operator=(const ExtendedLinearLiveness &) = delete;

public:
  ExtendedLinearLiveness(SILValue def);

  void compute();

  /// The array of defs. The first element is ownershipDef. The remaining
  /// elements are outer reborrows discovered during computation.
  ///
  /// TODO: These are always SILValues. Convert the iterator.
  NodeSetVector::iterator defBegin() const { return liveness.defBegin(); }
  NodeSetVector::iterator defEnd() const { return liveness.defBegin(); }

  const MultiDefPrunedLiveness &getLiveness() const { return liveness; }

  void print(toolchain::raw_ostream &OS) const;
  void dump() const;
};

} // namespace language

#endif
