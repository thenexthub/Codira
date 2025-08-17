//===--- ARC.h - Declare ARC target feature support -------------*- C++ -*-===//
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
// This file declares ARC TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_ARC_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_ARC_H

#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/TargetParser/Triple.h"

namespace language::Core {
namespace targets {

class LLVM_LIBRARY_VISIBILITY ARCTargetInfo : public TargetInfo {
public:
  ARCTargetInfo(const toolchain::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    NoAsmVariants = true;
    LongLongAlign = 32;
    SuitableAlign = 32;
    DoubleAlign = LongDoubleAlign = 32;
    SizeType = UnsignedInt;
    PtrDiffType = SignedInt;
    IntPtrType = SignedInt;
    UseZeroLengthBitfieldAlignment = true;
    resetDataLayout("e-m:e-p:32:32-i1:8:32-i8:8:32-i16:16:32-"
                    "i32:32:32-f32:32:32-i64:32-f64:32-a:0:32-n32");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override {
    return {};
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  std::string_view getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override {
    static const char *const GCCRegNames[] = {
        "r0",  "r1",  "r2",  "r3",  "r4",  "r5",     "r6",  "r7",
        "r8",  "r9",  "r10", "r11", "r12", "r13",    "r14", "r15",
        "r16", "r17", "r18", "r19", "r20", "r21",    "r22", "r23",
        "r24", "r25", "gp",  "sp",  "fp",  "ilink1", "r30", "blink"};
    return toolchain::ArrayRef(GCCRegNames);
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return {};
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    return false;
  }

  bool hasBitIntType() const override { return true; }

  bool isCLZForZeroUndef() const override { return false; }
};

} // namespace targets
} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_BASIC_TARGETS_ARC_H
