//===--- Floating.h - Types for the constexpr VM ----------------*- C++ -*-===//
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
// Defines the VM types and helpers operating on types.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_INTERP_FLOATING_H
#define LANGUAGE_CORE_AST_INTERP_FLOATING_H

#include "Primitives.h"
#include "language/Core/AST/APValue.h"
#include "toolchain/ADT/APFloat.h"

// XXX This is just a debugging help. Setting this to 1 will heap-allocate ALL
// floating values.
#define ALLOCATE_ALL 0

namespace language::Core {
namespace interp {

using APFloat = toolchain::APFloat;
using APSInt = toolchain::APSInt;
using APInt = toolchain::APInt;

/// If a Floating is constructed from Memory, it DOES NOT OWN THAT MEMORY.
/// It will NOT copy the memory (unless, of course, copy() is called) and it
/// won't alllocate anything. The allocation should happen via InterpState or
/// Program.
class Floating final {
private:
  union {
    uint64_t Val = 0;
    uint64_t *Memory;
  };
  toolchain::APFloatBase::Semantics Semantics;

  APFloat getValue() const {
    unsigned BitWidth = bitWidth();
    if (singleWord())
      return APFloat(getSemantics(), APInt(BitWidth, Val));
    unsigned NumWords = numWords();
    return APFloat(getSemantics(), APInt(BitWidth, NumWords, Memory));
  }

public:
  Floating() = default;
  Floating(toolchain::APFloatBase::Semantics Semantics)
      : Val(0), Semantics(Semantics) {}
  Floating(const APFloat &F) {

    Semantics = toolchain::APFloatBase::SemanticsToEnum(F.getSemantics());
    this->copy(F);
  }
  Floating(uint64_t *Memory, toolchain::APFloatBase::Semantics Semantics)
      : Memory(Memory), Semantics(Semantics) {}

  APFloat getAPFloat() const { return getValue(); }

  bool operator<(Floating RHS) const { return getValue() < RHS.getValue(); }
  bool operator>(Floating RHS) const { return getValue() > RHS.getValue(); }
  bool operator<=(Floating RHS) const { return getValue() <= RHS.getValue(); }
  bool operator>=(Floating RHS) const { return getValue() >= RHS.getValue(); }

  APFloat::opStatus convertToInteger(APSInt &Result) const {
    bool IsExact;
    return getValue().convertToInteger(Result, toolchain::APFloat::rmTowardZero,
                                       &IsExact);
  }

  void toSemantics(const toolchain::fltSemantics *Sem, toolchain::RoundingMode RM,
                   Floating *Result) const {
    APFloat Copy = getValue();
    bool LosesInfo;
    Copy.convert(*Sem, RM, &LosesInfo);
    (void)LosesInfo;
    Result->copy(Copy);
  }

  APSInt toAPSInt(unsigned NumBits = 0) const {
    return APSInt(getValue().bitcastToAPInt());
  }
  APValue toAPValue(const ASTContext &) const { return APValue(getValue()); }
  void print(toolchain::raw_ostream &OS) const {
    // Can't use APFloat::print() since it appends a newline.
    SmallVector<char, 16> Buffer;
    getValue().toString(Buffer);
    OS << Buffer;
  }
  std::string toDiagnosticString(const ASTContext &Ctx) const {
    std::string NameStr;
    toolchain::raw_string_ostream OS(NameStr);
    print(OS);
    return NameStr;
  }

  unsigned bitWidth() const {
    return toolchain::APFloatBase::semanticsSizeInBits(getSemantics());
  }
  unsigned numWords() const { return toolchain::APInt::getNumWords(bitWidth()); }
  bool singleWord() const {
#if ALLOCATE_ALL
    return false;
#endif
    return numWords() == 1;
  }
  static bool singleWord(const toolchain::fltSemantics &Sem) {
#if ALLOCATE_ALL
    return false;
#endif
    return APInt::getNumWords(toolchain::APFloatBase::getSizeInBits(Sem)) == 1;
  }
  const toolchain::fltSemantics &getSemantics() const {
    return toolchain::APFloatBase::EnumToSemantics(Semantics);
  }

  void copy(const APFloat &F) {
    if (singleWord()) {
      Val = F.bitcastToAPInt().getZExtValue();
    } else {
      assert(Memory);
      std::memcpy(Memory, F.bitcastToAPInt().getRawData(),
                  numWords() * sizeof(uint64_t));
    }
  }

  void take(uint64_t *NewMemory) {
    if (singleWord())
      return;

    if (Memory)
      std::memcpy(NewMemory, Memory, numWords() * sizeof(uint64_t));
    Memory = NewMemory;
  }

  bool isSigned() const { return true; }
  bool isNegative() const { return getValue().isNegative(); }
  bool isZero() const { return getValue().isZero(); }
  bool isNonZero() const { return getValue().isNonZero(); }
  bool isMin() const { return getValue().isSmallest(); }
  bool isMinusOne() const { return getValue().isExactlyValue(-1.0); }
  bool isNan() const { return getValue().isNaN(); }
  bool isSignaling() const { return getValue().isSignaling(); }
  bool isInf() const { return getValue().isInfinity(); }
  bool isFinite() const { return getValue().isFinite(); }
  bool isNormal() const { return getValue().isNormal(); }
  bool isDenormal() const { return getValue().isDenormal(); }
  toolchain::FPClassTest classify() const { return getValue().classify(); }
  APFloat::fltCategory getCategory() const { return getValue().getCategory(); }

  ComparisonCategoryResult compare(const Floating &RHS) const {
    toolchain::APFloatBase::cmpResult CmpRes = getValue().compare(RHS.getValue());
    switch (CmpRes) {
    case toolchain::APFloatBase::cmpLessThan:
      return ComparisonCategoryResult::Less;
    case toolchain::APFloatBase::cmpEqual:
      return ComparisonCategoryResult::Equal;
    case toolchain::APFloatBase::cmpGreaterThan:
      return ComparisonCategoryResult::Greater;
    case toolchain::APFloatBase::cmpUnordered:
      return ComparisonCategoryResult::Unordered;
    }
    toolchain_unreachable("Inavlid cmpResult value");
  }

  static APFloat::opStatus fromIntegral(APSInt Val,
                                        const toolchain::fltSemantics &Sem,
                                        toolchain::RoundingMode RM,
                                        Floating *Result) {
    APFloat F = APFloat(Sem);
    APFloat::opStatus Status = F.convertFromAPInt(Val, Val.isSigned(), RM);
    Result->copy(F);
    return Status;
  }

  static void bitcastFromMemory(const std::byte *Buff,
                                const toolchain::fltSemantics &Sem,
                                Floating *Result) {
    size_t Size = APFloat::semanticsSizeInBits(Sem);
    toolchain::APInt API(Size, true);
    toolchain::LoadIntFromMemory(API, (const uint8_t *)Buff, Size / 8);
    Result->copy(APFloat(Sem, API));
  }

  void bitcastToMemory(std::byte *Buff) const {
    toolchain::APInt API = getValue().bitcastToAPInt();
    toolchain::StoreIntToMemory(API, (uint8_t *)Buff, bitWidth() / 8);
  }

  // === Serialization support ===
  size_t bytesToSerialize() const {
    return sizeof(Semantics) + (numWords() * sizeof(uint64_t));
  }

  void serialize(std::byte *Buff) const {
    std::memcpy(Buff, &Semantics, sizeof(Semantics));
    if (singleWord()) {
      std::memcpy(Buff + sizeof(Semantics), &Val, sizeof(uint64_t));
    } else {
      std::memcpy(Buff + sizeof(Semantics), Memory,
                  numWords() * sizeof(uint64_t));
    }
  }

  static toolchain::APFloatBase::Semantics
  deserializeSemantics(const std::byte *Buff) {
    return *reinterpret_cast<const toolchain::APFloatBase::Semantics *>(Buff);
  }

  static void deserialize(const std::byte *Buff, Floating *Result) {
    toolchain::APFloatBase::Semantics Semantics;
    std::memcpy(&Semantics, Buff, sizeof(Semantics));

    unsigned BitWidth = toolchain::APFloat::semanticsSizeInBits(
        toolchain::APFloatBase::EnumToSemantics(Semantics));
    unsigned NumWords = toolchain::APInt::getNumWords(BitWidth);

    Result->Semantics = Semantics;
    if (NumWords == 1 && !ALLOCATE_ALL) {
      std::memcpy(&Result->Val, Buff + sizeof(Semantics), sizeof(uint64_t));
    } else {
      assert(Result->Memory);
      std::memcpy(Result->Memory, Buff + sizeof(Semantics),
                  NumWords * sizeof(uint64_t));
    }
  }

  // -------

  static APFloat::opStatus add(const Floating &A, const Floating &B,
                               toolchain::RoundingMode RM, Floating *R) {
    APFloat LHS = A.getValue();
    APFloat RHS = B.getValue();

    auto Status = LHS.add(RHS, RM);
    R->copy(LHS);
    return Status;
  }

  static APFloat::opStatus increment(const Floating &A, toolchain::RoundingMode RM,
                                     Floating *R) {
    APFloat One(A.getSemantics(), 1);
    APFloat LHS = A.getValue();

    auto Status = LHS.add(One, RM);
    R->copy(LHS);
    return Status;
  }

  static APFloat::opStatus sub(const Floating &A, const Floating &B,
                               toolchain::RoundingMode RM, Floating *R) {
    APFloat LHS = A.getValue();
    APFloat RHS = B.getValue();

    auto Status = LHS.subtract(RHS, RM);
    R->copy(LHS);
    return Status;
  }

  static APFloat::opStatus decrement(const Floating &A, toolchain::RoundingMode RM,
                                     Floating *R) {
    APFloat One(A.getSemantics(), 1);
    APFloat LHS = A.getValue();

    auto Status = LHS.subtract(One, RM);
    R->copy(LHS);
    return Status;
  }

  static APFloat::opStatus mul(const Floating &A, const Floating &B,
                               toolchain::RoundingMode RM, Floating *R) {

    APFloat LHS = A.getValue();
    APFloat RHS = B.getValue();

    auto Status = LHS.multiply(RHS, RM);
    R->copy(LHS);
    return Status;
  }

  static APFloat::opStatus div(const Floating &A, const Floating &B,
                               toolchain::RoundingMode RM, Floating *R) {
    APFloat LHS = A.getValue();
    APFloat RHS = B.getValue();

    auto Status = LHS.divide(RHS, RM);
    R->copy(LHS);
    return Status;
  }

  static bool neg(const Floating &A, Floating *R) {
    R->copy(-A.getValue());
    return false;
  }
};

toolchain::raw_ostream &operator<<(toolchain::raw_ostream &OS, Floating F);
Floating getSwappedBytes(Floating F);

} // namespace interp
} // namespace language::Core

#endif
