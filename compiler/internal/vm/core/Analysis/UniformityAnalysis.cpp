//===- UniformityAnalysis.cpp ---------------------------------------------===//
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

#include "vm/core/Analysis/UniformityAnalysis.h"
#include "vm/core/ADT/GenericUniformityImpl.h"
#include "vm/core/Analysis/CycleAnalysis.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;

template <>
bool toolchain::GenericUniformityAnalysisImpl<SSAContext>::hasDivergentDefs(
    const Instruction &I) const {
  return isDivergent((const Value *)&I);
}

template <>
bool toolchain::GenericUniformityAnalysisImpl<SSAContext>::markDefsDivergent(
    const Instruction &Instr) {
  return markDivergent(cast<Value>(&Instr));
}

template <> void toolchain::GenericUniformityAnalysisImpl<SSAContext>::initialize() {
  for (auto &I : instructions(F)) {
    InstructionUniformity IU = TTI->getInstructionUniformity(&I);
    switch (IU) {
    case InstructionUniformity::AlwaysUniform:
      addUniformOverride(I);
      continue;
    case InstructionUniformity::NeverUniform:
      markDivergent(I);
      continue;
    case InstructionUniformity::Default:
      break;
    }
  }
  for (auto &Arg : F.args()) {
    if (TTI->getInstructionUniformity(&Arg) ==
        InstructionUniformity::NeverUniform)
      markDivergent(&Arg);
  }
}

template <>
void toolchain::GenericUniformityAnalysisImpl<SSAContext>::pushUsers(
    const Value *V) {
  for (const auto *User : V->users()) {
    if (const auto *UserInstr = dyn_cast<const Instruction>(User)) {
      markDivergent(*UserInstr);
    }
  }
}

template <>
void toolchain::GenericUniformityAnalysisImpl<SSAContext>::pushUsers(
    const Instruction &Instr) {
  assert(!isAlwaysUniform(Instr));
  if (Instr.isTerminator())
    return;
  pushUsers(cast<Value>(&Instr));
}

template <>
bool toolchain::GenericUniformityAnalysisImpl<SSAContext>::usesValueFromCycle(
    const Instruction &I, const Cycle &DefCycle) const {
  assert(!isAlwaysUniform(I));
  for (const Use &U : I.operands()) {
    if (auto *I = dyn_cast<Instruction>(&U)) {
      if (DefCycle.contains(I->getParent()))
        return true;
    }
  }
  return false;
}

template <>
void toolchain::GenericUniformityAnalysisImpl<
    SSAContext>::propagateTemporalDivergence(const Instruction &I,
                                             const Cycle &DefCycle) {
  for (auto *User : I.users()) {
    auto *UserInstr = cast<Instruction>(User);
    if (DefCycle.contains(UserInstr->getParent()))
      continue;
    markDivergent(*UserInstr);
    recordTemporalDivergence(&I, UserInstr, &DefCycle);
  }
}

template <>
bool toolchain::GenericUniformityAnalysisImpl<SSAContext>::isDivergentUse(
    const Use &U) const {
  const auto *V = U.get();
  if (isDivergent(V))
    return true;
  if (const auto *DefInstr = dyn_cast<Instruction>(V)) {
    const auto *UseInstr = cast<Instruction>(U.getUser());
    return isTemporalDivergent(*UseInstr->getParent(), *DefInstr);
  }
  return false;
}

// This ensures explicit instantiation of
// GenericUniformityAnalysisImpl::ImplDeleter::operator()
template class toolchain::GenericUniformityInfo<SSAContext>;
template struct toolchain::GenericUniformityAnalysisImplDeleter<
    toolchain::GenericUniformityAnalysisImpl<SSAContext>>;

//===----------------------------------------------------------------------===//
//  UniformityInfoAnalysis and related pass implementations
//===----------------------------------------------------------------------===//

toolchain::UniformityInfo UniformityInfoAnalysis::run(Function &F,
                                                 FunctionAnalysisManager &FAM) {
  auto &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  auto &TTI = FAM.getResult<TargetIRAnalysis>(F);
  auto &CI = FAM.getResult<CycleAnalysis>(F);
  UniformityInfo UI{DT, CI, &TTI};
  // Skip computation if we can assume everything is uniform.
  if (TTI.hasBranchDivergence(&F))
    UI.compute();

  return UI;
}

AnalysisKey UniformityInfoAnalysis::Key;

UniformityInfoPrinterPass::UniformityInfoPrinterPass(raw_ostream &OS)
    : OS(OS) {}

PreservedAnalyses UniformityInfoPrinterPass::run(Function &F,
                                                 FunctionAnalysisManager &AM) {
  OS << "UniformityInfo for function '" << F.getName() << "':\n";
  AM.getResult<UniformityInfoAnalysis>(F).print(OS);

  return PreservedAnalyses::all();
}

//===----------------------------------------------------------------------===//
//  UniformityInfoWrapperPass Implementation
//===----------------------------------------------------------------------===//

char UniformityInfoWrapperPass::ID = 0;

UniformityInfoWrapperPass::UniformityInfoWrapperPass() : FunctionPass(ID) {}

INITIALIZE_PASS_BEGIN(UniformityInfoWrapperPass, "uniformity",
                      "Uniformity Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(CycleInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(UniformityInfoWrapperPass, "uniformity",
                    "Uniformity Analysis", false, true)

void UniformityInfoWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<CycleInfoWrapperPass>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
}

bool UniformityInfoWrapperPass::runOnFunction(Function &F) {
  auto &cycleInfo = getAnalysis<CycleInfoWrapperPass>().getResult();
  auto &domTree = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  auto &targetTransformInfo =
      getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);

  m_function = &F;
  m_uniformityInfo = UniformityInfo{domTree, cycleInfo, &targetTransformInfo};

  // Skip computation if we can assume everything is uniform.
  if (targetTransformInfo.hasBranchDivergence(m_function))
    m_uniformityInfo.compute();

  return false;
}

void UniformityInfoWrapperPass::print(raw_ostream &OS, const Module *) const {
  OS << "UniformityInfo for function '" << m_function->getName() << "':\n";
  m_uniformityInfo.print(OS);
}

void UniformityInfoWrapperPass::releaseMemory() {
  m_uniformityInfo = UniformityInfo{};
  m_function = nullptr;
}
