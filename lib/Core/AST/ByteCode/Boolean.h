//===--- Boolean.h - Wrapper for boolean types for the VM -------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_AST_INTERP_BOOLEAN_H
#define LANGUAGE_CORE_AST_INTERP_BOOLEAN_H

#include "Integral.h"
#include "language/Core/AST/APValue.h"
#include "language/Core/AST/ComparisonCategories.h"
#include "toolchain/ADT/APSInt.h"
#include "toolchain/Support/MathExtras.h"
#include "toolchain/Support/raw_ostream.h"
#include <cstddef>
#include <cstdint>

namespace language::Core {
namespace interp {

/// Wrapper around boolean types.
class Boolean final {
private:
  /// Underlying boolean.
  bool V;

public:
  /// Zero-initializes a boolean.
  Boolean() : V(false) {}
  explicit Boolean(bool V) : V(V) {}

  bool operator<(Boolean RHS) const { return V < RHS.V; }
  bool operator>(Boolean RHS) const { return V > RHS.V; }
  bool operator>(unsigned RHS) const { return static_cast<unsigned>(V) > RHS; }

  Boolean operator-() const { return Boolean(V); }
  Boolean operator-(const Boolean &Other) const { return Boolean(V - Other.V); }
  Boolean operator~() const { return Boolean(true); }
  Boolean operator!() const { return Boolean(!V); }

  template <typename Ty, typename = std::enable_if_t<std::is_integral_v<Ty>>>
  explicit operator Ty() const {
    return V;
  }

  APSInt toAPSInt() const {
    return APSInt(APInt(1, static_cast<uint64_t>(V), false), true);
  }
  APSInt toAPSInt(unsigned NumBits) const {
    return APSInt(toAPSInt().zextOrTrunc(NumBits), true);
  }
  APValue toAPValue(const ASTContext &) const { return APValue(toAPSInt()); }

  Boolean toUnsigned() const { return *this; }

  constexpr static unsigned bitWidth() { return 1; }
  bool isZero() const { return !V; }
  bool isMin() const { return isZero(); }

  constexpr static bool isMinusOne() { return false; }

  constexpr static bool isSigned() { return false; }

  constexpr static bool isNegative() { return false; }
  constexpr static bool isPositive() { return !isNegative(); }

  ComparisonCategoryResult compare(const Boolean &RHS) const {
    return Compare(V, RHS.V);
  }

  unsigned countLeadingZeros() const { return V ? 0 : 1; }

  Boolean truncate(unsigned TruncBits) const { return *this; }

  static Boolean bitcastFromMemory(const std::byte *Buff, unsigned BitWidth) {
    // Just load the first byte.
    bool Val = static_cast<bool>(*Buff);
    return Boolean(Val);
  }

  void bitcastToMemory(std::byte *Buff) { std::memcpy(Buff, &V, sizeof(V)); }

  void print(toolchain::raw_ostream &OS) const { OS << (V ? "true" : "false"); }
  std::string toDiagnosticString(const ASTContext &Ctx) const {
    std::string NameStr;
    toolchain::raw_string_ostream OS(NameStr);
    print(OS);
    return NameStr;
  }

  static Boolean min(unsigned NumBits) { return Boolean(false); }
  static Boolean max(unsigned NumBits) { return Boolean(true); }

  template <typename T> static Boolean from(T Value) {
    if constexpr (std::is_integral<T>::value)
      return Boolean(Value != 0);
    return Boolean(static_cast<decltype(Boolean::V)>(Value) != 0);
  }

  template <unsigned SrcBits, bool SrcSign>
  static std::enable_if_t<SrcBits != 0, Boolean>
  from(Integral<SrcBits, SrcSign> Value) {
    return Boolean(!Value.isZero());
  }

  static Boolean zero() { return from(false); }

  template <typename T> static Boolean from(T Value, unsigned NumBits) {
    return Boolean(Value);
  }

  static bool inRange(int64_t Value, unsigned NumBits) {
    return Value == 0 || Value == 1;
  }

  static bool increment(Boolean A, Boolean *R) {
    *R = Boolean(true);
    return false;
  }

  static bool decrement(Boolean A, Boolean *R) {
    toolchain_unreachable("Cannot decrement booleans");
  }

  static bool add(Boolean A, Boolean B, unsigned OpBits, Boolean *R) {
    *R = Boolean(A.V || B.V);
    return false;
  }

  static bool sub(Boolean A, Boolean B, unsigned OpBits, Boolean *R) {
    *R = Boolean(A.V ^ B.V);
    return false;
  }

  static bool mul(Boolean A, Boolean B, unsigned OpBits, Boolean *R) {
    *R = Boolean(A.V && B.V);
    return false;
  }

  static bool inv(Boolean A, Boolean *R) {
    *R = Boolean(!A.V);
    return false;
  }

  static bool neg(Boolean A, Boolean *R) {
    *R = Boolean(A.V);
    return false;
  }
};

inline toolchain::raw_ostream &operator<<(toolchain::raw_ostream &OS, const Boolean &B) {
  B.print(OS);
  return OS;
}

} // namespace interp
} // namespace language::Core

#endif
