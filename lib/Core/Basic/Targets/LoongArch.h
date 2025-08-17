//===-- LoongArch.h - Declare LoongArch target feature support --*- C++ -*-===//
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
// This file declares LoongArch TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_LOONGARCH_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_LOONGARCH_H

#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/TargetParser/Triple.h"

namespace language::Core {
namespace targets {

class LLVM_LIBRARY_VISIBILITY LoongArchTargetInfo : public TargetInfo {
protected:
  std::string ABI;
  std::string CPU;
  bool HasFeatureD;
  bool HasFeatureF;
  bool HasFeatureLSX;
  bool HasFeatureLASX;
  bool HasFeatureFrecipe;
  bool HasFeatureLAM_BH;
  bool HasFeatureLAMCAS;
  bool HasFeatureLD_SEQ_SA;
  bool HasFeatureDiv32;
  bool HasFeatureSCQ;

public:
  LoongArchTargetInfo(const toolchain::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    HasFeatureD = false;
    HasFeatureF = false;
    HasFeatureLSX = false;
    HasFeatureLASX = false;
    HasFeatureFrecipe = false;
    HasFeatureLAM_BH = false;
    HasFeatureLAMCAS = false;
    HasFeatureLD_SEQ_SA = false;
    HasFeatureDiv32 = false;
    HasFeatureSCQ = false;
    BFloat16Width = 16;
    BFloat16Align = 16;
    BFloat16Format = &toolchain::APFloat::BFloat();
    LongDoubleWidth = 128;
    LongDoubleAlign = 128;
    LongDoubleFormat = &toolchain::APFloat::IEEEquad();
    MCountName = "_mcount";
    HasFloat16 = true;
    SuitableAlign = 128;
    WCharType = SignedInt;
    WIntType = UnsignedInt;
    BitIntMaxAlign = 128;
  }

  bool setCPU(const std::string &Name) override {
    if (!isValidCPUName(Name))
      return false;
    CPU = Name;
    return true;
  }

  StringRef getCPU() const { return CPU; }

  StringRef getABI() const override { return ABI; }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override;

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  std::string_view getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override;

  int getEHDataRegisterNumber(unsigned RegNo) const override {
    if (RegNo == 0)
      return 4;
    if (RegNo == 1)
      return 5;
    return -1;
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override;

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override;
  std::string convertConstraint(const char *&Constraint) const override;

  bool hasBitIntType() const override { return true; }

  bool hasBFloat16Type() const override { return true; }

  bool useFP16ConversionIntrinsics() const override { return false; }

  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) override;

  ParsedTargetAttr parseTargetAttr(StringRef Str) const override;
  bool supportsTargetAttributeTune() const override { return true; }

  bool
  initFeatureMap(toolchain::StringMap<bool> &Features, DiagnosticsEngine &Diags,
                 StringRef CPU,
                 const std::vector<std::string> &FeaturesVec) const override;

  bool hasFeature(StringRef Feature) const override;

  bool isValidCPUName(StringRef Name) const override;
  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;
  bool isValidFeatureName(StringRef Name) const override;
};

class LLVM_LIBRARY_VISIBILITY LoongArch32TargetInfo
    : public LoongArchTargetInfo {
public:
  LoongArch32TargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts)
      : LoongArchTargetInfo(Triple, Opts) {
    IntPtrType = SignedInt;
    PtrDiffType = SignedInt;
    SizeType = UnsignedInt;
    resetDataLayout("e-m:e-p:32:32-i64:64-n32-S128");
    // TODO: select appropriate ABI.
    setABI("ilp32d");
  }

  bool setABI(const std::string &Name) override {
    if (Name == "ilp32d" || Name == "ilp32f" || Name == "ilp32s") {
      ABI = Name;
      return true;
    }
    return false;
  }
  void setMaxAtomicWidth() override {
    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 32;
  }
};

class LLVM_LIBRARY_VISIBILITY LoongArch64TargetInfo
    : public LoongArchTargetInfo {
public:
  LoongArch64TargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts)
      : LoongArchTargetInfo(Triple, Opts) {
    LongWidth = LongAlign = PointerWidth = PointerAlign = 64;
    IntMaxType = Int64Type = SignedLong;
    HasUnalignedAccess = true;
    resetDataLayout("e-m:e-p:64:64-i64:64-i128:128-n32:64-S128");
    // TODO: select appropriate ABI.
    setABI("lp64d");
  }

  bool setABI(const std::string &Name) override {
    if (Name == "lp64d" || Name == "lp64f" || Name == "lp64s") {
      ABI = Name;
      return true;
    }
    return false;
  }
  void setMaxAtomicWidth() override {
    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 64;
  }
};
} // end namespace targets
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_BASIC_TARGETS_LOONGARCH_H
