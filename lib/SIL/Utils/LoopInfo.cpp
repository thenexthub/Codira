//===--- LoopInfo.cpp - SIL Loop Analysis ---------------------------------===//
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

#include "language/SIL/LoopInfo.h"
#include "language/SIL/Dominance.h"
#include "toolchain/Support/GenericLoopInfoImpl.h"
#include "toolchain/Support/Debug.h"

using namespace language;

// Instantiate template members.
template class toolchain::LoopBase<SILBasicBlock, SILLoop>;
template class toolchain::LoopInfoBase<SILBasicBlock, SILLoop>;


void SILLoop::dump() const {
#if !defined(NDEBUG) || defined(TOOLCHAIN_ENABLE_DUMP)
  print(toolchain::dbgs());
#endif
}

SILLoopInfo::SILLoopInfo(SILFunction *F, DominanceInfo *DT) : Dominance(DT) {
  LI.analyze(*Dominance);
}

void SILLoopInfo::verify() const {
  LI.verify(*Dominance);
}
