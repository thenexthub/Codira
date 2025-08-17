//===--- RISCV.h - Declare RISC-V target feature support --------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
// This file declares RISC-V TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_RISCV_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_RISCV_H

#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/TargetParser/RISCVISAInfo.h"
#include "toolchain/TargetParser/Triple.h"
#include <optional>

namespace language::Core {
namespace targets {

// RISC-V Target
class RISCVTargetInfo : public TargetInfo {
protected:
  std::string ABI, CPU;
  std::unique_ptr<toolchain::RISCVISAInfo> ISAInfo;

private:
  bool FastScalarUnalignedAccess;
  bool HasExperimental = false;

public:
  RISCVTargetInfo(const toolchain::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    BFloat16Width = 16;
    BFloat16Align = 16;
    BFloat16Format = &toolchain::APFloat::BFloat();
    LongDoubleWidth = 128;
    LongDoubleAlign = 128;
    LongDoubleFormat = &toolchain::APFloat::IEEEquad();
    SuitableAlign = 128;
    WCharType = SignedInt;
    WIntType = UnsignedInt;
    HasRISCVVTypes = true;
    MCountName = "_mcount";
    HasFloat16 = true;
    HasStrictFP = true;
  }

  bool setCPU(const std::string &Name) override {
    if (!isValidCPUName(Name))
      return false;
    CPU = Name;
    return true;
  }

  StringRef getABI() const override { return ABI; }
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override;

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  std::string_view getClobbers() const override { return ""; }

  StringRef getConstraintRegister(StringRef Constraint,
                                  StringRef Expression) const override {
    return Expression;
  }

  ArrayRef<const char *> getGCCRegNames() const override;

  int getEHDataRegisterNumber(unsigned RegNo) const override {
    if (RegNo == 0)
      return 10;
    else if (RegNo == 1)
      return 11;
    else
      return -1;
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override;

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override;

  std::string convertConstraint(const char *&Constraint) const override;

  bool
  initFeatureMap(toolchain::StringMap<bool> &Features, DiagnosticsEngine &Diags,
                 StringRef CPU,
                 const std::vector<std::string> &FeaturesVec) const override;

  std::optional<std::pair<unsigned, unsigned>>
  getVScaleRange(const LangOptions &LangOpts, ArmStreamingKind Mode,
                 toolchain::StringMap<bool> *FeatureMap = nullptr) const override;

  bool hasFeature(StringRef Feature) const override;

  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) override;

  bool hasBitIntType() const override { return true; }

  bool hasBFloat16Type() const override { return true; }

  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override;

  bool useFP16ConversionIntrinsics() const override {
    return false;
  }

  bool isValidCPUName(StringRef Name) const override;
  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;
  bool isValidTuneCPUName(StringRef Name) const override;
  void fillValidTuneCPUList(SmallVectorImpl<StringRef> &Values) const override;
  bool supportsTargetAttributeTune() const override { return true; }
  ParsedTargetAttr parseTargetAttr(StringRef Str) const override;
  toolchain::APInt getFMVPriority(ArrayRef<StringRef> Features) const override;

  std::pair<unsigned, unsigned> hardwareInterferenceSizes() const override {
    return std::make_pair(32, 32);
  }

  bool supportsCpuSupports() const override { return getTriple().isOSLinux(); }
  bool supportsCpuIs() const override { return getTriple().isOSLinux(); }
  bool supportsCpuInit() const override { return getTriple().isOSLinux(); }
  bool validateCpuSupports(StringRef Feature) const override;
  bool validateCpuIs(StringRef CPUName) const override;
  bool isValidFeatureName(StringRef Name) const override;

  bool validateGlobalRegisterVariable(StringRef RegName, unsigned RegSize,
                                      bool &HasSizeMismatch) const override;

  bool checkCFProtectionBranchSupported(DiagnosticsEngine &) const override {
    // Always generate Zicfilp lpad insns
    // Non-zicfilp CPUs would read them as NOP
    return true;
  }

  bool
  checkCFProtectionReturnSupported(DiagnosticsEngine &Diags) const override {
    if (ISAInfo->hasExtension("zicfiss"))
      return true;
    return TargetInfo::checkCFProtectionReturnSupported(Diags);
  }

  CFBranchLabelSchemeKind getDefaultCFBranchLabelScheme() const override {
    return CFBranchLabelSchemeKind::FuncSig;
  }

  bool
  checkCFBranchLabelSchemeSupported(const CFBranchLabelSchemeKind Scheme,
                                    DiagnosticsEngine &Diags) const override {
    switch (Scheme) {
    case CFBranchLabelSchemeKind::Default:
    case CFBranchLabelSchemeKind::Unlabeled:
    case CFBranchLabelSchemeKind::FuncSig:
      return true;
    }
    return TargetInfo::checkCFBranchLabelSchemeSupported(Scheme, Diags);
  }
};
class LLVM_LIBRARY_VISIBILITY RISCV32TargetInfo : public RISCVTargetInfo {
public:
  RISCV32TargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts)
      : RISCVTargetInfo(Triple, Opts) {
    IntPtrType = SignedInt;
    PtrDiffType = SignedInt;
    SizeType = UnsignedInt;
    resetDataLayout("e-m:e-p:32:32-i64:64-n32-S128");
  }

  bool setABI(const std::string &Name) override {
    if (Name == "ilp32e") {
      ABI = Name;
      resetDataLayout("e-m:e-p:32:32-i64:64-n32-S32");
      return true;
    }

    if (Name == "ilp32" || Name == "ilp32f" || Name == "ilp32d") {
      ABI = Name;
      return true;
    }
    return false;
  }

  void setMaxAtomicWidth() override {
    MaxAtomicPromoteWidth = 128;

    if (ISAInfo->hasExtension("a"))
      MaxAtomicInlineWidth = 32;
  }
};
class LLVM_LIBRARY_VISIBILITY RISCV64TargetInfo : public RISCVTargetInfo {
public:
  RISCV64TargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts)
      : RISCVTargetInfo(Triple, Opts) {
    LongWidth = LongAlign = PointerWidth = PointerAlign = 64;
    IntMaxType = Int64Type = SignedLong;
    resetDataLayout("e-m:e-p:64:64-i64:64-i128:128-n32:64-S128");
  }

  bool setABI(const std::string &Name) override {
    if (Name == "lp64e") {
      ABI = Name;
      resetDataLayout("e-m:e-p:64:64-i64:64-i128:128-n32:64-S64");
      return true;
    }

    if (Name == "lp64" || Name == "lp64f" || Name == "lp64d") {
      ABI = Name;
      return true;
    }
    return false;
  }

  void setMaxAtomicWidth() override {
    MaxAtomicPromoteWidth = 128;

    if (ISAInfo->hasExtension("a"))
      MaxAtomicInlineWidth = 64;
  }
};
} // namespace targets
} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_BASIC_TARGETS_RISCV_H
