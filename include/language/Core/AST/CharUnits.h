//===--- CharUnits.h - Character units for sizes and offsets ----*- C++ -*-===//
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
//  This file defines the CharUnits class
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_CHARUNITS_H
#define LANGUAGE_CORE_AST_CHARUNITS_H

#include "toolchain/ADT/DenseMapInfo.h"
#include "toolchain/Support/Alignment.h"
#include "toolchain/Support/DataTypes.h"
#include "toolchain/Support/MathExtras.h"

namespace language::Core {

  /// CharUnits - This is an opaque type for sizes expressed in character units.
  /// Instances of this type represent a quantity as a multiple of the size
  /// of the standard C type, char, on the target architecture. As an opaque
  /// type, CharUnits protects you from accidentally combining operations on
  /// quantities in bit units and character units.
  ///
  /// In both C and C++, an object of type 'char', 'signed char', or 'unsigned
  /// char' occupies exactly one byte, so 'character unit' and 'byte' refer to
  /// the same quantity of storage. However, we use the term 'character unit'
  /// rather than 'byte' to avoid an implication that a character unit is
  /// exactly 8 bits.
  ///
  /// For portability, never assume that a target character is 8 bits wide. Use
  /// CharUnit values wherever you calculate sizes, offsets, or alignments
  /// in character units.
  class CharUnits {
    public:
      typedef int64_t QuantityType;

    private:
      QuantityType Quantity = 0;

      explicit CharUnits(QuantityType C) : Quantity(C) {}

    public:

      /// CharUnits - A default constructor.
      CharUnits() = default;

      /// Zero - Construct a CharUnits quantity of zero.
      static CharUnits Zero() {
        return CharUnits(0);
      }

      /// One - Construct a CharUnits quantity of one.
      static CharUnits One() {
        return CharUnits(1);
      }

      /// fromQuantity - Construct a CharUnits quantity from a raw integer type.
      static CharUnits fromQuantity(QuantityType Quantity) {
        return CharUnits(Quantity);
      }

      /// fromQuantity - Construct a CharUnits quantity from an toolchain::Align
      /// quantity.
      static CharUnits fromQuantity(toolchain::Align Quantity) {
        return CharUnits(Quantity.value());
      }

      // Compound assignment.
      CharUnits& operator+= (const CharUnits &Other) {
        Quantity += Other.Quantity;
        return *this;
      }
      CharUnits& operator++ () {
        ++Quantity;
        return *this;
      }
      CharUnits operator++ (int) {
        return CharUnits(Quantity++);
      }
      CharUnits& operator-= (const CharUnits &Other) {
        Quantity -= Other.Quantity;
        return *this;
      }
      CharUnits& operator-- () {
        --Quantity;
        return *this;
      }
      CharUnits operator-- (int) {
        return CharUnits(Quantity--);
      }

      // Comparison operators.
      bool operator== (const CharUnits &Other) const {
        return Quantity == Other.Quantity;
      }
      bool operator!= (const CharUnits &Other) const {
        return Quantity != Other.Quantity;
      }

      // Relational operators.
      bool operator<  (const CharUnits &Other) const {
        return Quantity <  Other.Quantity;
      }
      bool operator<= (const CharUnits &Other) const {
        return Quantity <= Other.Quantity;
      }
      bool operator>  (const CharUnits &Other) const {
        return Quantity >  Other.Quantity;
      }
      bool operator>= (const CharUnits &Other) const {
        return Quantity >= Other.Quantity;
      }

      // Other predicates.

      /// isZero - Test whether the quantity equals zero.
      bool isZero() const     { return Quantity == 0; }

      /// isOne - Test whether the quantity equals one.
      bool isOne() const      { return Quantity == 1; }

      /// isPositive - Test whether the quantity is greater than zero.
      bool isPositive() const { return Quantity  > 0; }

      /// isNegative - Test whether the quantity is less than zero.
      bool isNegative() const { return Quantity  < 0; }

      /// isPowerOfTwo - Test whether the quantity is a power of two.
      /// Zero is not a power of two.
      bool isPowerOfTwo() const {
        return (Quantity & -Quantity) == Quantity;
      }

      /// Test whether this is a multiple of the other value.
      ///
      /// Among other things, this promises that
      /// self.alignTo(N) will just return self.
      bool isMultipleOf(CharUnits N) const {
        return (*this % N) == 0;
      }

      // Arithmetic operators.
      CharUnits operator* (QuantityType N) const {
        return CharUnits(Quantity * N);
      }
      CharUnits &operator*= (QuantityType N) {
        Quantity *= N;
        return *this;
      }
      CharUnits operator/ (QuantityType N) const {
        return CharUnits(Quantity / N);
      }
      CharUnits &operator/= (QuantityType N) {
        Quantity /= N;
        return *this;
      }
      QuantityType operator/ (const CharUnits &Other) const {
        return Quantity / Other.Quantity;
      }
      CharUnits operator% (QuantityType N) const {
        return CharUnits(Quantity % N);
      }
      QuantityType operator% (const CharUnits &Other) const {
        return Quantity % Other.Quantity;
      }
      CharUnits operator+ (const CharUnits &Other) const {
        return CharUnits(Quantity + Other.Quantity);
      }
      CharUnits operator- (const CharUnits &Other) const {
        return CharUnits(Quantity - Other.Quantity);
      }
      CharUnits operator- () const {
        return CharUnits(-Quantity);
      }


      // Conversions.

      /// getQuantity - Get the raw integer representation of this quantity.
      QuantityType getQuantity() const { return Quantity; }

      /// getAsAlign - Returns Quantity as a valid toolchain::Align,
      /// Beware toolchain::Align assumes power of two 8-bit bytes.
      toolchain::Align getAsAlign() const { return toolchain::Align(Quantity); }

      /// getAsMaybeAlign - Returns Quantity as a valid toolchain::Align or
      /// std::nullopt, Beware toolchain::MaybeAlign assumes power of two 8-bit
      /// bytes.
      toolchain::MaybeAlign getAsMaybeAlign() const {
        return toolchain::MaybeAlign(Quantity);
      }

      /// alignTo - Returns the next integer (mod 2**64) that is
      /// greater than or equal to this quantity and is a multiple of \p Align.
      /// Align must be non-zero.
      CharUnits alignTo(const CharUnits &Align) const {
        return CharUnits(toolchain::alignTo(Quantity, Align.Quantity));
      }

      /// Given that this is a non-zero alignment value, what is the
      /// alignment at the given offset?
      CharUnits alignmentAtOffset(CharUnits offset) const {
        assert(Quantity != 0 && "offsetting from unknown alignment?");
        return CharUnits(toolchain::MinAlign(Quantity, offset.Quantity));
      }

      /// Given that this is the alignment of the first element of an
      /// array, return the minimum alignment of any element in the array.
      CharUnits alignmentOfArrayElement(CharUnits elementSize) const {
        // Since we don't track offsetted alignments, the alignment of
        // the second element (or any odd element) will be minimally
        // aligned.
        return alignmentAtOffset(elementSize);
      }


  }; // class CharUnit
} // namespace language::Core

inline language::Core::CharUnits operator* (language::Core::CharUnits::QuantityType Scale,
                                   const language::Core::CharUnits &CU) {
  return CU * Scale;
}

namespace toolchain {

template<> struct DenseMapInfo<language::Core::CharUnits> {
  static language::Core::CharUnits getEmptyKey() {
    language::Core::CharUnits::QuantityType Quantity =
      DenseMapInfo<language::Core::CharUnits::QuantityType>::getEmptyKey();

    return language::Core::CharUnits::fromQuantity(Quantity);
  }

  static language::Core::CharUnits getTombstoneKey() {
    language::Core::CharUnits::QuantityType Quantity =
      DenseMapInfo<language::Core::CharUnits::QuantityType>::getTombstoneKey();

    return language::Core::CharUnits::fromQuantity(Quantity);
  }

  static unsigned getHashValue(const language::Core::CharUnits &CU) {
    language::Core::CharUnits::QuantityType Quantity = CU.getQuantity();
    return DenseMapInfo<language::Core::CharUnits::QuantityType>::getHashValue(Quantity);
  }

  static bool isEqual(const language::Core::CharUnits &LHS,
                      const language::Core::CharUnits &RHS) {
    return LHS == RHS;
  }
};

} // end namespace toolchain

#endif // LANGUAGE_CORE_AST_CHARUNITS_H
