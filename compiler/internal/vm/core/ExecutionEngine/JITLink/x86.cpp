//===-------- x86.cpp - Generic JITLink x86 edge kinds, utilities ---------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// Generic utilities for graphs representing x86 objects.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ExecutionEngine/JITLink/x86.h"

#define DEBUG_TYPE "jitlink"

namespace vm::core::jitlink::x86 {

const char *getEdgeKindName(Edge::Kind K) {
  switch (K) {
  case Pointer32:
    return "Pointer32";
  case PCRel32:
    return "PCRel32";
  case Pointer16:
    return "Pointer16";
  case PCRel16:
    return "PCRel16";
  case Delta32:
    return "Delta32";
  case Delta32FromGOT:
    return "Delta32FromGOT";
  case RequestGOTAndTransformToDelta32FromGOT:
    return "RequestGOTAndTransformToDelta32FromGOT";
  case BranchPCRel32:
    return "BranchPCRel32";
  case BranchPCRel32ToPtrJumpStub:
    return "BranchPCRel32ToPtrJumpStub";
  case BranchPCRel32ToPtrJumpStubBypassable:
    return "BranchPCRel32ToPtrJumpStubBypassable";
  }

  return getGenericEdgeKindName(K);
}

const char NullPointerContent[PointerSize] = {0x00, 0x00, 0x00, 0x00};

const char PointerJumpStubContent[6] = {
    static_cast<char>(0xFFu), 0x25, 0x00, 0x00, 0x00, 0x00};

Error optimizeGOTAndStubAccesses(LinkGraph &G) {
  LLVM_DEBUG(dbgs() << "Optimizing GOT entries and stubs:\n");

  for (auto *B : G.blocks())
    for (auto &E : B->edges()) {
      if (E.getKind() == BranchPCRel32ToPtrJumpStubBypassable) {
        auto &StubBlock = E.getTarget().getBlock();
        assert(StubBlock.getSize() == sizeof(PointerJumpStubContent) &&
               "Stub block should be stub sized");
        assert(StubBlock.edges_size() == 1 &&
               "Stub block should only have one outgoing edge");

        auto &GOTBlock = StubBlock.edges().begin()->getTarget().getBlock();
        assert(GOTBlock.getSize() == G.getPointerSize() &&
               "GOT block should be pointer sized");
        assert(GOTBlock.edges_size() == 1 &&
               "GOT block should only have one outgoing edge");

        auto &GOTTarget = GOTBlock.edges().begin()->getTarget();
        orc::ExecutorAddr EdgeAddr = B->getAddress() + E.getOffset();
        orc::ExecutorAddr TargetAddr = GOTTarget.getAddress();

        int64_t Displacement = TargetAddr - EdgeAddr + 4;
        if (isInt<32>(Displacement)) {
          E.setKind(BranchPCRel32);
          E.setTarget(GOTTarget);
          LLVM_DEBUG({
            dbgs() << "  Replaced stub branch with direct branch:\n    ";
            printEdge(dbgs(), *B, E, getEdgeKindName(E.getKind()));
            dbgs() << "\n";
          });
        }
      }
    }

  return Error::success();
}

} // namespace vm::core::jitlink::x86
