//===--- UnicodeCharRanges.h - Types and functions for character ranges ---===//
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
#ifndef LLVM_SUPPORT_UNICODECHARRANGES_H
#define LLVM_SUPPORT_UNICODECHARRANGES_H

#include <IndexStoreDB_LLVMSupport/toolchain_ADT_ArrayRef.h>
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_SmallPtrSet.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_Compiler.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_Debug.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_Mutex.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_MutexGuard.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_raw_ostream.h>
#include <algorithm>

#define DEBUG_TYPE "unicode"

namespace toolchain {
namespace sys {

/// Represents a closed range of Unicode code points [Lower, Upper].
struct UnicodeCharRange {
  uint32_t Lower;
  uint32_t Upper;
};

inline bool operator<(uint32_t Value, UnicodeCharRange Range) {
  return Value < Range.Lower;
}
inline bool operator<(UnicodeCharRange Range, uint32_t Value) {
  return Range.Upper < Value;
}

/// Holds a reference to an ordered array of UnicodeCharRange and allows
/// to quickly check if a code point is contained in the set represented by this
/// array.
class UnicodeCharSet {
public:
  typedef ArrayRef<UnicodeCharRange> CharRanges;

  /// Constructs a UnicodeCharSet instance from an array of
  /// UnicodeCharRanges.
  ///
  /// Array pointed by \p Ranges should have the lifetime at least as long as
  /// the UnicodeCharSet instance, and should not change. Array is validated by
  /// the constructor, so it makes sense to create as few UnicodeCharSet
  /// instances per each array of ranges, as possible.
#ifdef NDEBUG

  // FIXME: This could use constexpr + static_assert. This way we
  // may get rid of NDEBUG in this header. Unfortunately there are some
  // problems to get this working with MSVC 2013. Change this when
  // the support for MSVC 2013 is dropped.
  constexpr UnicodeCharSet(CharRanges Ranges) : Ranges(Ranges) {}
#else
  UnicodeCharSet(CharRanges Ranges) : Ranges(Ranges) {
    assert(rangesAreValid());
  }
#endif

  /// Returns true if the character set contains the Unicode code point
  /// \p C.
  bool contains(uint32_t C) const {
    return std::binary_search(Ranges.begin(), Ranges.end(), C);
  }

private:
  /// Returns true if each of the ranges is a proper closed range
  /// [min, max], and if the ranges themselves are ordered and non-overlapping.
  bool rangesAreValid() const {
    uint32_t Prev = 0;
    for (CharRanges::const_iterator I = Ranges.begin(), E = Ranges.end();
         I != E; ++I) {
      if (I != Ranges.begin() && Prev >= I->Lower) {
        LLVM_DEBUG(dbgs() << "Upper bound 0x");
        LLVM_DEBUG(dbgs().write_hex(Prev));
        LLVM_DEBUG(dbgs() << " should be less than succeeding lower bound 0x");
        LLVM_DEBUG(dbgs().write_hex(I->Lower) << "\n");
        return false;
      }
      if (I->Upper < I->Lower) {
        LLVM_DEBUG(dbgs() << "Upper bound 0x");
        LLVM_DEBUG(dbgs().write_hex(I->Lower));
        LLVM_DEBUG(dbgs() << " should not be less than lower bound 0x");
        LLVM_DEBUG(dbgs().write_hex(I->Upper) << "\n");
        return false;
      }
      Prev = I->Upper;
    }

    return true;
  }

  const CharRanges Ranges;
};

} // namespace sys
} // namespace toolchain

#undef DEBUG_TYPE // "unicode"

#endif // LLVM_SUPPORT_UNICODECHARRANGES_H
