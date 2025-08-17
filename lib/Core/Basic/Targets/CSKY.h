//===--- CSKY.h - Declare CSKY target feature support -----------*- C++ -*-===//
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
// This file declares CSKY TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_CSKY_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_CSKY_H

#include "language/Core/Basic/MacroBuilder.h"
#include "language/Core/Basic/TargetInfo.h"
#include "toolchain/TargetParser/CSKYTargetParser.h"

namespace language::Core {
namespace targets {

class LLVM_LIBRARY_VISIBILITY CSKYTargetInfo : public TargetInfo {
protected:
  std::string ABI;
  toolchain::CSKY::ArchKind Arch = toolchain::CSKY::ArchKind::INVALID;
  std::string CPU;

  bool HardFloat = false;
  bool HardFloatABI = false;
  bool FPUV2_SF = false;
  bool FPUV2_DF = false;
  bool FPUV3_SF = false;
  bool FPUV3_DF = false;
  bool VDSPV2 = false;
  bool VDSPV1 = false;
  bool DSPV2 = false;
  bool is3E3R1 = false;

public:
  CSKYTargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts)
      : TargetInfo(Triple) {
    NoAsmVariants = true;
    LongLongAlign = 32;
    SuitableAlign = 32;
    DoubleAlign = LongDoubleAlign = 32;
    SizeType = UnsignedInt;
    PtrDiffType = SignedInt;
    IntPtrType = SignedInt;
    WCharType = SignedInt;
    WIntType = UnsignedInt;

    UseZeroLengthBitfieldAlignment = true;
    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 32;
    resetDataLayout("e-m:e-S32-p:32:32-i32:32:32-i64:32:32-f32:32:32-f64:32:32-"
                    "v64:32:32-v128:32:32-a:0:32-Fi32-n32");

    setABI("abiv2");
  }

  StringRef getABI() const override { return ABI; }
  bool setABI(const std::string &Name) override {
    if (Name == "abiv2" || Name == "abiv1") {
      ABI = Name;
      return true;
    }
    return false;
  }

  bool setCPU(const std::string &Name) override;

  bool isValidCPUName(StringRef Name) const override;

  unsigned getMinGlobalAlign(uint64_t, bool HasNonWeakDef) const override;

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override {
    return {};
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return VoidPtrBuiltinVaList;
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override;

  std::string_view getClobbers() const override { return ""; }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
  bool hasFeature(StringRef Feature) const override;
  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) override;

  /// Whether target allows to overalign ABI-specified preferred alignment
  bool allowsLargerPreferedTypeAlignment() const override { return false; }

  bool hasBitIntType() const override { return true; }

protected:
  ArrayRef<const char *> getGCCRegNames() const override;

  ArrayRef<GCCRegAlias> getGCCRegAliases() const override;
};

} // namespace targets
} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_BASIC_TARGETS_CSKY_H
