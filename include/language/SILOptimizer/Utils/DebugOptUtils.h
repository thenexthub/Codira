//===--- DebugOptUtils.h --------------------------------------------------===//
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
/// Debug Info related utilities that rely on code in SILOptimizer/ and thus can
/// not be in include/language/SIL/DebugUtils.h.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_DEBUGOPTUTILS_H
#define LANGUAGE_SILOPTIMIZER_DEBUGOPTUTILS_H

#include "language/SIL/DebugUtils.h"
#include "language/SIL/Projection.h"
#include "language/SIL/SILValue.h"
#include "language/SILOptimizer/Utils/InstOptUtils.h"

namespace language {

/// Deletes all of the debug instructions that use \p value.
inline void deleteAllDebugUses(SILValue value, InstModCallbacks &callbacks) {
  for (auto ui = value->use_begin(), ue = value->use_end(); ui != ue;) {
    auto *inst = ui->getUser();
    ++ui;
    if (inst->isDebugInstruction()) {
      callbacks.deleteInst(inst);
    }
  }
}

/// Deletes all of the debug uses of any result of \p inst.
inline void deleteAllDebugUses(SILInstruction *inst,
                               InstModCallbacks &callbacks) {
  for (SILValue v : inst->getResults()) {
    deleteAllDebugUses(v, callbacks);
  }
}

/// Transfer debug info associated with (the result of) \p I to a
/// new `debug_value` instruction before \p I is deleted.
void salvageDebugInfo(SILInstruction *I);

/// Transfer debug info associated with the store-like instruction \p SI to a
/// new `debug_value` instruction before \p SI is deleted.
/// \param SI The store instruction being deleted
/// \param SrcVal The old source, where the debuginfo should be propagated to
/// \param DestVal The old destination, where the debuginfo was
void salvageStoreDebugInfo(SILInstruction *SI,
                           SILValue SrcVal, SILValue DestVal);

/// Transfer debug information associated with the result of \p load to the
/// load's address operand.
///
/// TODO: combine this with salvageDebugInfo when it is supported by
/// optimizations.
void salvageLoadDebugInfo(LoadOperation load);

/// Create debug_value fragment for a new partial value.
///
/// Precondition: \p oldValue is a struct or class aggregate. \p proj projects a
/// field from the aggregate into \p newValue corresponding to struct_extract.
void createDebugFragments(SILValue oldValue, Projection proj,
                          SILValue newValue);

/// Erases the instruction \p I from it's parent block and deletes it, including
/// all debug instructions which use \p I.
/// Precondition: The instruction may only have debug instructions as uses.
/// If the iterator \p InstIter references any deleted instruction, it is
/// incremented.
///
/// \p callbacks InstModCallback to use.
///
/// Returns an iterator to the next non-deleted instruction after \p I.
inline SILBasicBlock::iterator
eraseFromParentWithDebugInsts(SILInstruction *inst,
                              InstModCallbacks callbacks = InstModCallbacks()) {
  auto nextII = std::next(inst->getIterator());

  auto results = inst->getResults();
  bool foundAny;
  do {
    foundAny = false;
    for (auto result : results) {
      while (!result->use_empty()) {
        foundAny = true;
        auto *user = result->use_begin()->getUser();
        assert(user->isDebugInstruction());
        if (nextII == user->getIterator())
          ++nextII;
        callbacks.deleteInst(user);
      }
    }
  } while (foundAny);

  // Just matching what eraseFromParentWithDebugInsts is today.
  if (nextII == inst->getIterator())
    ++nextII;
  language::salvageDebugInfo(inst);
  callbacks.deleteInst(inst, false /*do not notify*/);
  return nextII;
}

} // namespace language

#endif
