//===------- FixedPoint.h - Fixedd point types for the VM -------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_AST_INTERP_FIXED_POINT_H
#define LANGUAGE_CORE_AST_INTERP_FIXED_POINT_H

#include "language/Core/AST/APValue.h"
#include "language/Core/AST/ComparisonCategories.h"
#include "toolchain/ADT/APFixedPoint.h"

namespace language::Core {
namespace interp {

using APInt = toolchain::APInt;
using APSInt = toolchain::APSInt;

/// Wrapper around fixed point types.
class FixedPoint final {
private:
  toolchain::APFixedPoint V;

public:
  FixedPoint(toolchain::APFixedPoint &&V) : V(std::move(V)) {}
  FixedPoint(toolchain::APFixedPoint &V) : V(V) {}
  FixedPoint(APInt V, toolchain::FixedPointSemantics Sem) : V(V, Sem) {}
  // This needs to be default-constructible so toolchain::endian::read works.
  FixedPoint()
      : V(APInt(0, 0ULL, false),
          toolchain::FixedPointSemantics(0, 0, false, false, false)) {}

  static FixedPoint zero(toolchain::FixedPointSemantics Sem) {
    return FixedPoint(APInt(Sem.getWidth(), 0ULL, Sem.isSigned()), Sem);
  }

  static FixedPoint from(const APSInt &I, toolchain::FixedPointSemantics Sem,
                         bool *Overflow) {
    return FixedPoint(toolchain::APFixedPoint::getFromIntValue(I, Sem, Overflow));
  }
  static FixedPoint from(const toolchain::APFloat &I, toolchain::FixedPointSemantics Sem,
                         bool *Overflow) {
    return FixedPoint(toolchain::APFixedPoint::getFromFloatValue(I, Sem, Overflow));
  }

  operator bool() const { return V.getBoolValue(); }
  void print(toolchain::raw_ostream &OS) const { OS << V; }

  APValue toAPValue(const ASTContext &) const { return APValue(V); }
  APSInt toAPSInt(unsigned BitWidth = 0) const { return V.getValue(); }

  unsigned bitWidth() const { return V.getWidth(); }
  bool isSigned() const { return V.isSigned(); }
  bool isZero() const { return V.getValue().isZero(); }
  bool isNegative() const { return V.getValue().isNegative(); }
  bool isPositive() const { return V.getValue().isNonNegative(); }
  bool isMin() const {
    return V == toolchain::APFixedPoint::getMin(V.getSemantics());
  }
  bool isMinusOne() const { return V.isSigned() && V.getValue() == -1; }

  FixedPoint truncate(unsigned BitWidth) const { return *this; }

  FixedPoint toSemantics(const toolchain::FixedPointSemantics &Sem,
                         bool *Overflow) const {
    return FixedPoint(V.convert(Sem, Overflow));
  }
  toolchain::FixedPointSemantics getSemantics() const { return V.getSemantics(); }

  toolchain::APFloat toFloat(const toolchain::fltSemantics *Sem) const {
    return V.convertToFloat(*Sem);
  }

  toolchain::APSInt toInt(unsigned BitWidth, bool Signed, bool *Overflow) const {
    return V.convertToInt(BitWidth, Signed, Overflow);
  }

  std::string toDiagnosticString(const ASTContext &Ctx) const {
    return V.toString();
  }

  ComparisonCategoryResult compare(const FixedPoint &Other) const {
    int c = V.compare(Other.V);
    if (c == 0)
      return ComparisonCategoryResult::Equal;
    else if (c < 0)
      return ComparisonCategoryResult::Less;
    return ComparisonCategoryResult::Greater;
  }

  size_t bytesToSerialize() const {
    return sizeof(uint32_t) + (V.getValue().getBitWidth() / CHAR_BIT);
  }

  void serialize(std::byte *Buff) const {
    // Semantics followed by APInt.
    uint32_t SemI = V.getSemantics().toOpaqueInt();
    std::memcpy(Buff, &SemI, sizeof(SemI));

    toolchain::APInt API = V.getValue();
    toolchain::StoreIntToMemory(API, (uint8_t *)(Buff + sizeof(SemI)),
                           bitWidth() / 8);
  }

  static FixedPoint deserialize(const std::byte *Buff) {
    auto Sem = toolchain::FixedPointSemantics::getFromOpaqueInt(
        *reinterpret_cast<const uint32_t *>(Buff));
    unsigned BitWidth = Sem.getWidth();
    APInt I(BitWidth, 0ull, !Sem.isSigned());
    toolchain::LoadIntFromMemory(
        I, reinterpret_cast<const uint8_t *>(Buff + sizeof(uint32_t)),
        BitWidth / CHAR_BIT);

    return FixedPoint(I, Sem);
  }

  static bool neg(const FixedPoint &A, FixedPoint *R) {
    bool Overflow = false;
    *R = FixedPoint(A.V.negate(&Overflow));
    return Overflow;
  }

  static bool add(const FixedPoint A, const FixedPoint B, unsigned Bits,
                  FixedPoint *R) {
    bool Overflow = false;
    *R = FixedPoint(A.V.add(B.V, &Overflow));
    return Overflow;
  }
  static bool sub(const FixedPoint A, const FixedPoint B, unsigned Bits,
                  FixedPoint *R) {
    bool Overflow = false;
    *R = FixedPoint(A.V.sub(B.V, &Overflow));
    return Overflow;
  }
  static bool mul(const FixedPoint A, const FixedPoint B, unsigned Bits,
                  FixedPoint *R) {
    bool Overflow = false;
    *R = FixedPoint(A.V.mul(B.V, &Overflow));
    return Overflow;
  }
  static bool div(const FixedPoint A, const FixedPoint B, unsigned Bits,
                  FixedPoint *R) {
    bool Overflow = false;
    *R = FixedPoint(A.V.div(B.V, &Overflow));
    return Overflow;
  }

  static bool shiftLeft(const FixedPoint A, const FixedPoint B, unsigned OpBits,
                        FixedPoint *R) {
    unsigned Amt = B.V.getValue().getLimitedValue(OpBits);
    bool Overflow;
    *R = FixedPoint(A.V.shl(Amt, &Overflow));
    return Overflow;
  }
  static bool shiftRight(const FixedPoint A, const FixedPoint B,
                         unsigned OpBits, FixedPoint *R) {
    unsigned Amt = B.V.getValue().getLimitedValue(OpBits);
    bool Overflow;
    *R = FixedPoint(A.V.shr(Amt, &Overflow));
    return Overflow;
  }

  static bool rem(const FixedPoint A, const FixedPoint B, unsigned Bits,
                  FixedPoint *R) {
    toolchain_unreachable("Rem doesn't exist for fixed point values");
    return true;
  }
  static bool bitAnd(const FixedPoint A, const FixedPoint B, unsigned Bits,
                     FixedPoint *R) {
    return true;
  }
  static bool bitOr(const FixedPoint A, const FixedPoint B, unsigned Bits,
                    FixedPoint *R) {
    return true;
  }
  static bool bitXor(const FixedPoint A, const FixedPoint B, unsigned Bits,
                     FixedPoint *R) {
    return true;
  }

  static bool increment(const FixedPoint &A, FixedPoint *R) { return true; }
  static bool decrement(const FixedPoint &A, FixedPoint *R) { return true; }
};

inline FixedPoint getSwappedBytes(FixedPoint F) { return F; }

inline toolchain::raw_ostream &operator<<(toolchain::raw_ostream &OS, FixedPoint F) {
  F.print(OS);
  return OS;
}

} // namespace interp
} // namespace language::Core

#endif
