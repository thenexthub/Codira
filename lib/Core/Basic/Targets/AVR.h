//===--- AVR.h - Declare AVR target feature support -------------*- C++ -*-===//
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
// This file declares AVR TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_AVR_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_AVR_H

#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/TargetParser/Triple.h"

namespace language::Core {
namespace targets {

// AVR Target
class LLVM_LIBRARY_VISIBILITY AVRTargetInfo : public TargetInfo {
public:
  AVRTargetInfo(const toolchain::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    TLSSupported = false;
    PointerWidth = 16;
    PointerAlign = 8;
    ShortWidth = 16;
    ShortAlign = 8;
    IntWidth = 16;
    IntAlign = 8;
    LongWidth = 32;
    LongAlign = 8;
    LongLongWidth = 64;
    LongLongAlign = 8;
    SuitableAlign = 8;
    DefaultAlignForAttributeAligned = 8;
    HalfWidth = 16;
    HalfAlign = 8;
    FloatWidth = 32;
    FloatAlign = 8;
    DoubleWidth = 32;
    DoubleAlign = 8;
    DoubleFormat = &toolchain::APFloat::IEEEsingle();
    LongDoubleWidth = 32;
    LongDoubleAlign = 8;
    LongDoubleFormat = &toolchain::APFloat::IEEEsingle();
    SizeType = UnsignedInt;
    PtrDiffType = SignedInt;
    IntPtrType = SignedInt;
    Char16Type = UnsignedInt;
    WIntType = SignedInt;
    Int16Type = SignedInt;
    Char32Type = UnsignedLong;
    SigAtomicType = SignedChar;
    resetDataLayout("e-P1-p:16:8-i8:8-i16:8-i32:8-i64:8-f32:8-f64:8-n8:16-a:8");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override {
    return {};
  }

  bool allowsLargerPreferedTypeAlignment() const override { return false; }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  std::string_view getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override {
    static const char *const GCCRegNames[] = {
        "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",  "r8",  "r9",
        "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19",
        "r20", "r21", "r22", "r23", "r24", "r25", "X",   "Y",   "Z",   "SP"};
    return toolchain::ArrayRef(GCCRegNames);
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return {};
  }

  ArrayRef<TargetInfo::AddlRegName> getGCCAddlRegNames() const override {
    static const TargetInfo::AddlRegName AddlRegNames[] = {
        {{"r26", "r27"}, 26},
        {{"r28", "r29"}, 27},
        {{"r30", "r31"}, 28},
        {{"SPL", "SPH"}, 29},
    };
    return toolchain::ArrayRef(AddlRegNames);
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    // There aren't any multi-character AVR specific constraints.
    if (StringRef(Name).size() > 1)
      return false;

    switch (*Name) {
    default:
      return false;
    case 'a': // Simple upper registers
    case 'b': // Base pointer registers pairs
    case 'd': // Upper register
    case 'l': // Lower registers
    case 'e': // Pointer register pairs
    case 'q': // Stack pointer register
    case 'r': // Any register
    case 'w': // Special upper register pairs
    case 't': // Temporary register
    case 'x':
    case 'X': // Pointer register pair X
    case 'y':
    case 'Y': // Pointer register pair Y
    case 'z':
    case 'Z': // Pointer register pair Z
      Info.setAllowsRegister();
      return true;
    case 'I': // 6-bit positive integer constant
      // Due to issue https://github.com/toolchain/toolchain-project/issues/51513, we
      // allow value 64 in the frontend and let it be denied in the backend.
      Info.setRequiresImmediate(0, 64);
      return true;
    case 'J': // 6-bit negative integer constant
      Info.setRequiresImmediate(-63, 0);
      return true;
    case 'K': // Integer constant (Range: 2)
      Info.setRequiresImmediate(2);
      return true;
    case 'L': // Integer constant (Range: 0)
      Info.setRequiresImmediate(0);
      return true;
    case 'M': // 8-bit integer constant
      Info.setRequiresImmediate(0, 0xff);
      return true;
    case 'N': // Integer constant (Range: -1)
      Info.setRequiresImmediate(-1);
      return true;
    case 'O': // Integer constant (Range: 8, 16, 24)
      Info.setRequiresImmediate({8, 16, 24});
      return true;
    case 'P': // Integer constant (Range: 1)
      Info.setRequiresImmediate(1);
      return true;
    case 'R': // Integer constant (Range: -6 to 5)
      Info.setRequiresImmediate(-6, 5);
      return true;
    case 'G': // Floating point constant 0.0
      Info.setRequiresImmediate(0);
      return true;
    case 'Q': // A memory address based on Y or Z pointer with displacement.
      return true;
    }

    return false;
  }

  IntType getIntTypeByWidth(unsigned BitWidth, bool IsSigned) const final {
    // AVR prefers int for 16-bit integers.
    return BitWidth == 16 ? (IsSigned ? SignedInt : UnsignedInt)
                          : TargetInfo::getIntTypeByWidth(BitWidth, IsSigned);
  }

  IntType getLeastIntTypeByWidth(unsigned BitWidth, bool IsSigned) const final {
    // AVR uses int for int_least16_t and int_fast16_t.
    return BitWidth == 16
               ? (IsSigned ? SignedInt : UnsignedInt)
               : TargetInfo::getLeastIntTypeByWidth(BitWidth, IsSigned);
  }

  bool isValidCPUName(StringRef Name) const override;
  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;
  bool setCPU(const std::string &Name) override;
  std::optional<std::string> handleAsmEscapedChar(char EscChar) const override;
  StringRef getABI() const override { return ABI; }

  std::pair<unsigned, unsigned> hardwareInterferenceSizes() const override {
    return std::make_pair(32, 32);
  }

protected:
  std::string CPU;
  StringRef ABI;
  StringRef DefineName;
  StringRef Arch;
  int NumFlashBanks = 0;
};

} // namespace targets
} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_BASIC_TARGETS_AVR_H
