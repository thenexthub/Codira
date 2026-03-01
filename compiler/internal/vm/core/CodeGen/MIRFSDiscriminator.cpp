//===-------- MIRFSDiscriminator.cpp: Flow Sensitive Discriminator --------===//
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
// This file provides the implementation of a machine pass that adds the flow
// sensitive discriminator to the instruction debug information.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MIRFSDiscriminator.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/DenseSet.h"
#include "vm/core/Analysis/BlockFrequencyInfoImpl.h"
#include "vm/core/CodeGen/MIRFSDiscriminatorOptions.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/IR/DebugInfoMetadata.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PseudoProbe.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Support/xxhash.h"
#include "vm/core/Transforms/Utils/SampleProfileLoaderBaseUtil.h"

using namespace vm::core;
using namespace sampleprof;
using namespace sampleprofutil;

#define DEBUG_TYPE "mirfs-discriminators"

// TODO(xur): Remove this option and related code once we make true as the
// default.
cl::opt<bool> toolchain::ImprovedFSDiscriminator(
    "improved-fs-discriminator", cl::Hidden, cl::init(false),
    cl::desc("New FS discriminators encoding (incompatible with the original "
             "encoding)"));
char MIRAddFSDiscriminators::ID = 0;

INITIALIZE_PASS(MIRAddFSDiscriminators, DEBUG_TYPE,
                "Add MIR Flow Sensitive Discriminators",
                /* cfg = */ false, /* is_analysis = */ false)

char &toolchain::MIRAddFSDiscriminatorsID = MIRAddFSDiscriminators::ID;

FunctionPass *toolchain::createMIRAddFSDiscriminatorsPass(FSDiscriminatorPass P) {
  return new MIRAddFSDiscriminators(P);
}

// TODO(xur): Remove this once we switch to ImprovedFSDiscriminator.
// Compute a hash value using debug line number, and the line numbers from the
// inline stack.
static uint64_t getCallStackHashV0(const MachineBasicBlock &BB,
                                   const MachineInstr &MI,
                                   const DILocation *DIL) {
  auto updateHash = [](const StringRef &Str) -> uint64_t {
    if (Str.empty())
      return 0;
    return MD5Hash(Str);
  };
  uint64_t Ret = updateHash(std::to_string(DIL->getLine()));
  Ret ^= updateHash(BB.getName());
  Ret ^= updateHash(DIL->getScope()->getSubprogram()->getLinkageName());
  for (DIL = DIL->getInlinedAt(); DIL; DIL = DIL->getInlinedAt()) {
    Ret ^= updateHash(std::to_string(DIL->getLine()));
    Ret ^= updateHash(DIL->getScope()->getSubprogram()->getLinkageName());
  }
  return Ret;
}

static uint64_t getCallStackHash(const DILocation *DIL) {
  auto hashCombine = [](const uint64_t Seed, const uint64_t Val) {
    std::hash<uint64_t> Hasher;
    return Seed ^ (Hasher(Val) + 0x9e3779b9 + (Seed << 6) + (Seed >> 2));
  };
  uint64_t Ret = 0;
  for (DIL = DIL->getInlinedAt(); DIL; DIL = DIL->getInlinedAt()) {
    Ret = hashCombine(Ret, xxh3_64bits(ArrayRef<uint8_t>(DIL->getLine())));
    Ret = hashCombine(Ret, xxh3_64bits(DIL->getSubprogramLinkageName()));
  }
  return Ret;
}

// Traverse the CFG and assign FD discriminators. If two instructions
// have the same lineno and discriminator, but residing in different BBs,
// the latter instruction will get a new discriminator value. The new
// discriminator keeps the existing discriminator value but sets new bits
// b/w LowBit and HighBit.
bool MIRAddFSDiscriminators::runOnMachineFunction(MachineFunction &MF) {
  if (!EnableFSDiscriminator)
    return false;

  bool HasPseudoProbe = MF.getFunction().getParent()->getNamedMetadata(
      PseudoProbeDescMetadataName);

  if (!HasPseudoProbe && !MF.getFunction().shouldEmitDebugInfoForProfiling())
    return false;

  bool Changed = false;
  using LocationDiscriminator =
      std::tuple<StringRef, unsigned, unsigned, uint64_t>;
  using BBSet = DenseSet<const MachineBasicBlock *>;
  using LocationDiscriminatorBBMap = DenseMap<LocationDiscriminator, BBSet>;
  using LocationDiscriminatorCurrPassMap =
      DenseMap<LocationDiscriminator, unsigned>;

  LocationDiscriminatorBBMap LDBM;
  LocationDiscriminatorCurrPassMap LDCM;

  // Mask of discriminators before this pass.
  // TODO(xur): simplify this once we switch to ImprovedFSDiscriminator.
  unsigned LowBitTemp = LowBit;
  assert(LowBit > 0 && "LowBit in FSDiscriminator cannot be 0");
  if (ImprovedFSDiscriminator)
    LowBitTemp -= 1;
  unsigned BitMaskBefore = getN1Bits(LowBitTemp);
  // Mask of discriminators including this pass.
  unsigned BitMaskNow = getN1Bits(HighBit);
  // Mask of discriminators for bits specific to this pass.
  unsigned BitMaskThisPass = BitMaskNow ^ BitMaskBefore;
  unsigned NumNewD = 0;

  LLVM_DEBUG(dbgs() << "MIRAddFSDiscriminators working on Func: "
                    << MF.getFunction().getName() << " Highbit=" << HighBit
                    << "\n");

  for (MachineBasicBlock &BB : MF) {
    for (MachineInstr &I : BB) {
      if (HasPseudoProbe) {
        // Only assign discriminators to pseudo probe instructions. Call
        // instructions are excluded since their dwarf discriminators are used
        // for other purposes, i.e, storing probe ids.
        if (!I.isPseudoProbe())
          continue;
      } else if (ImprovedFSDiscriminator && I.isMetaInstruction()) {
        continue;
      }
      const DILocation *DIL = I.getDebugLoc().get();
      if (!DIL)
        continue;

      // Use the id of pseudo probe to compute the discriminator.
      unsigned LineNo =
          I.isPseudoProbe() ? I.getOperand(1).getImm() : DIL->getLine();
      if (LineNo == 0)
        continue;
      unsigned Discriminator = DIL->getDiscriminator();
      // Clean up discriminators for pseudo probes at the first FS discriminator
      // pass as their discriminators should not ever be used.
      if ((Pass == FSDiscriminatorPass::Pass1) && I.isPseudoProbe()) {
        Discriminator = 0;
        I.setDebugLoc(DIL->cloneWithDiscriminator(0));
      }
      uint64_t CallStackHashVal = 0;
      if (ImprovedFSDiscriminator)
        CallStackHashVal = getCallStackHash(DIL);

      LocationDiscriminator LD{DIL->getFilename(), LineNo, Discriminator,
                               CallStackHashVal};
      auto &BBMap = LDBM[LD];
      auto R = BBMap.insert(&BB);
      if (BBMap.size() == 1)
        continue;

      unsigned DiscriminatorCurrPass;
      DiscriminatorCurrPass = R.second ? ++LDCM[LD] : LDCM[LD];
      DiscriminatorCurrPass = DiscriminatorCurrPass << LowBit;
      if (!ImprovedFSDiscriminator)
        DiscriminatorCurrPass += getCallStackHashV0(BB, I, DIL);
      DiscriminatorCurrPass &= BitMaskThisPass;
      unsigned NewD = Discriminator | DiscriminatorCurrPass;
      const auto *const NewDIL = DIL->cloneWithDiscriminator(NewD);
      if (!NewDIL) {
        LLVM_DEBUG(dbgs() << "Could not encode discriminator: "
                          << DIL->getFilename() << ":" << DIL->getLine() << ":"
                          << DIL->getColumn() << ":" << Discriminator << " "
                          << I << "\n");
        continue;
      }

      I.setDebugLoc(NewDIL);
      NumNewD++;
      LLVM_DEBUG(dbgs() << DIL->getFilename() << ":" << DIL->getLine() << ":"
                        << DIL->getColumn() << ": add FS discriminator, from "
                        << Discriminator << " -> " << NewD << "\n");
      Changed = true;
    }
  }

  if (Changed) {
    createFSDiscriminatorVariable(MF.getFunction().getParent());
    LLVM_DEBUG(dbgs() << "Num of FS Discriminators: " << NumNewD << "\n");
    (void) NumNewD;
  }

  return Changed;
}
