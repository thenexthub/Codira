//===- HexagonGenMux.cpp --------------------------------------------------===//
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

// During instruction selection, MUX instructions are generated for
// conditional assignments. Since such assignments often present an
// opportunity to predicate instructions, HexagonExpandCondsets
// expands MUXes into pairs of conditional transfers, and then proceeds
// with predication of the producers/consumers of the registers involved.
// This happens after exiting from the SSA form, but before the machine
// instruction scheduler. After the scheduler and after the register
// allocation there can be cases of pairs of conditional transfers
// resulting from a MUX where neither of them was further predicated. If
// these transfers are now placed far enough from the instruction defining
// the predicate register, they cannot use the .new form. In such cases it
// is better to collapse them back to a single MUX instruction.

#include "Hexagon.h"
#include "HexagonInstrInfo.h"
#include "HexagonRegisterInfo.h"
#include "HexagonSubtarget.h"
#include "vm/core/ADT/BitVector.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/CodeGen/LiveRegUnits.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/IR/DebugLoc.h"
#include "vm/core/MC/MCInstrDesc.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/MathExtras.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>

#define DEBUG_TYPE "hexmux"

using namespace vm::core;

// Initialize this to 0 to always prefer generating mux by default.
static cl::opt<unsigned> MinPredDist("hexagon-gen-mux-threshold", cl::Hidden,
  cl::init(0), cl::desc("Minimum distance between predicate definition and "
  "farther of the two predicated uses"));

namespace {

  class HexagonGenMux : public MachineFunctionPass {
  public:
    static char ID;

    HexagonGenMux() : MachineFunctionPass(ID) {}

    StringRef getPassName() const override {
      return "Hexagon generate mux instructions";
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      MachineFunctionPass::getAnalysisUsage(AU);
    }

    bool runOnMachineFunction(MachineFunction &MF) override;

    MachineFunctionProperties getRequiredProperties() const override {
      return MachineFunctionProperties().setNoVRegs();
    }

  private:
    const HexagonInstrInfo *HII = nullptr;
    const HexagonRegisterInfo *HRI = nullptr;

    struct CondsetInfo {
      unsigned PredR = 0;
      unsigned TrueX = std::numeric_limits<unsigned>::max();
      unsigned FalseX = std::numeric_limits<unsigned>::max();

      CondsetInfo() = default;
    };

    struct DefUseInfo {
      BitVector Defs, Uses;

      DefUseInfo() = default;
      DefUseInfo(const BitVector &D, const BitVector &U) : Defs(D), Uses(U) {}
    };

    struct MuxInfo {
      MachineBasicBlock::iterator At;
      unsigned DefR, PredR;
      MachineOperand *SrcT, *SrcF;
      MachineInstr *Def1, *Def2;

      MuxInfo(MachineBasicBlock::iterator It, unsigned DR, unsigned PR,
              MachineOperand *TOp, MachineOperand *FOp, MachineInstr &D1,
              MachineInstr &D2)
          : At(It), DefR(DR), PredR(PR), SrcT(TOp), SrcF(FOp), Def1(&D1),
            Def2(&D2) {}
    };

    using InstrIndexMap = DenseMap<MachineInstr *, unsigned>;
    using DefUseInfoMap = DenseMap<unsigned, DefUseInfo>;
    using MuxInfoList = SmallVector<MuxInfo, 4>;

    bool isRegPair(unsigned Reg) const {
      return Hexagon::DoubleRegsRegClass.contains(Reg);
    }

    void getSubRegs(unsigned Reg, BitVector &SRs) const;
    void expandReg(unsigned Reg, BitVector &Set) const;
    void getDefsUses(const MachineInstr *MI, BitVector &Defs,
          BitVector &Uses) const;
    void buildMaps(MachineBasicBlock &B, InstrIndexMap &I2X,
          DefUseInfoMap &DUM);
    bool isCondTransfer(unsigned Opc) const;
    unsigned getMuxOpcode(const MachineOperand &Src1,
          const MachineOperand &Src2) const;
    bool genMuxInBlock(MachineBasicBlock &B);
  };

} // end anonymous namespace

char HexagonGenMux::ID = 0;

INITIALIZE_PASS(HexagonGenMux, "hexagon-gen-mux",
  "Hexagon generate mux instructions", false, false)

void HexagonGenMux::getSubRegs(unsigned Reg, BitVector &SRs) const {
  for (MCPhysReg I : HRI->subregs(Reg))
    SRs[I] = true;
}

void HexagonGenMux::expandReg(unsigned Reg, BitVector &Set) const {
  if (isRegPair(Reg))
    getSubRegs(Reg, Set);
  else
    Set[Reg] = true;
}

void HexagonGenMux::getDefsUses(const MachineInstr *MI, BitVector &Defs,
      BitVector &Uses) const {
  // First, get the implicit defs and uses for this instruction.
  unsigned Opc = MI->getOpcode();
  const MCInstrDesc &D = HII->get(Opc);
  for (MCPhysReg R : D.implicit_defs())
    expandReg(R, Defs);
  for (MCPhysReg R : D.implicit_uses())
    expandReg(R, Uses);

  // Look over all operands, and collect explicit defs and uses.
  for (const MachineOperand &MO : MI->operands()) {
    if (!MO.isReg() || MO.isImplicit())
      continue;
    Register R = MO.getReg();
    BitVector &Set = MO.isDef() ? Defs : Uses;
    expandReg(R, Set);
  }
}

void HexagonGenMux::buildMaps(MachineBasicBlock &B, InstrIndexMap &I2X,
      DefUseInfoMap &DUM) {
  unsigned Index = 0;
  unsigned NR = HRI->getNumRegs();
  BitVector Defs(NR), Uses(NR);

  for (MachineInstr &MI : B) {
    I2X.insert(std::make_pair(&MI, Index));
    Defs.reset();
    Uses.reset();
    getDefsUses(&MI, Defs, Uses);
    DUM.insert(std::make_pair(Index, DefUseInfo(Defs, Uses)));
    Index++;
  }
}

bool HexagonGenMux::isCondTransfer(unsigned Opc) const {
  switch (Opc) {
    case Hexagon::A2_tfrt:
    case Hexagon::A2_tfrf:
    case Hexagon::C2_cmoveit:
    case Hexagon::C2_cmoveif:
      return true;
  }
  return false;
}

unsigned HexagonGenMux::getMuxOpcode(const MachineOperand &Src1,
      const MachineOperand &Src2) const {
  bool IsReg1 = Src1.isReg(), IsReg2 = Src2.isReg();
  if (IsReg1)
    return IsReg2 ? Hexagon::C2_mux : Hexagon::C2_muxir;
  if (IsReg2)
    return Hexagon::C2_muxri;

  // Neither is a register. The first source is extendable, but the second
  // is not (s8).
  if (Src2.isImm() && isInt<8>(Src2.getImm()))
    return Hexagon::C2_muxii;

  return 0;
}

bool HexagonGenMux::genMuxInBlock(MachineBasicBlock &B) {
  bool Changed = false;
  InstrIndexMap I2X;
  DefUseInfoMap DUM;
  buildMaps(B, I2X, DUM);

  using CondsetMap = DenseMap<unsigned, CondsetInfo>;

  CondsetMap CM;
  MuxInfoList ML;

  for (MachineInstr &MI : toolchain::make_early_inc_range(B)) {
    unsigned Opc = MI.getOpcode();
    if (!isCondTransfer(Opc))
      continue;
    Register DR = MI.getOperand(0).getReg();
    if (isRegPair(DR))
      continue;
    MachineOperand &PredOp = MI.getOperand(1);
    if (PredOp.isUndef())
      continue;

    Register PR = PredOp.getReg();
    unsigned Idx = I2X.lookup(&MI);
    CondsetMap::iterator F = CM.find(DR);
    bool IfTrue = HII->isPredicatedTrue(Opc);

    // If there is no record of a conditional transfer for this register,
    // or the predicate register differs, create a new record for it.
    if (F != CM.end() && F->second.PredR != PR) {
      CM.erase(F);
      F = CM.end();
    }
    if (F == CM.end()) {
      F = CM.try_emplace(DR).first;
      F->second.PredR = PR;
    }
    CondsetInfo &CI = F->second;
    if (IfTrue)
      CI.TrueX = Idx;
    else
      CI.FalseX = Idx;
    if (CI.TrueX == std::numeric_limits<unsigned>::max() ||
        CI.FalseX == std::numeric_limits<unsigned>::max())
      continue;

    // There is now a complete definition of DR, i.e. we have the predicate
    // register, the definition if-true, and definition if-false.

    // First, check if the definitions are far enough from the definition
    // of the predicate register.
    unsigned MinX = std::min(CI.TrueX, CI.FalseX);
    unsigned MaxX = std::max(CI.TrueX, CI.FalseX);
    // Specifically, check if the predicate definition is within a prescribed
    // distance from the farther of the two predicated instructions.
    unsigned SearchX = (MaxX >= MinPredDist) ? MaxX-MinPredDist : 0;
    bool NearDef = false;
    for (unsigned X = SearchX; X < MaxX; ++X) {
      const DefUseInfo &DU = DUM.lookup(X);
      if (!DU.Defs[PR])
        continue;
      NearDef = true;
      break;
    }
    if (NearDef)
      continue;

    // The predicate register is not defined in the last few instructions.
    // Check if the conversion to MUX is possible (either "up", i.e. at the
    // place of the earlier partial definition, or "down", where the later
    // definition is located). Examine all defs and uses between these two
    // definitions.
    // SR1, SR2 - source registers from the first and the second definition.
    MachineBasicBlock::iterator It1 = B.begin(), It2 = B.begin();
    std::advance(It1, MinX);
    std::advance(It2, MaxX);
    MachineInstr &Def1 = *It1, &Def2 = *It2;
    MachineOperand *Src1 = &Def1.getOperand(2), *Src2 = &Def2.getOperand(2);
    Register SR1 = Src1->isReg() ? Src1->getReg() : Register();
    Register SR2 = Src2->isReg() ? Src2->getReg() : Register();
    bool Failure = false, CanUp = true, CanDown = true;
    for (unsigned X = MinX+1; X < MaxX; X++) {
      const DefUseInfo &DU = DUM.lookup(X);
      if (DU.Defs[PR] || DU.Defs[DR] || DU.Uses[DR]) {
        Failure = true;
        break;
      }
      if (CanDown && DU.Defs[SR1])
        CanDown = false;
      if (CanUp && DU.Defs[SR2])
        CanUp = false;
    }
    if (Failure || (!CanUp && !CanDown))
      continue;

    MachineOperand *SrcT = (MinX == CI.TrueX) ? Src1 : Src2;
    MachineOperand *SrcF = (MinX == CI.FalseX) ? Src1 : Src2;
    // Prefer "down", since this will move the MUX farther away from the
    // predicate definition.
    MachineBasicBlock::iterator At = CanDown ? Def2 : Def1;
    ML.push_back(MuxInfo(At, DR, PR, SrcT, SrcF, Def1, Def2));
  }

  for (MuxInfo &MX : ML) {
    unsigned MxOpc = getMuxOpcode(*MX.SrcT, *MX.SrcF);
    if (!MxOpc)
      continue;
    // Basic correctness check: since we are deleting instructions, validate the
    // iterators. There is a possibility that one of Def1 or Def2 is translated
    // to "mux" and being considered for other "mux" instructions.
    if (!MX.At->getParent() || !MX.Def1->getParent() || !MX.Def2->getParent())
      continue;

    MachineBasicBlock &B = *MX.At->getParent();
    const DebugLoc &DL = B.findDebugLoc(MX.At);
    auto NewMux = BuildMI(B, MX.At, DL, HII->get(MxOpc), MX.DefR)
                      .addReg(MX.PredR)
                      .add(*MX.SrcT)
                      .add(*MX.SrcF);
    NewMux->clearKillInfo();
    B.remove(MX.Def1);
    B.remove(MX.Def2);
    Changed = true;
  }

  // Fix up kill flags.

  LiveRegUnits LPR(*HRI);
  LPR.addLiveOuts(B);
  for (MachineInstr &I : toolchain::reverse(B)) {
    if (I.isDebugInstr())
      continue;
    // This isn't 100% accurate, but it's safe.
    // It won't detect (as a kill) a case like this
    //   r0 = add r0, 1    <-- r0 should be "killed"
    //   ... = r0
    for (MachineOperand &Op : I.operands()) {
      if (!Op.isReg() || !Op.isUse())
        continue;
      assert(Op.getSubReg() == 0 && "Should have physical registers only");
      bool Live = !LPR.available(Op.getReg());
      Op.setIsKill(!Live);
    }
    LPR.stepBackward(I);
  }

  return Changed;
}

bool HexagonGenMux::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;
  HII = MF.getSubtarget<HexagonSubtarget>().getInstrInfo();
  HRI = MF.getSubtarget<HexagonSubtarget>().getRegisterInfo();
  bool Changed = false;
  for (auto &I : MF)
    Changed |= genMuxInBlock(I);
  return Changed;
}

FunctionPass *toolchain::createHexagonGenMux() {
  return new HexagonGenMux();
}
