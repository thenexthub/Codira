//===--- SourceLoc.h - Source Locations and Ranges --------------*- C++ -*-===//
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
//  This file defines types used to reason about source locations and ranges.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_SOURCELOC_H
#define LANGUAGE_BASIC_SOURCELOC_H

#include "language/Basic/Debug.h"
#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/DenseMapInfo.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/SMLoc.h"
#include <assert.h>
#include <functional>

namespace language {
  class SourceManager;

/// SourceLoc in language is just an SMLoc.  We define it as a different type
/// (instead of as a typedef) just to remove the "getFromPointer" methods and
/// enforce purity in the Codira codebase.
class SourceLoc {
  friend class SourceManager;
  friend class SourceRange;
  friend class CharSourceRange;
  friend class DiagnosticConsumer;

  toolchain::SMLoc Value;

public:
  SourceLoc() {}
  explicit SourceLoc(toolchain::SMLoc Value) : Value(Value) {}
  
  bool isValid() const { return Value.isValid(); }
  bool isInvalid() const { return !isValid(); }

  /// An explicit bool operator so one can check if a SourceLoc is valid in an
  /// if statement:
  ///
  /// if (auto x = getSourceLoc()) { ... }
  explicit operator bool() const { return isValid(); }

  bool operator==(const SourceLoc &RHS) const { return RHS.Value == Value; }
  bool operator!=(const SourceLoc &RHS) const { return !operator==(RHS); }
  
  /// Return a source location advanced a specified number of bytes.
  SourceLoc getAdvancedLoc(int ByteOffset) const {
    assert(isValid() && "Can't advance an invalid location");
    return SourceLoc(
        toolchain::SMLoc::getFromPointer(Value.getPointer() + ByteOffset));
  }

  SourceLoc getAdvancedLocOrInvalid(int ByteOffset) const {
    if (isValid())
      return getAdvancedLoc(ByteOffset);
    return SourceLoc();
  }

  const void *getOpaquePointerValue() const { return Value.getPointer(); }

  /// Print out the SourceLoc.  If this location is in the same buffer
  /// as specified by \c LastBufferID, then we don't print the filename.  If
  /// not, we do print the filename, and then update \c LastBufferID with the
  /// BufferID printed.
  void print(raw_ostream &OS, const SourceManager &SM,
             unsigned &LastBufferID) const;

  void printLineAndColumn(raw_ostream &OS, const SourceManager &SM,
                          unsigned BufferID = 0) const;

  void print(raw_ostream &OS, const SourceManager &SM) const {
    unsigned Tmp = ~0U;
    print(OS, SM, Tmp);
  }

  LANGUAGE_DEBUG_DUMPER(dump(const SourceManager &SM));

	friend size_t hash_value(SourceLoc loc) {
		return reinterpret_cast<uintptr_t>(loc.getOpaquePointerValue());
	}

	friend void simple_display(raw_ostream &OS, const SourceLoc &loc) {
		// Nothing meaningful to print.
	}
};

/// SourceRange in language is a pair of locations.  However, note that the end
/// location is the start of the last token in the range, not the last character
/// in the range.  This is unlike SMRange, so we use a distinct type to make
/// sure that proper conversions happen where important.
class SourceRange {
public:
  SourceLoc Start, End;

  SourceRange() {}
  SourceRange(SourceLoc Loc) : Start(Loc), End(Loc) {}
  SourceRange(SourceLoc Start, SourceLoc End) : Start(Start), End(End) {
    assert(Start.isValid() == End.isValid() &&
           "Start and end should either both be valid or both be invalid!");
  }
  
  bool isValid() const { return Start.isValid(); }
  bool isInvalid() const { return !isValid(); }

  /// An explicit bool operator so one can check if a SourceRange is valid in an
  /// if statement:
  ///
  /// if (auto x = getSourceRange()) { ... }
  explicit operator bool() const { return isValid(); }

  /// Combine the given source ranges into the smallest contiguous SourceRange
  /// that includes them all, ignoring any invalid ranges present.
  static SourceRange combine(ArrayRef<SourceRange> ranges);

  /// Combine the given source ranges into the smallest contiguous SourceRange
  /// that includes them all, ignoring any invalid ranges present.
  template <typename ...T>
  static SourceRange combine(T... ranges) {
    return SourceRange::combine({ranges...});
  }

  /// Extend this SourceRange to the smallest continuous SourceRange that
  /// includes both this range and the other one.
  void widen(SourceRange Other);

  /// Checks whether this range contains the given location. Note that the given
  /// location should correspond to the start of a token, since locations inside
  /// the last token may be considered outside the range by this function.
  bool contains(SourceLoc Loc) const;

  /// Checks whether this range overlaps with the given range.
  bool overlaps(SourceRange Other) const;

  bool operator==(const SourceRange &other) const {
    return Start == other.Start && End == other.End;
  }
  bool operator!=(const SourceRange &other) const { return !operator==(other); }

  /// Print out the SourceRange.  If the locations are in the same buffer
  /// as specified by LastBufferID, then we don't print the filename.  If not,
  /// we do print the filename, and then update LastBufferID with the BufferID
  /// printed.
  void print(raw_ostream &OS, const SourceManager &SM,
             unsigned &LastBufferID, bool PrintText = true) const;

  void print(raw_ostream &OS, const SourceManager &SM,
             bool PrintText = true) const {
    unsigned Tmp = ~0U;
    print(OS, SM, Tmp, PrintText);
  }

  LANGUAGE_DEBUG_DUMPER(dump(const SourceManager &SM));

  friend size_t hash_value(SourceRange range) {
    return toolchain::hash_combine(range.Start, range.End);
  }

  friend void simple_display(raw_ostream &OS, const SourceRange &loc) {
    // Nothing meaningful to print.
  }

};

/// A half-open character-based source range.
class CharSourceRange {
  SourceLoc Start;
  unsigned ByteLength;

public:
  /// Constructs an invalid range.
  CharSourceRange() = default;

  CharSourceRange(SourceLoc Start, unsigned ByteLength)
    : Start(Start), ByteLength(ByteLength) {}

  /// Constructs a character range which starts and ends at the
  /// specified character locations.
  CharSourceRange(const SourceManager &SM, SourceLoc Start, SourceLoc End);

  /// Use Lexer::getCharSourceRangeFromSourceRange() instead.
  CharSourceRange(const SourceManager &SM, SourceRange Range) = delete;

  bool isValid() const { return Start.isValid(); }
  bool isInvalid() const { return !isValid(); }

  bool operator==(const CharSourceRange &other) const {
    return Start == other.Start && ByteLength == other.ByteLength;
  }
  bool operator!=(const CharSourceRange &other) const {
    return !operator==(other);
  }

  SourceLoc getStart() const { return Start; }
  SourceLoc getEnd() const { return Start.getAdvancedLocOrInvalid(ByteLength); }

  /// Returns true if the given source location is contained in the range.
  bool contains(SourceLoc loc) const {
    auto less = std::less<const char *>();
    auto less_equal = std::less_equal<const char *>();
    return less_equal(getStart().Value.getPointer(), loc.Value.getPointer()) &&
           less(loc.Value.getPointer(), getEnd().Value.getPointer());
  }

  bool contains(CharSourceRange Other) const {
    auto less_equal = std::less_equal<const char *>();
    return contains(Other.getStart()) &&
     less_equal(Other.getEnd().Value.getPointer(), getEnd().Value.getPointer());
  }

  /// expands *this to cover Other
  void widen(CharSourceRange Other) {
    auto Diff = Other.getEnd().Value.getPointer() - getEnd().Value.getPointer();
    if (Diff > 0) {
      ByteLength += Diff;
    }
    const auto MyStartPtr = getStart().Value.getPointer();
    Diff = MyStartPtr - Other.getStart().Value.getPointer();
    if (Diff > 0) {
      ByteLength += Diff;
      Start = SourceLoc(toolchain::SMLoc::getFromPointer(MyStartPtr - Diff));
    }
  }

  bool overlaps(CharSourceRange Other) const {
    if (getByteLength() == 0 || Other.getByteLength() == 0) return false;
    return contains(Other.getStart()) || Other.contains(getStart());
  }

  StringRef str() const {
    return StringRef(Start.Value.getPointer(), ByteLength);
  }

  /// Return the length of this valid range in bytes.  Can be zero.
  unsigned getByteLength() const {
    assert(isValid() && "length does not make sense for an invalid range");
    return ByteLength;
  }
  
  /// Print out the CharSourceRange.  If the locations are in the same buffer
  /// as specified by LastBufferID, then we don't print the filename.  If not,
  /// we do print the filename, and then update LastBufferID with the BufferID
  /// printed.
  void print(raw_ostream &OS, const SourceManager &SM,
             unsigned &LastBufferID, bool PrintText = true) const;

  void print(raw_ostream &OS, const SourceManager &SM,
             bool PrintText = true) const {
    unsigned Tmp = ~0U;
    print(OS, SM, Tmp, PrintText);
  }
  
  LANGUAGE_DEBUG_DUMPER(dump(const SourceManager &SM));
};

} // end namespace language

namespace toolchain {
template <typename T, typename Enable> struct DenseMapInfo;

template <> struct DenseMapInfo<language::SourceLoc> {
  static language::SourceLoc getEmptyKey() {
    return language::SourceLoc(
        SMLoc::getFromPointer(DenseMapInfo<const char *>::getEmptyKey()));
  }

  static language::SourceLoc getTombstoneKey() {
    // Make this different from empty key. See for context:
    // http://lists.toolchain.org/pipermail/toolchain-dev/2015-July/088744.html
    return language::SourceLoc(
        SMLoc::getFromPointer(DenseMapInfo<const char *>::getTombstoneKey()));
  }

  static unsigned getHashValue(const language::SourceLoc &Val) {
    return DenseMapInfo<const void *>::getHashValue(
        Val.getOpaquePointerValue());
  }

  static bool isEqual(const language::SourceLoc &LHS,
                      const language::SourceLoc &RHS) {
    return LHS == RHS;
  }
};

template <> struct DenseMapInfo<language::SourceRange> {
  static language::SourceRange getEmptyKey() {
    return language::SourceRange(language::SourceLoc(
        SMLoc::getFromPointer(DenseMapInfo<const char *>::getEmptyKey())));
  }

  static language::SourceRange getTombstoneKey() {
    // Make this different from empty key. See for context:
    // http://lists.toolchain.org/pipermail/toolchain-dev/2015-July/088744.html
    return language::SourceRange(language::SourceLoc(
        SMLoc::getFromPointer(DenseMapInfo<const char *>::getTombstoneKey())));
  }

  static unsigned getHashValue(const language::SourceRange &Val) {
    return hash_combine(Val.Start.getOpaquePointerValue(),
                        Val.End.getOpaquePointerValue());
  }

  static bool isEqual(const language::SourceRange &LHS,
                      const language::SourceRange &RHS) {
    return LHS == RHS;
  }
};
} // namespace toolchain

#endif // LANGUAGE_BASIC_SOURCELOC_H
