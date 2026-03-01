/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef CODIRA_UTILS_UNICODE
#define CODIRA_UTILS_UNICODE

#include <cstdint>
#include <cstring>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace Codira::Unicode {
using UTF8 = unsigned char;
using UTF16 = unsigned short;
using UTF32 = unsigned;

/**
 * UTF conversion result type
 */
enum class ConversionResult { OK, SOURCE_EXHAUSTED, SOURCE_ILLEGAL, TARGET_EXHAUSTED };

/**
 * Helper class, reference to array.
 */
template <class T>
struct ArrayRef {
    ArrayRef(const ArrayRef& other) = default;
    ArrayRef(ArrayRef&& other) = default;
    ArrayRef& operator=(const ArrayRef& other) = default;
    ArrayRef& operator=(ArrayRef&& other) = default;

    template <int Length>
    constexpr ArrayRef(T (&arr)[Length]) : beginv{arr}, sz{Length}
    {
    }
    
    constexpr ArrayRef(const T* v, int s): beginv{v}, sz{s} {}
    ArrayRef(const std::vector<T>& v) : beginv{v.data()}, sz{static_cast<int>(v.size())} {}

    int Size() const { return sz; }
    bool Empty() const { return sz == 0; }

    const T* begin() const { return beginv; }
    const T* end() const { return beginv + sz; }

private:
    const T* beginv;
    int sz;
};

struct StringRef : public ArrayRef<UTF8> {
    StringRef(const StringRef& other) = default;
    StringRef(StringRef&& other) = default;
    StringRef& operator=(const StringRef& other) = default;
    StringRef& operator=(StringRef&& other) = default;

    template <int Length>
    constexpr StringRef(const char (&arr)[Length])
        : ArrayRef{reinterpret_cast<const UTF8*>(arr), static_cast<int>(strlen(arr))}
    {
    }

    constexpr StringRef(const UTF8* v, int s) : ArrayRef{v, s} {}
    explicit StringRef(const std::vector<UTF8>& v) : ArrayRef{v} {}
    
    StringRef(const std::string& s) : ArrayRef{reinterpret_cast<const UTF8*>(s.data()), static_cast<int>(s.size())} {}
    StringRef(std::string_view s) : ArrayRef{reinterpret_cast<const UTF8*>(s.data()), static_cast<int>(s.size())} {}

    std::vector<UTF32> ToUTF32() const;
};

/**
 * Converts a UTF8 sequence to UTF32.
 */
ConversionResult ConvertUTF8toUTF32(
    const UTF8** sourceStart, const UTF8* sourceEnd, UTF32** targetStart, UTF32* targetEnd);

constexpr int UNI_MAX_UTF8_BYTES_PER_CODE_POINT{4};

/**
 * Read one Unicode character from \p sourceStart. This function asserts a Unicode codepoint is successfully read.
 * \p sourceStart is advanced to tell the caller how many chars are consumed.
 */
UTF32 ReadOneUnicodeChar(const UTF8** sourceStart, const UTF8* sourceEnd);

/**
 * Convert an Unicode code point to UTF8 sequence.
 * \param Source a Unicode code point.
 * \param [in,out] ResultPtr pointer to the output buffer, needs to be at least \c UNI_MAX_UTF8_BYTES_PER_CODE_POINT
 * bytes. On success \c ResultPtr is updated one past end of the converted sequence.
 * \returns true on success.
 */
bool ConvertCodepointToUTF8(UTF32 codepoint, char*& resultPtr);

///@{
/**
 * Converts a stream of raw bytes assumed to be UTF32 into a UTF8 std::string.
 *
 * \param [in] SrcBytes A buffer of what is assumed to be UTF-32 encoded text.
 * \param [out] Out Converted UTF-8 is stored here on success.
 * \returns true on success
 */
bool ConvertUTF32ToUTF8String(ArrayRef<char> srcBytes, std::string& res);
bool ConvertUTF32ToUTF8String(ArrayRef<UTF32> src, std::string& res);
///@}

bool IsASCII(UTF32 v);
/**
 * \returns true if character \p c falls in ascii and can be used as the first character of an identifier
 */
bool IsASCIIIdStart(UTF32 c);
/**
 * \returns true if character \p c falls in ascii and can be used as a following character of an identifier
 */
bool IsASCIIIdContinue(UTF32 c);
/**
 * \returns true if character \p c falls in Unicode XID_Start. This includes ASCII letters and excludes underscore.
 */
bool IsXIDStart(UTF32 c);
/**
 * \returns true if character \p c falls in Unicode XID_Start. This includes ASCII letters and underscore.
 */
bool IsCODEXIDStart(UTF32 c);
/**
 * \returns true if character \p c falls in Unicode XID_Continue. This includes ASCII letters, digits, and underscore.
 */
bool IsXIDContinue(UTF32 c);

/**
 * \returns true if character \p v is an alphanumeric character.
 */
 bool IsAlnum(int v);

/**
 * \returns true if character \p v is a digit (0-9).
 */
bool IsDigit(int v);

/**
 * \returns true if character \p v is a hexadecimal digit (0-9, a-f, A-F).
 */
bool IsXDigit(int v);

/**
 * Result enum for normalisation form C quick check
 */
enum class NfcQcResult {
    YES,
    NO,
    MAYBE,
};

/**
 * Get canonical combining class value of codepoint \p c. The value can be used either as an enum or an integer. See
 * Unicode 15.0.0 3.11 D104-D106.
 */
uint_fast8_t GetCanonicalCombiningClass(UTF32 c);

/**
 * Quick check whether a string conforms to NFC.
 * \returns MAYBE indicates a normalisation has to be done to further know if \p s is in normalised from.
 */
NfcQcResult NfcQuickCheck(const std::string& s);

/**
 * Convert \p s into NFD.
 */
std::string CanonicalDecompose(const std::string& s);

///@{
/*
 * Convert \p s into NFC.
 */
std::string CanonicalRecompose(const std::string& s);
std::string CanonicalRecompose(const UTF8* begin, const UTF8* end);
///@}

/// Get the Display width of character
/// \param isCODEK: indicates whether the codepoint is in CODEK context, that is, ambiguous width characters are treated as
/// double width; otherwise, they're treated as single width.
int LookupWidth(UTF32 cp, bool isCODEK);

/**
 Check whether a string conforms to NFC.
 */
bool IsNFC(const std::string& s);

/**
 * Converts a unicode string to Normalisation Form C.
 * \param [in,out] s the string to convert. If \p s is already in NFC form, no conversion is performed (which is faster
 * than \p CanonicalRecompose).
 */
void NFC(std::string& s);

/**
 * Decompose Hanguls.
 * Exposed only for testing.
 */
void DecomposeHangul(UTF32 c, std::function<void(UTF32)>&& emitChar);
std::optional<UTF32> ComposeHangul(UTF32 a, UTF32 b);

/// Returns Unicode tr11 based width of \p codepoint, or 0 if \p codepoint is a control character.
/// If \p isCODEK is true, ambiguous width characters are treated as double width; otherwise they are treated as single
/// width.
int SingleCharWidth(UTF32 codepoint, bool isCODEK = false);

/// Returns displayd width in columns of string \p s, based on Unicode tr11.
/// Ambiguous width chars are treates as double width if \p isCODEK is true; otherwise they are treated as single width.
int StrWidth(StringRef s, bool isCODEK = false);

///@{
/// Returns display width of string \p s when used in codec diagnostic emitter. That is, the length of 0x09 is 4;
/// otherwise the result equals to that of \ref StrWidth or \ref SingleCharWidth.
int DisplayWidth(StringRef s, bool isCODEK = false);
int DisplayWidth(UTF32 cp, bool isCODEK = false);
///@}

struct UnicodeCharRange {
    UTF32 lower;
    UTF32 upper;

    friend inline bool operator<(UTF32 value, const UnicodeCharRange& range)
    {
        return value < range.lower;
    }
    friend inline bool operator<(const UnicodeCharRange& range, UTF32 value)
    {
        return range.upper < value;
    }
};

/**
 * Helper class that checks whether a give unicode codepoint is in the specific range.
 */
struct UnicodeCharSet {
    const UnicodeCharRange* ranges;
    int size;

    template <int Size>
    explicit constexpr UnicodeCharSet(const UnicodeCharRange (&r)[Size]) noexcept : ranges{r}, size{Size}
    {
    }

    bool Contains(UTF32 c) const;
};

/**
 * Returns whether this unicode is legal, including Unicode Cs, that is, [0, 0xd7ff] U [0xe000, 0x10ffff].
 */
bool IsLegalUnicode(UTF32 c);
} // namespace Codira::Unicode
#endif
