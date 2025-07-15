//===--- SimplifyInstruction.h - Fold instructions --------------*- C++ -*-===//
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
// An analysis that provides utilities for folding instructions. Since it is an
// analysis it does not modify the IR in anyway. This is left to actual
// SIL Transforms.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_ANALYSIS_SIMPLIFYINSTRUCTION_H
#define LANGUAGE_SILOPTIMIZER_ANALYSIS_SIMPLIFYINSTRUCTION_H

#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILInstruction.h"

namespace language {

class SILInstruction;
struct InstModCallbacks;

/// Replace an instruction with a simplified result and erase it. If the
/// instruction initiates a scope, do not replace the end of its scope; it will
/// be deleted along with its parent.
///
/// NOTE: When OSSA is enabled this API assumes OSSA is properly formed and will
/// insert compensating instructions.
SILBasicBlock::iterator
replaceAllSimplifiedUsesAndErase(SILInstruction *I, SILValue result,
                                 InstModCallbacks &callbacks,
                                 DeadEndBlocks *deadEndBlocks = nullptr);

/// Attempt to map \p inst to a simplified result. Upon success, replace \p inst
/// with this simplified result and erase \p inst. If the instruction initiates
/// a scope, do not replace the end of its scope; it will be deleted along with
/// its parent.
///
/// NOTE: When OSSA is enabled this API assumes OSSA is properly formed and will
/// insert compensating instructions.
/// NOTE: When \p I is in an OSSA function, this fails to optimize if \p
/// deadEndBlocks is null.
SILBasicBlock::iterator simplifyAndReplaceAllSimplifiedUsesAndErase(
    SILInstruction *I, InstModCallbacks &callbacks,
    DeadEndBlocks *deadEndBlocks = nullptr);

// Simplify invocations of builtin operations that may overflow.
/// All such operations return a tuple (result, overflow_flag).
/// This function try to simplify such operations, but returns only a
/// simplified first element of a tuple. The overflow flag is not returned
/// explicitly, because this simplification is only possible if there is
/// no overflow. Therefore the overflow flag is known to have a value of 0 if
/// simplification was successful.
/// In case when a simplification is not possible, a null SILValue is returned.
SILValue simplifyOverflowBuiltinInstruction(BuiltinInst *BI);

} // end namespace language

#endif // LANGUAGE_SILOPTIMIZER_ANALYSIS_SIMPLIFYINSTRUCTION_H
