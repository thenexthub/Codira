//===- toolchain/ADT/StringExtras.h - Useful string functions --------*- C++ -*-===//
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
// This file contains some functions that are useful when dealing with strings.
//
//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_ADT_STRINGEXTRAS_H
#define TOOLCHAIN_ADT_STRINGEXTRAS_H

#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringRef.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <string>
#include <utility>

inline namespace __language { inline namespace __runtime {
namespace toolchain {

template<typename T> class SmallVectorImpl;
class raw_ostream;

/// hexdigit - Return the hexadecimal character for the
/// given number \p X (which should be less than 16).
inline char hexdigit(unsigned X, bool LowerCase = false) {
  const char HexChar = LowerCase ? 'a' : 'A';
  return X < 10 ? '0' + X : HexChar + X - 10;
}

/// Given an array of c-style strings terminated by a null pointer, construct
/// a vector of StringRefs representing the same strings without the terminating
/// null string.
inline std::vector<StringRef> toStringRefArray(const char *const *Strings) {
  std::vector<StringRef> Result;
  while (*Strings)
    Result.push_back(*Strings++);
  return Result;
}

/// Construct a string ref from a boolean.
inline StringRef toStringRef(bool B) { return StringRef(B ? "true" : "false"); }

/// Construct a string ref from an array ref of unsigned chars.
inline StringRef toStringRef(ArrayRef<uint8_t> Input) {
  return StringRef(reinterpret_cast<const char *>(Input.begin()), Input.size());
}

/// Construct a string ref from an array ref of unsigned chars.
inline ArrayRef<uint8_t> arrayRefFromStringRef(StringRef Input) {
  return {Input.bytes_begin(), Input.bytes_end()};
}

/// Interpret the given character \p C as a hexadecimal digit and return its
/// value.
///
/// If \p C is not a valid hex digit, -1U is returned.
inline unsigned hexDigitValue(char C) {
  struct HexTable {
    unsigned LUT[255] = {};
    constexpr HexTable() {
      // Default initialize everything to invalid.
      for (int i = 0; i < 255; ++i)
        LUT[i] = ~0U;
      // Initialize `0`-`9`.
      for (int i = 0; i < 10; ++i)
        LUT['0' + i] = i;
      // Initialize `A`-`F` and `a`-`f`.
      for (int i = 0; i < 6; ++i)
        LUT['A' + i] = LUT['a' + i] = 10 + i;
    }
  };
  constexpr HexTable Table;
  return Table.LUT[static_cast<unsigned char>(C)];
}

/// Checks if character \p C is one of the 10 decimal digits.
inline bool isDigit(char C) { return C >= '0' && C <= '9'; }

/// Checks if character \p C is a hexadecimal numeric character.
inline bool isHexDigit(char C) { return hexDigitValue(C) != ~0U; }

/// Checks if character \p C is a valid letter as classified by "C" locale.
inline bool isAlpha(char C) {
  return ('a' <= C && C <= 'z') || ('A' <= C && C <= 'Z');
}

/// Checks whether character \p C is either a decimal digit or an uppercase or
/// lowercase letter as classified by "C" locale.
inline bool isAlnum(char C) { return isAlpha(C) || isDigit(C); }

/// Checks whether character \p C is valid ASCII (high bit is zero).
inline bool isASCII(char C) { return static_cast<unsigned char>(C) <= 127; }

/// Checks whether all characters in S are ASCII.
inline bool isASCII(toolchain::StringRef S) {
  for (char C : S)
    if (TOOLCHAIN_UNLIKELY(!isASCII(C)))
      return false;
  return true;
}

/// Checks whether character \p C is printable.
///
/// Locale-independent version of the C standard library isprint whose results
/// may differ on different platforms.
inline bool isPrint(char C) {
  unsigned char UC = static_cast<unsigned char>(C);
  return (0x20 <= UC) && (UC <= 0x7E);
}

/// Checks whether character \p C is whitespace in the "C" locale.
///
/// Locale-independent version of the C standard library isspace.
inline bool isSpace(char C) {
  return C == ' ' || C == '\f' || C == '\n' || C == '\r' || C == '\t' ||
         C == '\v';
}

/// Returns the corresponding lowercase character if \p x is uppercase.
inline char toLower(char x) {
  if (x >= 'A' && x <= 'Z')
    return x - 'A' + 'a';
  return x;
}

/// Returns the corresponding uppercase character if \p x is lowercase.
inline char toUpper(char x) {
  if (x >= 'a' && x <= 'z')
    return x - 'a' + 'A';
  return x;
}

inline std::string utohexstr(uint64_t X, bool LowerCase = false) {
  char Buffer[17];
  char *BufPtr = std::end(Buffer);

  if (X == 0) *--BufPtr = '0';

  while (X) {
    unsigned char Mod = static_cast<unsigned char>(X) & 15;
    *--BufPtr = hexdigit(Mod, LowerCase);
    X >>= 4;
  }

  return std::string(BufPtr, std::end(Buffer));
}

/// Convert buffer \p Input to its hexadecimal representation.
/// The returned string is double the size of \p Input.
inline std::string toHex(StringRef Input, bool LowerCase = false) {
  static const char *const LUT = "0123456789ABCDEF";
  const uint8_t Offset = LowerCase ? 32 : 0;
  size_t Length = Input.size();

  std::string Output;
  Output.reserve(2 * Length);
  for (size_t i = 0; i < Length; ++i) {
    const unsigned char c = Input[i];
    Output.push_back(LUT[c >> 4] | Offset);
    Output.push_back(LUT[c & 15] | Offset);
  }
  return Output;
}

inline std::string toHex(ArrayRef<uint8_t> Input, bool LowerCase = false) {
  return toHex(toStringRef(Input), LowerCase);
}

/// Store the binary representation of the two provided values, \p MSB and
/// \p LSB, that make up the nibbles of a hexadecimal digit. If \p MSB or \p LSB
/// do not correspond to proper nibbles of a hexadecimal digit, this method
/// returns false. Otherwise, returns true.
inline bool tryGetHexFromNibbles(char MSB, char LSB, uint8_t &Hex) {
  unsigned U1 = hexDigitValue(MSB);
  unsigned U2 = hexDigitValue(LSB);
  if (U1 == ~0U || U2 == ~0U)
    return false;

  Hex = static_cast<uint8_t>((U1 << 4) | U2);
  return true;
}

/// Return the binary representation of the two provided values, \p MSB and
/// \p LSB, that make up the nibbles of a hexadecimal digit.
inline uint8_t hexFromNibbles(char MSB, char LSB) {
  uint8_t Hex = 0;
  bool GotHex = tryGetHexFromNibbles(MSB, LSB, Hex);
  (void)GotHex;
  assert(GotHex && "MSB and/or LSB do not correspond to hex digits");
  return Hex;
}

/// Convert hexadecimal string \p Input to its binary representation and store
/// the result in \p Output. Returns true if the binary representation could be
/// converted from the hexadecimal string. Returns false if \p Input contains
/// non-hexadecimal digits. The output string is half the size of \p Input.
inline bool tryGetFromHex(StringRef Input, std::string &Output) {
  if (Input.empty())
    return true;

  Output.reserve((Input.size() + 1) / 2);
  if (Input.size() % 2 == 1) {
    uint8_t Hex = 0;
    if (!tryGetHexFromNibbles('0', Input.front(), Hex))
      return false;

    Output.push_back(Hex);
    Input = Input.drop_front();
  }

  assert(Input.size() % 2 == 0);
  while (!Input.empty()) {
    uint8_t Hex = 0;
    if (!tryGetHexFromNibbles(Input[0], Input[1], Hex))
      return false;

    Output.push_back(Hex);
    Input = Input.drop_front(2);
  }
  return true;
}

/// Convert hexadecimal string \p Input to its binary representation.
/// The return string is half the size of \p Input.
inline std::string fromHex(StringRef Input) {
  std::string Hex;
  bool GotHex = tryGetFromHex(Input, Hex);
  (void)GotHex;
  assert(GotHex && "Input contains non hex digits");
  return Hex;
}

inline std::string utostr(uint64_t X, bool isNeg = false) {
  char Buffer[21];
  char *BufPtr = std::end(Buffer);

  if (X == 0) *--BufPtr = '0';  // Handle special case...

  while (X) {
    *--BufPtr = '0' + char(X % 10);
    X /= 10;
  }

  if (isNeg) *--BufPtr = '-';   // Add negative sign...
  return std::string(BufPtr, std::end(Buffer));
}

inline std::string itostr(int64_t X) {
  if (X < 0)
    return utostr(static_cast<uint64_t>(1) + ~static_cast<uint64_t>(X), true);
  else
    return utostr(static_cast<uint64_t>(X));
}

/// Returns the English suffix for an ordinal integer (-st, -nd, -rd, -th).
inline StringRef getOrdinalSuffix(unsigned Val) {
  // It is critically important that we do this perfectly for
  // user-written sequences with over 100 elements.
  switch (Val % 100) {
  case 11:
  case 12:
  case 13:
    return "th";
  default:
    switch (Val % 10) {
      case 1: return "st";
      case 2: return "nd";
      case 3: return "rd";
      default: return "th";
    }
  }
}

namespace detail {

template <typename IteratorT>
inline std::string join_impl(IteratorT Begin, IteratorT End,
                             StringRef Separator, std::input_iterator_tag) {
  std::string S;
  if (Begin == End)
    return S;

  S += (*Begin);
  while (++Begin != End) {
    S += Separator;
    S += (*Begin);
  }
  return S;
}

template <typename IteratorT>
inline std::string join_impl(IteratorT Begin, IteratorT End,
                             StringRef Separator, std::forward_iterator_tag) {
  std::string S;
  if (Begin == End)
    return S;

  size_t Len = (std::distance(Begin, End) - 1) * Separator.size();
  for (IteratorT I = Begin; I != End; ++I)
    Len += (*I).size();
  S.reserve(Len);
  size_t PrevCapacity = S.capacity();
  (void)PrevCapacity;
  S += (*Begin);
  while (++Begin != End) {
    S += Separator;
    S += (*Begin);
  }
  assert(PrevCapacity == S.capacity() && "String grew during building");
  return S;
}

template <typename Sep>
inline void join_items_impl(std::string &Result, Sep Separator) {}

template <typename Sep, typename Arg>
inline void join_items_impl(std::string &Result, Sep Separator,
                            const Arg &Item) {
  Result += Item;
}

template <typename Sep, typename Arg1, typename... Args>
inline void join_items_impl(std::string &Result, Sep Separator, const Arg1 &A1,
                            Args &&... Items) {
  Result += A1;
  Result += Separator;
  join_items_impl(Result, Separator, std::forward<Args>(Items)...);
}

inline size_t join_one_item_size(char) { return 1; }
inline size_t join_one_item_size(const char *S) { return S ? ::strlen(S) : 0; }

template <typename T> inline size_t join_one_item_size(const T &Str) {
  return Str.size();
}

inline size_t join_items_size() { return 0; }

template <typename A1> inline size_t join_items_size(const A1 &A) {
  return join_one_item_size(A);
}
template <typename A1, typename... Args>
inline size_t join_items_size(const A1 &A, Args &&... Items) {
  return join_one_item_size(A) + join_items_size(std::forward<Args>(Items)...);
}

} // end namespace detail

/// Joins the strings in the range [Begin, End), adding Separator between
/// the elements.
template <typename IteratorT>
inline std::string join(IteratorT Begin, IteratorT End, StringRef Separator) {
  using tag = typename std::iterator_traits<IteratorT>::iterator_category;
  return detail::join_impl(Begin, End, Separator, tag());
}

/// Joins the strings in the range [R.begin(), R.end()), adding Separator
/// between the elements.
template <typename Range>
inline std::string join(Range &&R, StringRef Separator) {
  return join(R.begin(), R.end(), Separator);
}

/// Joins the strings in the parameter pack \p Items, adding \p Separator
/// between the elements.  All arguments must be implicitly convertible to
/// std::string, or there should be an overload of std::string::operator+=()
/// that accepts the argument explicitly.
template <typename Sep, typename... Args>
inline std::string join_items(Sep Separator, Args &&... Items) {
  std::string Result;
  if (sizeof...(Items) == 0)
    return Result;

  size_t NS = detail::join_one_item_size(Separator);
  size_t NI = detail::join_items_size(std::forward<Args>(Items)...);
  Result.reserve(NI + (sizeof...(Items) - 1) * NS + 1);
  detail::join_items_impl(Result, Separator, std::forward<Args>(Items)...);
  return Result;
}

/// A helper class to return the specified delimiter string after the first
/// invocation of operator StringRef().  Used to generate a comma-separated
/// list from a loop like so:
///
/// \code
///   ListSeparator LS;
///   for (auto &I : C)
///     OS << LS << I.getName();
/// \end
class ListSeparator {
  bool First = true;
  StringRef Separator;

public:
  ListSeparator(StringRef Separator = ", ") : Separator(Separator) {}
  operator StringRef() {
    if (First) {
      First = false;
      return {};
    }
    return Separator;
  }
};

} // end namespace toolchain
}} // namespace language::runtime

#endif // TOOLCHAIN_ADT_STRINGEXTRAS_H
