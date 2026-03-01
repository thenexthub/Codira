//===---- systemz.cpp - Generic JITLink systemz edge kinds, utilities -----===//
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
// Generic utilities for graphs representing systemz objects.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ExecutionEngine/JITLink/systemz.h"

#define DEBUG_TYPE "jitlink"

namespace vm::core {
namespace jitlink {
namespace systemz {

const char NullPointerContent[8] = {0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00};

const char Pointer64JumpStubContent[8] = {
    static_cast<char>(0xC4u),
    0x18,
    0x00,
    0x00,
    0x00,
    0x00, // lgrl r1
    static_cast<char>(0x07u),
    static_cast<char>(0xF1u), // BCR 15, 1
};

const char *getEdgeKindName(Edge::Kind R) {
  switch (R) {
  case Pointer64:
    return "Pointer64";
  case Pointer32:
    return "Pointer32";
  case Pointer20:
    return "Pointer20";
  case Pointer16:
    return "Pointer16";
  case Pointer12:
    return "Pointer12";
  case Pointer8:
    return "Pointer8";
  case Delta64:
    return "Delta64";
  case Delta32:
    return "Delta32";
  case Delta16:
    return "Delta16";
  case Delta32dbl:
    return "Delta32dbl";
  case Delta24dbl:
    return "Delta24dbl";
  case Delta16dbl:
    return "Delta16dbl";
  case Delta12dbl:
    return "Delta12dbl";
  case NegDelta64:
    return "NegDelta64";
  case NegDelta32:
    return "NegDelta32";
  case DeltaPLT32dbl:
    return "DeltaPLT32dbl";
  case DeltaPLT24dbl:
    return "DeltaPLT24dbl";
  case DeltaPLT16dbl:
    return "DeltaPLT16dbl";
  case DeltaPLT12dbl:
    return "DeltaPLT12dbl";
  case DeltaPLT64:
    return "DeltaPLT64";
  case DeltaPLT32:
    return "DeltaPLT32";
  case Delta64FromGOT:
    return "Delta64FromGOT";
  case Delta32FromGOT:
    return "Delta32FromGOT";
  case Delta16FromGOT:
    return "Delta16FromGOT";
  case Delta64PLTFromGOT:
    return "Delta64PLTFromGOT";
  case Delta32PLTFromGOT:
    return "Delta32PLTFromGOT";
  case Delta16PLTFromGOT:
    return "Delta16PLTFromGOT";
  case Delta32GOTBase:
    return "Delta32GOTBase";
  case Delta32dblGOTBase:
    return "Delta32dblGOTBase";
  case RequestGOTAndTransformToDelta64FromGOT:
    return "RequestGOTAndTransformToDelta64FromGOT";
  case RequestGOTAndTransformToDelta32FromGOT:
    return "RequestGOTAndTransformToDelta32FromGOT";
  case RequestGOTAndTransformToDelta20FromGOT:
    return "RequestGOTAndTransformToDelta20FromGOT";
  case RequestGOTAndTransformToDelta16FromGOT:
    return "RequestGOTAndTransformToDelta16FromGOT";
  case RequestGOTAndTransformToDelta12FromGOT:
    return "RequestGOTAndTransformToDelta12FromGOT";
  case RequestGOTAndTransformToDelta32dbl:
    return "RequestGOTAndTransformToDelta32dbl";
  case RequestTLSDescInGOTAndTransformToDelta64FromGOT:
    return "RequestTLSDescInGOTAndTransformToDelta64FromGOT";
  default:
    return getGenericEdgeKindName(static_cast<Edge::Kind>(R));
  }
}

Error optimizeGOTAndStubAccesses(LinkGraph &G) {
  LLVM_DEBUG(dbgs() << "Optimizing GOT entries and stubs:\n");

  for (auto *B : G.blocks())
    for (auto &E : B->edges()) {
      if (E.getKind() == systemz::DeltaPLT32dbl) {
        auto &StubBlock = E.getTarget().getBlock();
        if (StubBlock.getSize() == sizeof(Pointer64JumpStubContent) &&
            StubBlock.edges_size() == 1) {
          auto &GOTBlock = StubBlock.edges().begin()->getTarget().getBlock();
          assert(GOTBlock.getSize() == G.getPointerSize() &&
                 "GOT block should be pointer sized");
          assert(GOTBlock.edges_size() == 1 &&
                 "GOT block should only have one outgoing edge");

          auto &GOTTarget = GOTBlock.edges().begin()->getTarget();
          orc::ExecutorAddr EdgeAddr = B->getAddress() + E.getOffset();
          orc::ExecutorAddr TargetAddr = GOTTarget.getAddress();

          int64_t Displacement = TargetAddr + E.getAddend() - EdgeAddr;
          if (isInt<33>(Displacement)) {
            E.setKind(systemz::Delta32dbl);
            E.setTarget(GOTTarget);
            LLVM_DEBUG({
              dbgs() << "  Replaced stub branch with direct branch:\n    ";
              printEdge(dbgs(), *B, E, getEdgeKindName(E.getKind()));
              dbgs() << "\n";
            });
          }
        }
      }
    }

  return Error::success();
}

} // namespace systemz
} // namespace jitlink
} // namespace vm::core
