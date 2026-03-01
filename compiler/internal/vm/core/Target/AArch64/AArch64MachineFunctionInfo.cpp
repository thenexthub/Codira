//=- AArch64MachineFunctionInfo.cpp - AArch64 Machine Function Info ---------=//

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
///
/// \file
/// This file implements AArch64-specific per-machine-function
/// information.
///
//===----------------------------------------------------------------------===//

#include "AArch64MachineFunctionInfo.h"
#include "AArch64InstrInfo.h"
#include "AArch64Subtarget.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Metadata.h"
#include "vm/core/IR/Module.h"
#include "vm/core/MC/MCAsmInfo.h"

using namespace vm::core;

static std::optional<uint64_t>
getSVEStackSize(const AArch64FunctionInfo &MFI,
                uint64_t (AArch64FunctionInfo::*GetStackSize)() const) {
  if (!MFI.hasCalculatedStackSizeSVE())
    return std::nullopt;
  return (MFI.*GetStackSize)();
}

yaml::AArch64FunctionInfo::AArch64FunctionInfo(
    const toolchain::AArch64FunctionInfo &MFI)
    : HasRedZone(MFI.hasRedZone()),
      StackSizeZPR(
          getSVEStackSize(MFI, &toolchain::AArch64FunctionInfo::getStackSizeZPR)),
      StackSizePPR(
          getSVEStackSize(MFI, &toolchain::AArch64FunctionInfo::getStackSizePPR)),
      HasStackFrame(MFI.hasStackFrame()
                        ? std::optional<bool>(MFI.hasStackFrame())
                        : std::nullopt),
      HasStreamingModeChanges(
          MFI.hasStreamingModeChanges()
              ? std::optional<bool>(MFI.hasStreamingModeChanges())
              : std::nullopt) {}

void yaml::AArch64FunctionInfo::mappingImpl(yaml::IO &YamlIO) {
  MappingTraits<AArch64FunctionInfo>::mapping(YamlIO, *this);
}

void AArch64FunctionInfo::initializeBaseYamlFields(
    const yaml::AArch64FunctionInfo &YamlMFI) {
  if (YamlMFI.HasRedZone)
    HasRedZone = YamlMFI.HasRedZone;
  if (YamlMFI.StackSizeZPR || YamlMFI.StackSizePPR)
    setStackSizeSVE(YamlMFI.StackSizeZPR.value_or(0),
                    YamlMFI.StackSizePPR.value_or(0));
  if (YamlMFI.HasStackFrame)
    setHasStackFrame(*YamlMFI.HasStackFrame);
  if (YamlMFI.HasStreamingModeChanges)
    setHasStreamingModeChanges(*YamlMFI.HasStreamingModeChanges);
}

static SignReturnAddress GetSignReturnAddress(const Function &F) {
  if (F.hasFnAttribute("ptrauth-returns"))
    return SignReturnAddress::NonLeaf;

  // The function should be signed in the following situations:
  // - sign-return-address=all
  // - sign-return-address=non-leaf and the functions spills the LR
  if (!F.hasFnAttribute("sign-return-address"))
    return SignReturnAddress::None;

  StringRef Scope = F.getFnAttribute("sign-return-address").getValueAsString();
  return StringSwitch<SignReturnAddress>(Scope)
      .Case("none", SignReturnAddress::None)
      .Case("non-leaf", SignReturnAddress::NonLeaf)
      .Case("all", SignReturnAddress::All);
}

static bool ShouldSignWithBKey(const Function &F, const AArch64Subtarget &STI) {
  if (F.hasFnAttribute("ptrauth-returns"))
    return true;
  if (!F.hasFnAttribute("sign-return-address-key")) {
    if (STI.getTargetTriple().isOSWindows())
      return true;
    return false;
  }

  const StringRef Key =
      F.getFnAttribute("sign-return-address-key").getValueAsString();
  assert(Key == "a_key" || Key == "b_key");
  return Key == "b_key";
}

static bool hasELFSignedGOTHelper(const Function &F,
                                  const AArch64Subtarget *STI) {
  if (!STI->getTargetTriple().isOSBinFormatELF())
    return false;
  const Module *M = F.getParent();
  const auto *Flag = mdconst::extract_or_null<ConstantInt>(
      M->getModuleFlag("ptrauth-elf-got"));
  if (Flag && Flag->getZExtValue() == 1)
    return true;
  return false;
}

AArch64FunctionInfo::AArch64FunctionInfo(const Function &F,
                                         const AArch64Subtarget *STI) {
  // If we already know that the function doesn't have a redzone, set
  // HasRedZone here.
  if (F.hasFnAttribute(Attribute::NoRedZone))
    HasRedZone = false;
  SignCondition = GetSignReturnAddress(F);
  SignWithBKey = ShouldSignWithBKey(F, *STI);
  HasELFSignedGOT = hasELFSignedGOTHelper(F, STI);
  // TODO: skip functions that have no instrumented allocas for optimization
  IsMTETagged = F.hasFnAttribute(Attribute::SanitizeMemTag);

  // BTI/PAuthLR are set on the function attribute.
  BranchTargetEnforcement = F.hasFnAttribute("branch-target-enforcement");
  BranchProtectionPAuthLR = F.hasFnAttribute("branch-protection-pauth-lr");

  // Parse the SME function attributes.
  SMEFnAttrs = SMEAttrs(F);

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

  if (STI->isTargetWindows()) {
    if (!F.hasFnAttribute("no-stack-arg-probe"))
      StackProbeSize = ProbeSize;
  } else {
    // Round down to the stack alignment.
    uint64_t StackAlign =
        STI->getFrameLowering()->getTransientStackAlign().value();
    ProbeSize = std::max(StackAlign, ProbeSize & ~(StackAlign - 1U));
    StringRef ProbeKind;
    if (F.hasFnAttribute("probe-stack"))
      ProbeKind = F.getFnAttribute("probe-stack").getValueAsString();
    else if (const auto *PS = dyn_cast_or_null<MDString>(
                 F.getParent()->getModuleFlag("probe-stack")))
      ProbeKind = PS->getString();
    if (ProbeKind.size()) {
      if (ProbeKind != "inline-asm")
        report_fatal_error("Unsupported stack probing method");
      StackProbeSize = ProbeSize;
    }
  }
}

MachineFunctionInfo *AArch64FunctionInfo::clone(
    BumpPtrAllocator &Allocator, MachineFunction &DestMF,
    const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
    const {
  return DestMF.cloneInfo<AArch64FunctionInfo>(*this);
}

static bool isLRSpilled(const MachineFunction &MF) {
  return toolchain::any_of(
      MF.getFrameInfo().getCalleeSavedInfo(),
      [](const auto &Info) { return Info.getReg() == AArch64::LR; });
}

bool AArch64FunctionInfo::shouldSignReturnAddress(SignReturnAddress Condition,
                                                  bool IsLRSpilled) {
  switch (Condition) {
  case SignReturnAddress::None:
    return false;
  case SignReturnAddress::NonLeaf:
    return IsLRSpilled;
  case SignReturnAddress::All:
    return true;
  }
  llvm_unreachable("Unknown SignReturnAddress enum");
}

bool AArch64FunctionInfo::shouldSignReturnAddress(
    const MachineFunction &MF) const {
  return shouldSignReturnAddress(SignCondition, isLRSpilled(MF));
}

bool AArch64FunctionInfo::needsShadowCallStackPrologueEpilogue(
    MachineFunction &MF) const {
  if (!(isLRSpilled(MF) &&
        MF.getFunction().hasFnAttribute(Attribute::ShadowCallStack)))
    return false;

  if (!MF.getSubtarget<AArch64Subtarget>().isXRegisterReserved(18))
    report_fatal_error("Must reserve x18 to use shadow call stack");

  return true;
}

bool AArch64FunctionInfo::needsDwarfUnwindInfo(
    const MachineFunction &MF) const {
  if (!NeedsDwarfUnwindInfo)
    NeedsDwarfUnwindInfo = MF.needsFrameMoves() &&
                           !MF.getTarget().getMCAsmInfo()->usesWindowsCFI();

  return *NeedsDwarfUnwindInfo;
}

bool AArch64FunctionInfo::needsAsyncDwarfUnwindInfo(
    const MachineFunction &MF) const {
  if (!NeedsAsyncDwarfUnwindInfo) {
    const Function &F = MF.getFunction();
    const AArch64FunctionInfo *AFI = MF.getInfo<AArch64FunctionInfo>();
    //  The check got "minsize" is because epilogue unwind info is not emitted
    //  (yet) for homogeneous epilogues, outlined functions, and functions
    //  outlined from.
    NeedsAsyncDwarfUnwindInfo =
        needsDwarfUnwindInfo(MF) &&
        ((F.getUWTableKind() == UWTableKind::Async && !F.hasMinSize()) ||
         AFI->hasStreamingModeChanges());
  }
  return *NeedsAsyncDwarfUnwindInfo;
}
