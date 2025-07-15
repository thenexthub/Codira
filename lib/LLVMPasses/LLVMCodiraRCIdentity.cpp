//===--- LLVMCodiraRCIdentity.cpp - LLVM RCIdentity Analysis for Codira -----===//
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

#include "language/LLVMPasses/Passes.h"
#include "LLVMARCOpts.h"
#include "toolchain/IR/Module.h"

using namespace toolchain;
using language::CodiraRCIdentity;

toolchain::Value *
CodiraRCIdentity::stripPointerCasts(toolchain::Value *Val) {
  return Val->stripPointerCasts();
}

toolchain::Value *
CodiraRCIdentity::stripReferenceForwarding(toolchain::Value *Val) {
  auto Inst = dyn_cast<Instruction>(Val);
  if (!Inst)
    return Val;
  auto Kind = classifyInstruction(*Inst);
  switch(Kind) {
  case RT_RetainN:
  case RT_UnknownObjectRetainN:
  case RT_BridgeRetainN:
  case RT_ReleaseN:
  case RT_UnknownObjectReleaseN:
  case RT_BridgeReleaseN:
  case RT_FixLifetime:
  case RT_Retain:
  case RT_UnknownObjectRetain:
  case RT_Release:
  case RT_UnknownObjectRelease:
  case RT_Unknown:
  case RT_AllocObject:
  case RT_NoMemoryAccessed:
  case RT_BridgeRelease:
  case RT_BridgeRetain:
  case RT_RetainUnowned:
  case RT_CheckUnowned:
  case RT_ObjCRelease:
  case RT_EndBorrow:
    break;
  // ObjC forwards references.
  case RT_ObjCRetain:
    Val = cast<CallInst>(Inst)->getArgOperand(0);
    break;
  }
  return Val;
}

toolchain::Value *
CodiraRCIdentity::getCodiraRCIdentityRoot(toolchain::Value *Val) {
  // Only allow this method to go up a fixed number of levels to make sure
  // we don't explode compile time.
  toolchain::Value *OldVal = Val;
  unsigned levels = 0;
  do {
    toolchain::Value *NewVal = Val;
    // Try to strip off pointer casts and reference forwarding.
    Val = stripPointerCasts(Val);
    Val = stripReferenceForwarding(Val);
    // Nothing was stripped off.
    if (NewVal == Val)
      break;
    // Hit max number of levels.
    if (++levels > MaxRecursionDepth)
      return OldVal;
  } while (true);
  return Val;
}
