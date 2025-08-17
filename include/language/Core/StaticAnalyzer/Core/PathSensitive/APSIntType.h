//== APSIntType.h - Simple record of the type of APSInts --------*- C++ -*--==//
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

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_APSINTTYPE_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_APSINTTYPE_H

#include "toolchain/ADT/APSInt.h"
#include <tuple>

namespace language::Core {
namespace ento {

/// A record of the "type" of an APSInt, used for conversions.
class APSIntType {
  uint32_t BitWidth;
  bool IsUnsigned;

public:
  constexpr APSIntType(uint32_t Width, bool Unsigned)
      : BitWidth(Width), IsUnsigned(Unsigned) {}

  /* implicit */ APSIntType(const toolchain::APSInt &Value)
    : BitWidth(Value.getBitWidth()), IsUnsigned(Value.isUnsigned()) {}

  uint32_t getBitWidth() const { return BitWidth; }
  bool isUnsigned() const { return IsUnsigned; }

  /// Convert a given APSInt, in place, to match this type.
  ///
  /// This behaves like a C cast: converting 255u8 (0xFF) to s16 gives
  /// 255 (0x00FF), and converting -1s8 (0xFF) to u16 gives 65535 (0xFFFF).
  void apply(toolchain::APSInt &Value) const {
    // Note the order here. We extend first to preserve the sign, if this value
    // is signed, /then/ match the signedness of the result type.
    Value = Value.extOrTrunc(BitWidth);
    Value.setIsUnsigned(IsUnsigned);
  }

  /// Convert and return a new APSInt with the given value, but this
  /// type's bit width and signedness.
  ///
  /// \see apply
  toolchain::APSInt convert(const toolchain::APSInt &Value) const LLVM_READONLY {
    toolchain::APSInt Result(Value, Value.isUnsigned());
    apply(Result);
    return Result;
  }

  /// Returns an all-zero value for this type.
  toolchain::APSInt getZeroValue() const LLVM_READONLY {
    return toolchain::APSInt(BitWidth, IsUnsigned);
  }

  /// Returns the minimum value for this type.
  toolchain::APSInt getMinValue() const LLVM_READONLY {
    return toolchain::APSInt::getMinValue(BitWidth, IsUnsigned);
  }

  /// Returns the maximum value for this type.
  toolchain::APSInt getMaxValue() const LLVM_READONLY {
    return toolchain::APSInt::getMaxValue(BitWidth, IsUnsigned);
  }

  toolchain::APSInt getValue(uint64_t RawValue) const LLVM_READONLY {
    return (toolchain::APSInt(BitWidth, IsUnsigned) = RawValue);
  }

  /// Used to classify whether a value is representable using this type.
  ///
  /// \see testInRange
  enum RangeTestResultKind {
    RTR_Below = -1, ///< Value is less than the minimum representable value.
    RTR_Within = 0, ///< Value is representable using this type.
    RTR_Above = 1   ///< Value is greater than the maximum representable value.
  };

  /// Tests whether a given value is losslessly representable using this type.
  ///
  /// \param Val The value to test.
  /// \param AllowMixedSign Whether or not to allow signedness conversions.
  ///                       This determines whether -1s8 is considered in range
  ///                       for 'unsigned char' (u8).
  RangeTestResultKind testInRange(const toolchain::APSInt &Val,
                                  bool AllowMixedSign) const LLVM_READONLY;

  bool operator==(const APSIntType &Other) const {
    return BitWidth == Other.BitWidth && IsUnsigned == Other.IsUnsigned;
  }

  /// Provide an ordering for finding a common conversion type.
  ///
  /// Unsigned integers are considered to be better conversion types than
  /// signed integers of the same width.
  bool operator<(const APSIntType &Other) const {
    return std::tie(BitWidth, IsUnsigned) <
           std::tie(Other.BitWidth, Other.IsUnsigned);
  }
};

} // end ento namespace
} // end clang namespace

#endif
