//===--- SILInstructionWorklist.cpp ---------------------------------------===//
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

#define DEBUG_TYPE "sil-instruction-worklist"
#include "language/SIL/SILInstructionWorklist.h"

using namespace language;

void SILInstructionWorklistBase::withDebugStream(
    std::function<void(toolchain::raw_ostream &stream, const char *loggingName)>
        perform) {
#ifndef NDEBUG
  TOOLCHAIN_DEBUG(perform(toolchain::dbgs(), loggingName));
#endif
}
