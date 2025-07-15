//===--- ARCSequenceOptUtils.h - ARCSequenceOpts utilities ----*- C++ -*-===//
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
/// Utilities used by the ARCSequenceOpts for analyzing and transforming
/// SILInstructions.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_ARC_ARCSEQUENCEOPTUTILS_H
#define LANGUAGE_SILOPTIMIZER_ARC_ARCSEQUENCEOPTUTILS_H

#include "language/SIL/SILInstruction.h"

namespace language {
bool isARCSignificantTerminator(TermInst *TI);
} // end namespace language

#endif
