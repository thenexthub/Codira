//===--- PhiStorageOptimizer.h - Phi storage optimizer --------------------===//
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
/// This file defines PhiStorageOptimizer, a utility for use with the
/// mandatory AddressLowering pass.
///
//===----------------------------------------------------------------------===//

#include "AddressLowering.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILValue.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/SmallVector.h"

namespace language {

class DominanceInfo;

class CoalescedPhi {
  friend class PhiStorageOptimizer;

  SmallVector<Operand *, 4> coalescedOperands;

  CoalescedPhi(const CoalescedPhi &) = delete;
  CoalescedPhi &operator=(const CoalescedPhi &) = delete;

public:
  CoalescedPhi() = default;
  CoalescedPhi(CoalescedPhi &&) = default;
  CoalescedPhi &operator=(CoalescedPhi &&) = default;

  void coalesce(PhiValue phi, const ValueStorageMap &valueStorageMap,
                DominanceInfo *domInfo);

  bool empty() const { return coalescedOperands.empty(); }

  ArrayRef<Operand *> getCoalescedOperands() const { return coalescedOperands; }

  SILInstruction::OperandValueRange getCoalescedValues() const {
    return SILInstruction::getOperandValues(getCoalescedOperands());
  }
};

} // namespace language
