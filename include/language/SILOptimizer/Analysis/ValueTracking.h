//===--- ValueTracking.h - SIL Value Tracking Analysis ----------*- C++ -*-===//
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
//
// This file contains routines which analyze chains of computations.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_ANALYSIS_VALUETRACKING_H
#define LANGUAGE_SILOPTIMIZER_ANALYSIS_VALUETRACKING_H

#include "language/SIL/MemAccessUtils.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILInstruction.h"

namespace language {

/// Returns true if \p V is a function argument which may not alias to
/// any other pointer in the function.
///
/// This does not look through any projections. The caller must do that.
bool isExclusiveArgument(SILValue V);

/// Returns true if \p V is a locally allocated object.
///
/// Note: this may look through a single level of indirection (via
/// ref_element_addr) when \p V is the address of a class property. However, it
/// does not look through init/open_existential_addr.
bool pointsToLocalObject(SILValue V);

/// Returns true if \p V is a uniquely identified address or reference. Two
/// uniquely identified pointers with distinct roots cannot alias. However, a
/// uniquely identified pointer may alias with unidentified pointers. For
/// example, the uniquely identified pointer may escape to a call that returns
/// an alias of that pointer.
///
/// It may be any of:
///
/// - an address projection based on a locally allocated address with no
/// indirection
///
/// - a locally allocated reference, or an address projection based on that
/// reference with one level of indirection (an address into the locally
/// allocated object).
///
/// - an address projection based on an exclusive argument with no levels of
/// indirection (e.g. ref_element_addr, project_box, etc.).
///
/// TODO: Fold this into the AccessStorage API. pointsToLocalObject should be
/// performed by AccessStorage::isUniquelyIdentified.
inline bool isUniquelyIdentified(SILValue V) {
  SILValue objectRef = V;
  if (V->getType().isAddress()) {
    auto storage = AccessStorage::compute(V);
    if (!storage)
      return false;

    if (storage.isUniquelyIdentified())
      return true;

    if (!storage.isObjectAccess())
      return false;

    objectRef = storage.getObject();
  }
  return pointsToLocalObject(objectRef);
}

enum class IsZeroKind {
  Zero,
  NotZero,
  Unknown
};

/// Check if the value \p Value is known to be zero, non-zero or unknown.
IsZeroKind isZeroValue(SILValue Value);

/// Checks if a sign bit of a value is known to be set, not set or unknown.
/// Essentially, it is a simple form of a range analysis.
/// This approach is inspired by the corresponding implementation of
/// ComputeSignBit in LLVM's value tracking implementation.
/// It is planned to extend this approach to track all bits of a value.
/// Therefore it can be considered to be the beginning of a range analysis
/// infrastructure for the Codira compiler.
std::optional<bool> computeSignBit(SILValue Value);

/// Check if execution of a given builtin instruction can result in overflows.
/// Returns true of an overflow can happen. Otherwise returns false.
bool canOverflow(BuiltinInst *BI);

} // end namespace language

#endif // LANGUAGE_SILOPTIMIZER_ANALYSIS_VALUETRACKING_H
