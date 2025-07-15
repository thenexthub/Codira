//===--- JumpDest.h - Jump Destination Representation -----------*- C++ -*-===//
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
// Types relating to branch destinations.
//
//===----------------------------------------------------------------------===//

#ifndef JUMPDEST_H
#define JUMPDEST_H

#include "language/Basic/Assertions.h"
#include "language/SIL/SILLocation.h"
#include "toolchain/Support/Compiler.h"
#include "Cleanup.h"

namespace language {
  class SILBasicBlock;
  class CaseStmt;
  
namespace Lowering {

struct TOOLCHAIN_LIBRARY_VISIBILITY ThrownErrorInfo {
  SILValue IndirectErrorResult;
  bool Discard;

  explicit ThrownErrorInfo(SILValue indirectErrorAddr, bool discard=false)
    : IndirectErrorResult(indirectErrorAddr), Discard(discard) {}

  static ThrownErrorInfo forDiscard() {
    return ThrownErrorInfo(SILValue(), /*discard=*/true);
  }
};

/// The destination of a direct jump.  Codira currently does not
/// support indirect branches or goto, so the jump mechanism only
/// needs to worry about branches out of scopes, not into them.
class TOOLCHAIN_LIBRARY_VISIBILITY JumpDest {
  SILBasicBlock *Block = nullptr;
  CleanupsDepth Depth = CleanupsDepth::invalid();
  CleanupLocation CleanupLoc;
  std::optional<ThrownErrorInfo> ThrownError;

public:
  JumpDest(CleanupLocation L) : CleanupLoc(L) {}

  JumpDest(SILBasicBlock *block, CleanupsDepth depth, CleanupLocation l,
           std::optional<ThrownErrorInfo> ThrownError = std::nullopt)
      : Block(block), Depth(depth), CleanupLoc(l), ThrownError(ThrownError) {}

  SILBasicBlock *getBlock() const { return Block; }
  SILBasicBlock *takeBlock() {
    auto *BB = Block;
    Block = nullptr;
    return BB;
  }
  CleanupsDepth getDepth() const { return Depth; }
  CleanupLocation getCleanupLocation() const { return CleanupLoc; }

  ThrownErrorInfo getThrownError() const {
    assert(ThrownError);
    return *ThrownError;
  }

  JumpDest translate(CleanupsDepth NewDepth) && {
    assert(!ThrownError);

    JumpDest NewValue(Block, NewDepth, CleanupLoc);
    Block = nullptr;
    Depth = CleanupsDepth::invalid();
    // Null location.
    CleanupLoc = CleanupLocation(ArtificialUnreachableLocation());
    return NewValue;
  }

  bool isValid() const { return Block != nullptr; }
  static JumpDest invalid() {
    return JumpDest(CleanupLocation::invalid());
  }
};
  
} // end namespace Lowering
} // end namespace language

#endif
