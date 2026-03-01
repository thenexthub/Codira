//===-- PPCTOCRegDeps.cpp - Add Extra TOC Register Dependencies -----------===//
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
// When resolving an address using the ELF ABI TOC pointer, two relocations are
// generally required: one for the high part and one for the low part. Only
// the high part generally explicitly depends on r2 (the TOC pointer). And, so,
// we might produce code like this:
//
// .Ltmp526:
//         addis 3, 2, .LC12@toc@ha
// .Ltmp1628:
//         std 2, 40(1)
//         ld 5, 0(27)
//         ld 2, 8(27)
//         ld 11, 16(27)
//         ld 3, .LC12@toc@l(3)
//         rldicl 4, 4, 0, 32
//         mtctr 5
//         bctrl
//         ld 2, 40(1)
//
// And there is nothing wrong with this code, as such, but there is a linker bug
// in binutils (https://sourceware.org/bugzilla/show_bug.cgi?id=18414) that will
// misoptimize this code sequence to this:
//         nop
//         std     r2,40(r1)
//         ld      r5,0(r27)
//         ld      r2,8(r27)
//         ld      r11,16(r27)
//         ld      r3,-32472(r2)
//         clrldi  r4,r4,32
//         mtctr   r5
//         bctrl
//         ld      r2,40(r1)
// because the linker does not know (and does not check) that the value in r2
// changed in between the instruction using the .LC12@toc@ha (TOC-relative)
// relocation and the instruction using the .LC12@toc@l(3) relocation.
// Because it finds these instructions using the relocations (and not by
// scanning the instructions), it has been asserted that there is no good way
// to detect the change of r2 in between. As a result, this bug may never be
// fixed (i.e. it may become part of the definition of the ABI). GCC was
// updated to add extra dependencies on r2 to instructions using the @toc@l
// relocations to avoid this problem, and we'll do the same here.
//
// This is done as a separate pass because:
//  1. These extra r2 dependencies are not really properties of the
//     instructions, but rather due to a linker bug, and maybe one day we'll be
//     able to get rid of them when targeting linkers without this bug (and,
//     thus, keeping the logic centralized here will make that
//     straightforward).
//  2. There are ISel-level peephole optimizations that propagate the @toc@l
//     relocations to some user instructions, and so the exta dependencies do
//     not apply only to a fixed set of instructions (without undesirable
//     definition replication).
//
//===----------------------------------------------------------------------===//

#include "PPC.h"
#include "PPCInstrInfo.h"
#include "PPCTargetMachine.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;

#define DEBUG_TYPE "ppc-toc-reg-deps"

namespace {
  // PPCTOCRegDeps pass - For simple functions without epilogue code, move
  // returns up, and create conditional returns, to avoid unnecessary
  // branch-to-blr sequences.
  struct PPCTOCRegDeps : public MachineFunctionPass {
    static char ID;
    PPCTOCRegDeps() : MachineFunctionPass(ID) {}

  protected:
    bool hasTOCLoReloc(const MachineInstr &MI) {
      if (MI.getOpcode() == PPC::LDtocL || MI.getOpcode() == PPC::ADDItocL8 ||
          MI.getOpcode() == PPC::LWZtocL)
        return true;

      for (const MachineOperand &MO : MI.operands()) {
        if (MO.getTargetFlags() == PPCII::MO_TOC_LO)
          return true;
      }

      return false;
    }

    bool processBlock(MachineBasicBlock &MBB) {
      bool Changed = false;

      const bool isPPC64 =
          MBB.getParent()->getSubtarget<PPCSubtarget>().isPPC64();
      const unsigned TOCReg = isPPC64 ? PPC::X2 : PPC::R2;

      for (auto &MI : MBB) {
        if (!hasTOCLoReloc(MI))
          continue;

        MI.addOperand(MachineOperand::CreateReg(TOCReg,
                                                false  /*IsDef*/,
                                                true  /*IsImp*/));
        Changed = true;
      }

      return Changed;
    }

public:
    bool runOnMachineFunction(MachineFunction &MF) override {
      bool Changed = false;

      for (MachineBasicBlock &B : toolchain::make_early_inc_range(MF))
        if (processBlock(B))
          Changed = true;

      return Changed;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      MachineFunctionPass::getAnalysisUsage(AU);
    }
  };
}

INITIALIZE_PASS(PPCTOCRegDeps, DEBUG_TYPE,
                "PowerPC TOC Register Dependencies", false, false)

char PPCTOCRegDeps::ID = 0;
FunctionPass*
toolchain::createPPCTOCRegDepsPass() { return new PPCTOCRegDeps(); }

