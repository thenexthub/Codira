//===- MachineUniformityAnalysis.cpp --------------------------------------===//
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

#include "vm/core/CodeGen/MachineUniformityAnalysis.h"
#include "vm/core/ADT/GenericUniformityImpl.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/MachineCycleAnalysis.h"
#include "vm/core/CodeGen/MachineDominators.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/MachineSSAContext.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;

template <>
bool toolchain::GenericUniformityAnalysisImpl<MachineSSAContext>::hasDivergentDefs(
    const MachineInstr &I) const {
  for (auto &op : I.all_defs()) {
    if (isDivergent(op.getReg()))
      return true;
  }
  return false;
}

template <>
bool toolchain::GenericUniformityAnalysisImpl<MachineSSAContext>::markDefsDivergent(
    const MachineInstr &Instr) {
  bool insertedDivergent = false;
  const auto &MRI = F.getRegInfo();
  const auto &RBI = *F.getSubtarget().getRegBankInfo();
  const auto &TRI = *MRI.getTargetRegisterInfo();
  for (auto &op : Instr.all_defs()) {
    if (!op.getReg().isVirtual())
      continue;
    assert(!op.getSubReg());
    if (TRI.isUniformReg(MRI, RBI, op.getReg()))
      continue;
    insertedDivergent |= markDivergent(op.getReg());
  }
  return insertedDivergent;
}

template <>
void toolchain::GenericUniformityAnalysisImpl<MachineSSAContext>::initialize() {
  const auto &InstrInfo = *F.getSubtarget().getInstrInfo();

  for (const MachineBasicBlock &block : F) {
    for (const MachineInstr &instr : block) {
      auto uniformity = InstrInfo.getInstructionUniformity(instr);

      switch (uniformity) {
      case InstructionUniformity::AlwaysUniform:
        addUniformOverride(instr);
        break;
      case InstructionUniformity::NeverUniform:
        markDivergent(instr);
        break;
      case InstructionUniformity::Default:
        break;
      }
    }
  }
}

template <>
void toolchain::GenericUniformityAnalysisImpl<MachineSSAContext>::pushUsers(
    Register Reg) {
  assert(isDivergent(Reg));
  const auto &RegInfo = F.getRegInfo();
  for (MachineInstr &UserInstr : RegInfo.use_instructions(Reg)) {
    markDivergent(UserInstr);
  }
}

template <>
void toolchain::GenericUniformityAnalysisImpl<MachineSSAContext>::pushUsers(
    const MachineInstr &Instr) {
  assert(!isAlwaysUniform(Instr));
  if (Instr.isTerminator())
    return;
  for (const MachineOperand &op : Instr.all_defs()) {
    auto Reg = op.getReg();
    if (isDivergent(Reg))
      pushUsers(Reg);
  }
}

template <>
bool toolchain::GenericUniformityAnalysisImpl<MachineSSAContext>::usesValueFromCycle(
    const MachineInstr &I, const MachineCycle &DefCycle) const {
  assert(!isAlwaysUniform(I));
  for (auto &Op : I.operands()) {
    if (!Op.isReg() || !Op.readsReg())
      continue;
    auto Reg = Op.getReg();

    // FIXME: Physical registers need to be properly checked instead of always
    // returning true
    if (Reg.isPhysical())
      return true;

    auto *Def = F.getRegInfo().getVRegDef(Reg);
    if (DefCycle.contains(Def->getParent()))
      return true;
  }
  return false;
}

template <>
void toolchain::GenericUniformityAnalysisImpl<MachineSSAContext>::
    propagateTemporalDivergence(const MachineInstr &I,
                                const MachineCycle &DefCycle) {
  const auto &RegInfo = F.getRegInfo();
  for (auto &Op : I.all_defs()) {
    if (!Op.getReg().isVirtual())
      continue;
    auto Reg = Op.getReg();
    for (MachineInstr &UserInstr : RegInfo.use_instructions(Reg)) {
      if (DefCycle.contains(UserInstr.getParent()))
        continue;
      markDivergent(UserInstr);

      recordTemporalDivergence(Reg, &UserInstr, &DefCycle);
    }
  }
}

template <>
bool toolchain::GenericUniformityAnalysisImpl<MachineSSAContext>::isDivergentUse(
    const MachineOperand &U) const {
  if (!U.isReg())
    return false;

  auto Reg = U.getReg();
  if (isDivergent(Reg))
    return true;

  const auto &RegInfo = F.getRegInfo();
  auto *Def = RegInfo.getOneDef(Reg);
  if (!Def)
    return true;

  auto *DefInstr = Def->getParent();
  auto *UseInstr = U.getParent();
  return isTemporalDivergent(*UseInstr->getParent(), *DefInstr);
}

// This ensures explicit instantiation of
// GenericUniformityAnalysisImpl::ImplDeleter::operator()
template class toolchain::GenericUniformityInfo<MachineSSAContext>;
template struct toolchain::GenericUniformityAnalysisImplDeleter<
    toolchain::GenericUniformityAnalysisImpl<MachineSSAContext>>;

MachineUniformityInfo toolchain::computeMachineUniformityInfo(
    MachineFunction &F, const MachineCycleInfo &cycleInfo,
    const MachineDominatorTree &domTree, bool HasBranchDivergence) {
  assert(F.getRegInfo().isSSA() && "Expected to be run on SSA form!");
  MachineUniformityInfo UI(domTree, cycleInfo);
  if (HasBranchDivergence)
    UI.compute();
  return UI;
}

namespace {

class MachineUniformityInfoPrinterPass : public MachineFunctionPass {
public:
  static char ID;

  MachineUniformityInfoPrinterPass();

  bool runOnMachineFunction(MachineFunction &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

} // namespace

AnalysisKey MachineUniformityAnalysis::Key;

MachineUniformityAnalysis::Result
MachineUniformityAnalysis::run(MachineFunction &MF,
                               MachineFunctionAnalysisManager &MFAM) {
  auto &DomTree = MFAM.getResult<MachineDominatorTreeAnalysis>(MF);
  auto &CI = MFAM.getResult<MachineCycleAnalysis>(MF);
  auto &FAM = MFAM.getResult<FunctionAnalysisManagerMachineFunctionProxy>(MF)
                  .getManager();
  auto &F = MF.getFunction();
  auto &TTI = FAM.getResult<TargetIRAnalysis>(F);
  return computeMachineUniformityInfo(MF, CI, DomTree,
                                      TTI.hasBranchDivergence(&F));
}

PreservedAnalyses
MachineUniformityPrinterPass::run(MachineFunction &MF,
                                  MachineFunctionAnalysisManager &MFAM) {
  auto &MUI = MFAM.getResult<MachineUniformityAnalysis>(MF);
  OS << "MachineUniformityInfo for function: ";
  MF.getFunction().printAsOperand(OS, /*PrintType=*/false);
  OS << '\n';
  MUI.print(OS);
  return PreservedAnalyses::all();
}

char MachineUniformityAnalysisPass::ID = 0;

MachineUniformityAnalysisPass::MachineUniformityAnalysisPass()
    : MachineFunctionPass(ID) {
  initializeMachineUniformityAnalysisPassPass(*PassRegistry::getPassRegistry());
}

INITIALIZE_PASS_BEGIN(MachineUniformityAnalysisPass, "machine-uniformity",
                      "Machine Uniformity Info Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(MachineCycleInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTreeWrapperPass)
INITIALIZE_PASS_END(MachineUniformityAnalysisPass, "machine-uniformity",
                    "Machine Uniformity Info Analysis", false, true)

void MachineUniformityAnalysisPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<MachineCycleInfoWrapperPass>();
  AU.addRequired<MachineDominatorTreeWrapperPass>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool MachineUniformityAnalysisPass::runOnMachineFunction(MachineFunction &MF) {
  auto &DomTree = getAnalysis<MachineDominatorTreeWrapperPass>().getDomTree();
  auto &CI = getAnalysis<MachineCycleInfoWrapperPass>().getCycleInfo();
  // FIXME: Query TTI::hasBranchDivergence. -run-pass seems to end up with a
  // default NoTTI
  UI = computeMachineUniformityInfo(MF, CI, DomTree, true);
  return false;
}

void MachineUniformityAnalysisPass::print(raw_ostream &OS,
                                          const Module *) const {
  OS << "MachineUniformityInfo for function: ";
  UI.getFunction().getFunction().printAsOperand(OS, /*PrintType=*/false);
  OS << '\n';
  UI.print(OS);
}

char MachineUniformityInfoPrinterPass::ID = 0;

MachineUniformityInfoPrinterPass::MachineUniformityInfoPrinterPass()
    : MachineFunctionPass(ID) {
  initializeMachineUniformityInfoPrinterPassPass(
      *PassRegistry::getPassRegistry());
}

INITIALIZE_PASS_BEGIN(MachineUniformityInfoPrinterPass,
                      "print-machine-uniformity",
                      "Print Machine Uniformity Info Analysis", true, true)
INITIALIZE_PASS_DEPENDENCY(MachineUniformityAnalysisPass)
INITIALIZE_PASS_END(MachineUniformityInfoPrinterPass,
                    "print-machine-uniformity",
                    "Print Machine Uniformity Info Analysis", true, true)

void MachineUniformityInfoPrinterPass::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<MachineUniformityAnalysisPass>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool MachineUniformityInfoPrinterPass::runOnMachineFunction(
    MachineFunction &F) {
  auto &UI = getAnalysis<MachineUniformityAnalysisPass>();
  UI.print(errs());
  return false;
}
