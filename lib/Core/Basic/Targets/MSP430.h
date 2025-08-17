//===--- MSP430.h - Declare MSP430 target feature support -------*- C++ -*-===//
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
// This file declares MSP430 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_MSP430_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_MSP430_H

#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/TargetParser/Triple.h"

namespace language::Core {
namespace targets {

class LLVM_LIBRARY_VISIBILITY MSP430TargetInfo : public TargetInfo {
  static const char *const GCCRegNames[];

public:
  MSP430TargetInfo(const toolchain::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    TLSSupported = false;
    IntWidth = 16;
    IntAlign = 16;
    LongWidth = 32;
    LongLongWidth = 64;
    LongAlign = LongLongAlign = 16;
    FloatWidth = 32;
    FloatAlign = 16;
    DoubleWidth = LongDoubleWidth = 64;
    DoubleAlign = LongDoubleAlign = 16;
    PointerWidth = 16;
    PointerAlign = 16;
    SuitableAlign = 16;
    SizeType = UnsignedInt;
    IntMaxType = SignedLongLong;
    IntPtrType = SignedInt;
    PtrDiffType = SignedInt;
    SigAtomicType = SignedLong;
    resetDataLayout("e-m:e-p:16:16-i32:16-i64:16-f32:16-f64:16-a:8-n8:16-S16");
  }
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override {
    // FIXME: Implement.
    return {};
  }

  bool allowsLargerPreferedTypeAlignment() const override { return false; }

  bool hasFeature(StringRef Feature) const override {
    return Feature == "msp430";
  }

  ArrayRef<const char *> getGCCRegNames() const override;

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    // Make r0 - r3 be recognized by llc (f.e., in clobber list)
    static const TargetInfo::GCCRegAlias GCCRegAliases[] = {
        {{"r0"}, "pc"},
        {{"r1"}, "sp"},
        {{"r2"}, "sr"},
        {{"r3"}, "cg"},
    };
    return toolchain::ArrayRef(GCCRegAliases);
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override {
    // FIXME: implement
    switch (*Name) {
    case 'K': // the constant 1
    case 'L': // constant -1^20 .. 1^19
    case 'M': // constant 1-4:
      return true;
    }
    // No target constraints for now.
    return false;
  }

  std::string_view getClobbers() const override {
    // FIXME: Is this really right?
    return "";
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    // FIXME: implement
    return TargetInfo::CharPtrBuiltinVaList;
  }
};

} // namespace targets
} // namespace language::Core
#endif // LANGUAGE_CORE_LIB_BASIC_TARGETS_MSP430_H
