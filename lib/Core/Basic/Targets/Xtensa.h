//===--- Xtensa.h - Declare Xtensa target feature support -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
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
// This file declares Xtensa TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_XTENSA_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_XTENSA_H

#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/TargetParser/Triple.h"

#include "language/Core/Basic/Builtins.h"
#include "language/Core/Basic/MacroBuilder.h"
#include "language/Core/Basic/TargetBuiltins.h"

namespace language::Core {
namespace targets {

class LLVM_LIBRARY_VISIBILITY XtensaTargetInfo : public TargetInfo {
  static const Builtin::Info BuiltinInfo[];

protected:
  std::string CPU;

public:
  XtensaTargetInfo(const toolchain::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    // no big-endianess support yet
    BigEndian = false;
    NoAsmVariants = true;
    LongLongAlign = 64;
    SuitableAlign = 32;
    DoubleAlign = LongDoubleAlign = 64;
    SizeType = UnsignedInt;
    PtrDiffType = SignedInt;
    IntPtrType = SignedInt;
    WCharType = SignedInt;
    WIntType = UnsignedInt;
    UseZeroLengthBitfieldAlignment = true;
    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 32;
    resetDataLayout("e-m:e-p:32:32-i8:8:32-i16:16:32-i64:64-n32");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override {
    return {};
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::XtensaABIBuiltinVaList;
  }

  std::string_view getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override {
    static const char *const GCCRegNames[] = {
        // General register name
        "a0", "sp", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10",
        "a11", "a12", "a13", "a14", "a15",
        // Special register name
        "sar"};
    return toolchain::ArrayRef(GCCRegNames);
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return {};
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    switch (*Name) {
    default:
      return false;
    case 'a':
      Info.setAllowsRegister();
      return true;
    }
    return false;
  }

  int getEHDataRegisterNumber(unsigned RegNo) const override {
    return (RegNo < 2) ? RegNo : -1;
  }

  bool isValidCPUName(StringRef Name) const override {
    return toolchain::StringSwitch<bool>(Name).Case("generic", true).Default(false);
  }

  bool setCPU(const std::string &Name) override {
    CPU = Name;
    return isValidCPUName(Name);
  }
};

} // namespace targets
} // namespace language::Core
#endif // LANGUAGE_CORE_LIB_BASIC_TARGETS_XTENSA_H
