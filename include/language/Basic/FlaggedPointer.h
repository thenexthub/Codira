//===--- FlaggedPointer.h - Explicit pointer tagging container --*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// This file defines the FlaggedPointer class.
//
//===----------------------------------------------------------------------===//
//
#ifndef LANGUAGE_BASIC_FLAGGEDPOINTER_H
#define LANGUAGE_BASIC_FLAGGEDPOINTER_H

#include <algorithm>
#include <cassert>

#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/PointerLikeTypeTraits.h"

namespace language {

/// This class implements a pair of a pointer and boolean flag.
/// Like PointerIntPair, it represents this by mangling a bit into the low part
/// of the pointer, taking advantage of pointer alignment. Unlike
/// PointerIntPair, you must specify the bit position explicitly, instead of
/// automatically placing an integer into the highest bits possible.
///
/// Composing this with `PointerIntPair` is not allowed.
template <typename PointerTy,
          unsigned BitPosition,
          typename PtrTraits = toolchain::PointerLikeTypeTraits<PointerTy>>
class FlaggedPointer {
  intptr_t Value;
  static_assert(PtrTraits::NumLowBitsAvailable > 0,
                "Not enough bits to store flag at this position");
  enum : uintptr_t {
    FlagMask = (uintptr_t)1 << BitPosition,
    PointerBitMask = ~FlagMask
  };
public:
  FlaggedPointer() : Value(0) {}
  FlaggedPointer(PointerTy PtrVal, bool FlagVal) {
    setPointerAndFlag(PtrVal, FlagVal);
  }
  explicit FlaggedPointer(PointerTy PtrVal) {
    initWithPointer(PtrVal);
  }

  /// Returns the underlying pointer with the flag bit masked out.
  PointerTy getPointer() const {
    return PtrTraits::getFromVoidPointer(
      reinterpret_cast<void*>(Value & PointerBitMask));
  }

  void setPointer(PointerTy PtrVal) {
    intptr_t PtrWord = reinterpret_cast<intptr_t>(
      PtrTraits::getAsVoidPointer(PtrVal));
    assert((PtrWord & ~PointerBitMask) == 0 &&
           "Pointer is not sufficiently aligned");
    Value = PtrWord | (Value & ~PointerBitMask);
  }

  bool getFlag() const {
    return (bool)(Value & FlagMask);
  }

  void setFlag(bool FlagVal) {
    intptr_t FlagWord = static_cast<intptr_t>(FlagVal);

    Value &= ~FlagMask;
    Value |= FlagWord << BitPosition;
  }

  /// Set the pointer value and assert if it overlaps with
  /// the flag's bit position.
  void initWithPointer(PointerTy PtrVal) {
    intptr_t PtrWord = reinterpret_cast<intptr_t>(
      PtrTraits::getAsVoidPointer(PtrVal));
    assert((PtrWord & ~PointerBitMask) == 0 &&
      "Pointer is not sufficiently aligned");
    Value = PtrWord;
  }

  /// Set the pointer value, set the flag, and assert
  /// if the pointer's value would overlap with the flag's
  /// bit position.
  void setPointerAndFlag(PointerTy PtrVal, bool FlagVal) {
    intptr_t PtrWord = reinterpret_cast<intptr_t>(
      PtrTraits::getAsVoidPointer(PtrVal));
    assert((PtrWord & ~PointerBitMask) == 0 &&
      "Pointer is not sufficiently aligned");
    intptr_t FlagWord = static_cast<intptr_t>(FlagVal);

    Value = PtrWord | (FlagWord << BitPosition);
  }

  PointerTy const *getAddrOfPointer() const {
    return const_cast<FlaggedPointer *>(this)->getAddrOfPointer();
  }

  PointerTy *getAddrOfPointer() {
    assert(Value == reinterpret_cast<intptr_t>(getPointer()) &&
      "Can only return the address if IntBits is cleared and "
      "PtrTraits doesn't change the pointer");
    return reinterpret_cast<PointerTy *>(&Value);
  }

  /// Get the raw pointer value for the underlying pointer
  /// including its flag value.
  void *getOpaqueValue() const {
    return reinterpret_cast<void*>(Value);
  }

  void setFromOpaqueValue(void *Val) {
    Value = reinterpret_cast<intptr_t>(Val);
  }

  static FlaggedPointer getFromOpaqueValue(const void *V) {
    FlaggedPointer P;
    P.setFromOpaqueValue(const_cast<void *>(V));
    return P;
  }

  bool operator==(const FlaggedPointer &RHS) const {
    return Value == RHS.Value;
  }
  bool operator!=(const FlaggedPointer &RHS) const {
    return Value != RHS.Value;
  }
  bool operator<(const FlaggedPointer &RHS) const {
    return Value < RHS.Value;
  }
  bool operator>(const FlaggedPointer &RHS) const {
    return Value > RHS.Value;
  }
  bool operator<=(const FlaggedPointer &RHS) const {
    return Value <= RHS.Value;
  }
  bool operator>=(const FlaggedPointer &RHS) const {
    return Value >= RHS.Value;
  }
};

} // end namespace language

// Teach SmallPtrSet that FlaggedPointer is "basically a pointer".
template <typename PointerTy, unsigned BitPosition, typename PtrTraits>
struct toolchain::PointerLikeTypeTraits<
  language::FlaggedPointer<PointerTy, BitPosition, PtrTraits>> {
public:
  static inline void *
    getAsVoidPointer(const language::FlaggedPointer<PointerTy, BitPosition> &P) {
      return P.getOpaqueValue();
  }
  static inline language::FlaggedPointer<PointerTy, BitPosition>
    getFromVoidPointer(void *P) {
      return language::FlaggedPointer<PointerTy, BitPosition>::getFromOpaqueValue(P);
  }
  static inline language::FlaggedPointer<PointerTy, BitPosition>
    getFromVoidPointer(const void *P) {
      return language::FlaggedPointer<PointerTy, BitPosition>::getFromOpaqueValue(P);
  }
  enum {
    NumLowBitsAvailable = (BitPosition >= PtrTraits::NumLowBitsAvailable)
      ? PtrTraits::NumLowBitsAvailable
      : (std::min(int(BitPosition + 1),
        int(PtrTraits::NumLowBitsAvailable)) - 1)
  };
};

#endif // LANGUAGE_BASIC_FLAGGEDPOINTER_H
