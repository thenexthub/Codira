//=- RISCVMachineFunctionInfo.cpp - RISC-V machine function info --*- C++ -*-=//
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
// This file declares RISCV-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#include "RISCVMachineFunctionInfo.h"
#include "vm/core/IR/Module.h"

using namespace vm::core;

yaml::RISCVMachineFunctionInfo::RISCVMachineFunctionInfo(
    const toolchain::RISCVMachineFunctionInfo &MFI)
    : VarArgsFrameIndex(MFI.getVarArgsFrameIndex()),
      VarArgsSaveSize(MFI.getVarArgsSaveSize()) {}

MachineFunctionInfo *RISCVMachineFunctionInfo::clone(
    BumpPtrAllocator &Allocator, MachineFunction &DestMF,
    const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
    const {
  return DestMF.cloneInfo<RISCVMachineFunctionInfo>(*this);
}

RISCVMachineFunctionInfo::RISCVMachineFunctionInfo(const Function &F,
                                                   const RISCVSubtarget *STI) {

  // The default stack probe size is 4096 if the function has no
  // stack-probe-size attribute. This is a safe default because it is the
  // smallest possible guard page size.
  uint64_t ProbeSize = 4096;
  if (F.hasFnAttribute("stack-probe-size"))
    ProbeSize = F.getFnAttributeAsParsedInteger("stack-probe-size");
  else if (const auto *PS = mdconst::extract_or_null<ConstantInt>(
               F.getParent()->getModuleFlag("stack-probe-size")))
    ProbeSize = PS->getZExtValue();
  assert(int64_t(ProbeSize) > 0 && "Invalid stack probe size");

  // Round down to the stack alignment.
  uint64_t StackAlign =
      STI->getFrameLowering()->getTransientStackAlign().value();
  ProbeSize = std::max(StackAlign, alignDown(ProbeSize, StackAlign));
  StringRef ProbeKind;
  if (F.hasFnAttribute("probe-stack"))
    ProbeKind = F.getFnAttribute("probe-stack").getValueAsString();
  else if (const auto *PS = dyn_cast_or_null<MDString>(
               F.getParent()->getModuleFlag("probe-stack")))
    ProbeKind = PS->getString();
  if (ProbeKind.size()) {
    StackProbeSize = ProbeSize;
  }
}

RISCVMachineFunctionInfo::InterruptStackKind
RISCVMachineFunctionInfo::getInterruptStackKind(
    const MachineFunction &MF) const {
  if (!MF.getFunction().hasFnAttribute("interrupt"))
    return InterruptStackKind::None;

  assert(VarArgsSaveSize == 0 &&
         "Interrupt functions should not having incoming varargs");

  StringRef InterruptVal =
      MF.getFunction().getFnAttribute("interrupt").getValueAsString();

  return StringSwitch<RISCVMachineFunctionInfo::InterruptStackKind>(
             InterruptVal)
      .Case("qci-nest", InterruptStackKind::QCINest)
      .Case("qci-nonest", InterruptStackKind::QCINoNest)
      .Case("SiFive-CLIC-preemptible",
            InterruptStackKind::SiFiveCLICPreemptible)
      .Case("SiFive-CLIC-stack-swap", InterruptStackKind::SiFiveCLICStackSwap)
      .Case("SiFive-CLIC-preemptible-stack-swap",
            InterruptStackKind::SiFiveCLICPreemptibleStackSwap)
      .Default(InterruptStackKind::None);
}

void yaml::RISCVMachineFunctionInfo::mappingImpl(yaml::IO &YamlIO) {
  MappingTraits<RISCVMachineFunctionInfo>::mapping(YamlIO, *this);
}

RISCVMachineFunctionInfo::PushPopKind
RISCVMachineFunctionInfo::getPushPopKind(const MachineFunction &MF) const {
  // We cannot use fixed locations for the callee saved spill slots if the
  // function uses a varargs save area.
  // TODO: Use a separate placement for vararg registers to enable Zcmp.
  if (VarArgsSaveSize != 0)
    return PushPopKind::None;

  // SiFive interrupts are not compatible with push/pop.
  if (useSiFiveInterrupt(MF))
    return PushPopKind::None;

  // Zcmp is not compatible with the frame pointer convention.
  if (MF.getSubtarget<RISCVSubtarget>().hasStdExtZcmp() &&
      !MF.getTarget().Options.DisableFramePointerElim(MF))
    return PushPopKind::StdExtZcmp;

  // Xqccmp is Zcmp but has a push order compatible with the frame-pointer
  // convention.
  if (MF.getSubtarget<RISCVSubtarget>().hasVendorXqccmp())
    return PushPopKind::VendorXqccmp;

  return PushPopKind::None;
}

bool RISCVMachineFunctionInfo::hasImplicitFPUpdates(
    const MachineFunction &MF) const {
  switch (getInterruptStackKind(MF)) {
  case InterruptStackKind::QCINest:
  case InterruptStackKind::QCINoNest:
    // QC.C.MIENTER and QC.C.MIENTER.NEST both update FP on function entry.
    return true;
  default:
    break;
  }

  switch (getPushPopKind(MF)) {
  case PushPopKind::VendorXqccmp:
    // When using Xqccmp, we will use `QC.CM.PUSHFP` when Frame Pointers are
    // enabled, which will update FP.
    return true;
  default:
    break;
  }

  return false;
}

void RISCVMachineFunctionInfo::initializeBaseYamlFields(
    const yaml::RISCVMachineFunctionInfo &YamlMFI) {
  VarArgsFrameIndex = YamlMFI.VarArgsFrameIndex;
  VarArgsSaveSize = YamlMFI.VarArgsSaveSize;
}

void RISCVMachineFunctionInfo::addSExt32Register(Register Reg) {
  SExt32Registers.push_back(Reg);
}

bool RISCVMachineFunctionInfo::isSExt32Register(Register Reg) const {
  return is_contained(SExt32Registers, Reg);
}
