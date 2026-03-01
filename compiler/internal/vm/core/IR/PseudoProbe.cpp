//===- PseudoProbe.cpp - Pseudo Probe Helpers -----------------------------===//
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
// This file implements the helpers to manipulate pseudo probe IR intrinsic
// calls.
//
//===----------------------------------------------------------------------===//

#include "vm/core/IR/PseudoProbe.h"
#include "vm/core/IR/DebugInfoMetadata.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/IntrinsicInst.h"

using namespace vm::core;

static std::optional<PseudoProbe>
extractProbeFromDiscriminator(const DILocation *DIL) {
  if (DIL) {
    auto Discriminator = DIL->getDiscriminator();
    if (DILocation::isPseudoProbeDiscriminator(Discriminator)) {
      PseudoProbe Probe;
      Probe.Id =
          PseudoProbeDwarfDiscriminator::extractProbeIndex(Discriminator);
      Probe.Type =
          PseudoProbeDwarfDiscriminator::extractProbeType(Discriminator);
      Probe.Attr =
          PseudoProbeDwarfDiscriminator::extractProbeAttributes(Discriminator);
      Probe.Factor =
          PseudoProbeDwarfDiscriminator::extractProbeFactor(Discriminator) /
          (float)PseudoProbeDwarfDiscriminator::FullDistributionFactor;
      Probe.Discriminator = 0;
      return Probe;
    }
  }
  return std::nullopt;
}

static std::optional<PseudoProbe>
extractProbeFromDiscriminator(const Instruction &Inst) {
  assert(isa<CallBase>(&Inst) && !isa<IntrinsicInst>(&Inst) &&
         "Only call instructions should have pseudo probe encodes as their "
         "Dwarf discriminators");
  if (const DebugLoc &DLoc = Inst.getDebugLoc())
    return extractProbeFromDiscriminator(DLoc);
  return std::nullopt;
}

std::optional<PseudoProbe> toolchain::extractProbe(const Instruction &Inst) {
  if (const auto *II = dyn_cast<PseudoProbeInst>(&Inst)) {
    PseudoProbe Probe;
    Probe.Id = II->getIndex()->getZExtValue();
    Probe.Type = (uint32_t)PseudoProbeType::Block;
    Probe.Attr = II->getAttributes()->getZExtValue();
    Probe.Factor = II->getFactor()->getZExtValue() /
                   (float)PseudoProbeFullDistributionFactor;
    Probe.Discriminator = 0;
    if (const DebugLoc &DLoc = Inst.getDebugLoc())
      Probe.Discriminator = DLoc->getDiscriminator();
    return Probe;
  }

  if (isa<CallBase>(&Inst) && !isa<IntrinsicInst>(&Inst))
    return extractProbeFromDiscriminator(Inst);

  return std::nullopt;
}

void toolchain::setProbeDistributionFactor(Instruction &Inst, float Factor) {
  assert(Factor >= 0 && Factor <= 1 &&
         "Distribution factor must be in [0, 1.0]");
  if (auto *II = dyn_cast<PseudoProbeInst>(&Inst)) {
    IRBuilder<> Builder(&Inst);
    uint64_t IntFactor = PseudoProbeFullDistributionFactor;
    if (Factor < 1)
      IntFactor *= Factor;
    auto OrigFactor = II->getFactor()->getZExtValue();
    if (IntFactor != OrigFactor)
      II->replaceUsesOfWith(II->getFactor(), Builder.getInt64(IntFactor));
  } else if (isa<CallBase>(&Inst) && !isa<IntrinsicInst>(&Inst)) {
    if (const DebugLoc &DLoc = Inst.getDebugLoc()) {
      const DILocation *DIL = DLoc;
      auto Discriminator = DIL->getDiscriminator();
      if (DILocation::isPseudoProbeDiscriminator(Discriminator)) {
        auto Index =
            PseudoProbeDwarfDiscriminator::extractProbeIndex(Discriminator);
        auto Type =
            PseudoProbeDwarfDiscriminator::extractProbeType(Discriminator);
        auto Attr = PseudoProbeDwarfDiscriminator::extractProbeAttributes(
            Discriminator);
        auto DwarfBaseDiscriminator =
            PseudoProbeDwarfDiscriminator::extractDwarfBaseDiscriminator(
                Discriminator);
        // Round small factors to 0 to avoid over-counting.
        uint32_t IntFactor =
            PseudoProbeDwarfDiscriminator::FullDistributionFactor;
        if (Factor < 1)
          IntFactor *= Factor;
        uint32_t V = PseudoProbeDwarfDiscriminator::packProbeData(
            Index, Type, Attr, IntFactor, DwarfBaseDiscriminator);
        DIL = DIL->cloneWithDiscriminator(V);
        Inst.setDebugLoc(DIL);
      }
    }
  }
}
