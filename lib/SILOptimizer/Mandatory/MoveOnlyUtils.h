//===--- MoveOnlyUtils.h --------------------------------------------------===//
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

#ifndef LANGUAGE_SILOPTIMIZER_MANDATORY_MOVEONLYUTILS_H
#define LANGUAGE_SILOPTIMIZER_MANDATORY_MOVEONLYUTILS_H

namespace language {

class SILFunction;
class MarkUnresolvedNonCopyableValueInst;
class Operand;

namespace siloptimizer {

class DiagnosticEmitter;

bool cleanupNonCopyableCopiesAfterEmittingDiagnostic(SILFunction *fn);

/// Emit an error if we missed any copies when running markers. To check if a
/// diagnostic was emitted, use \p diagnosticEmitter.getDiagnosticCount().
void emitCheckerMissedCopyOfNonCopyableTypeErrors(
    SILFunction *fn, DiagnosticEmitter &diagnosticEmitter);

bool eliminateTemporaryAllocationsFromLet(
    MarkUnresolvedNonCopyableValueInst *markedInst);

namespace noncopyable {

bool memInstMustConsume(Operand *memOper);
bool memInstMustReinitialize(Operand *memOper);
bool memInstMustInitialize(Operand *memOper);

} // namespace noncopyable

} // namespace siloptimizer

} // namespace language

#endif
