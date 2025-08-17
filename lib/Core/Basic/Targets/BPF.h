//===--- BPF.h - Declare BPF target feature support -------------*- C++ -*-===//
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
// This file declares BPF TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_BPF_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_BPF_H

#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/TargetParser/Triple.h"

namespace language::Core {
namespace targets {

class LLVM_LIBRARY_VISIBILITY BPFTargetInfo : public TargetInfo {
  bool HasAlu32 = false;

public:
  BPFTargetInfo(const toolchain::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    LongWidth = LongAlign = PointerWidth = PointerAlign = 64;
    SizeType = UnsignedLong;
    PtrDiffType = SignedLong;
    IntPtrType = SignedLong;
    IntMaxType = SignedLong;
    Int64Type = SignedLong;
    RegParmMax = 5;
    if (Triple.getArch() == toolchain::Triple::bpfeb) {
      resetDataLayout("E-m:e-p:64:64-i64:64-i128:128-n32:64-S128");
    } else {
      resetDataLayout("e-m:e-p:64:64-i64:64-i128:128-n32:64-S128");
    }
    MaxAtomicPromoteWidth = 64;
    MaxAtomicInlineWidth = 64;
    TLSSupported = false;
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  bool hasFeature(StringRef Feature) const override {
    return Feature == "bpf" || Feature == "alu32" || Feature == "dwarfris";
  }

  void setFeatureEnabled(toolchain::StringMap<bool> &Features, StringRef Name,
                         bool Enabled) const override {
    Features[Name] = Enabled;
  }
  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) override;

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override;

  std::string_view getClobbers() const override { return ""; }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  bool isValidGCCRegisterName(StringRef Name) const override { return true; }
  ArrayRef<const char *> getGCCRegNames() const override { return {}; }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    switch (*Name) {
    default:
      break;
    case 'w':
      if (HasAlu32) {
        Info.setAllowsRegister();
      }
      break;
    }
    return true;
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return {};
  }

  bool allowDebugInfoForExternalRef() const override { return true; }

  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override {
    switch (CC) {
    default:
      return CCCR_Warning;
    case CC_C:
    case CC_DeviceKernel:
      return CCCR_OK;
    }
  }

  bool isValidCPUName(StringRef Name) const override;

  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;

  bool setCPU(const std::string &Name) override {
    if (Name == "v3" || Name == "v4") {
      HasAlu32 = true;
    }

    StringRef CPUName(Name);
    return isValidCPUName(CPUName);
  }

  std::pair<unsigned, unsigned> hardwareInterferenceSizes() const override {
    return std::make_pair(32, 32);
  }
};
} // namespace targets
} // namespace language::Core
#endif // LANGUAGE_CORE_LIB_BASIC_TARGETS_BPF_H
