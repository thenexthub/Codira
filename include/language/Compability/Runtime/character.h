//===-- language/Compability/Runtime/character.h -----------------------*- C++ -*-===//
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

// Defines API between compiled code and the CHARACTER
// support functions in the runtime library.

#ifndef LANGUAGE_COMPABILITY_RUNTIME_CHARACTER_H_
#define LANGUAGE_COMPABILITY_RUNTIME_CHARACTER_H_
#include "language/Compability/Runtime/entry-names.h"
#include <cstddef>
#include <cstdint>

namespace language::Compability::runtime {

class Descriptor;

template <typename CHAR>
RT_API_ATTRS int CharacterScalarCompare(
    const CHAR *x, const CHAR *y, std::size_t xChars, std::size_t yChars);
extern template RT_API_ATTRS int CharacterScalarCompare<char>(
    const char *x, const char *y, std::size_t xChars, std::size_t yChars);
extern template RT_API_ATTRS int CharacterScalarCompare<char16_t>(
    const char16_t *x, const char16_t *y, std::size_t xChars,
    std::size_t yChars);
extern template RT_API_ATTRS int CharacterScalarCompare<char32_t>(
    const char32_t *x, const char32_t *y, std::size_t xChars,
    std::size_t yChars);

extern "C" {

// Appends the corresponding (or expanded) characters of 'operand'
// to the (elements of) a (re)allocation of 'accumulator', which must be an
// initialized CHARACTER allocatable scalar or array descriptor -- use
// AllocatableInitCharacter() to set one up.  Crashes when not
// conforming.  Assumes independence of data.
void RTDECL(CharacterConcatenate)(Descriptor &accumulator,
    const Descriptor &from, const char *sourceFile = nullptr,
    int sourceLine = 0);

// Convenience specialization for ASCII scalars concatenation.
void RTDECL(CharacterConcatenateScalar1)(
    Descriptor &accumulator, const char *from, std::size_t chars);

// CHARACTER comparisons.  The kinds must match.  Like std::memcmp(),
// the result is less than zero, zero, or greater than zero if the first
// argument is less than the second, equal to the second, or greater than
// the second, respectively.  The shorter argument is treated as if it were
// padded on the right with blanks.
// N.B.: Calls to the restricted specific intrinsic functions LGE, LGT, LLE,
// & LLT are converted into calls to these during lowering; they don't have
// to be able to be passed as actual procedure arguments.
int RTDECL(CharacterCompareScalar)(const Descriptor &, const Descriptor &);
int RTDECL(CharacterCompareScalar1)(
    const char *x, const char *y, std::size_t xChars, std::size_t yChars);
int RTDECL(CharacterCompareScalar2)(const char16_t *x, const char16_t *y,
    std::size_t xChars, std::size_t yChars);
int RTDECL(CharacterCompareScalar4)(const char32_t *x, const char32_t *y,
    std::size_t xChars, std::size_t yChars);

// General CHARACTER comparison; the result is a LOGICAL(KIND=1) array that
// is established and populated.
void RTDECL(CharacterCompare)(
    Descriptor &result, const Descriptor &, const Descriptor &);

// Special-case support for optimized ASCII scalar expressions.

// Copies data from 'rhs' to the remaining space (lhsLength - offset)
// in 'lhs', if any.  Returns the new offset.  Assumes independence.
std::size_t RTDECL(CharacterAppend1)(char *lhs, std::size_t lhsBytes,
    std::size_t offset, const char *rhs, std::size_t rhsBytes);

// Appends any necessary spaces to a CHARACTER(KIND=1) scalar.
void RTDECL(CharacterPad1)(char *lhs, std::size_t bytes, std::size_t offset);

// Intrinsic functions
// The result descriptors below are all established by the runtime.
void RTDECL(Adjustl)(Descriptor &result, const Descriptor &,
    const char *sourceFile = nullptr, int sourceLine = 0);
void RTDECL(Adjustr)(Descriptor &result, const Descriptor &,
    const char *sourceFile = nullptr, int sourceLine = 0);
std::size_t RTDECL(LenTrim1)(const char *, std::size_t);
std::size_t RTDECL(LenTrim2)(const char16_t *, std::size_t);
std::size_t RTDECL(LenTrim4)(const char32_t *, std::size_t);
void RTDECL(LenTrim)(Descriptor &result, const Descriptor &, int kind,
    const char *sourceFile = nullptr, int sourceLine = 0);
void RTDECL(Repeat)(Descriptor &result, const Descriptor &string,
    std::int64_t ncopies, const char *sourceFile = nullptr, int sourceLine = 0);
void RTDECL(Trim)(Descriptor &result, const Descriptor &string,
    const char *sourceFile = nullptr, int sourceLine = 0);

void RTDECL(CharacterMax)(Descriptor &accumulator, const Descriptor &x,
    const char *sourceFile = nullptr, int sourceLine = 0);
void RTDECL(CharacterMin)(Descriptor &accumulator, const Descriptor &x,
    const char *sourceFile = nullptr, int sourceLine = 0);

std::size_t RTDECL(Index1)(const char *, std::size_t, const char *substring,
    std::size_t, bool back = false);
std::size_t RTDECL(Index2)(const char16_t *, std::size_t,
    const char16_t *substring, std::size_t, bool back = false);
std::size_t RTDECL(Index4)(const char32_t *, std::size_t,
    const char32_t *substring, std::size_t, bool back = false);
void RTDECL(Index)(Descriptor &result, const Descriptor &string,
    const Descriptor &substring, const Descriptor *back /*can be null*/,
    int kind, const char *sourceFile = nullptr, int sourceLine = 0);

std::size_t RTDECL(Scan1)(
    const char *, std::size_t, const char *set, std::size_t, bool back = false);
std::size_t RTDECL(Scan2)(const char16_t *, std::size_t, const char16_t *set,
    std::size_t, bool back = false);
std::size_t RTDECL(Scan4)(const char32_t *, std::size_t, const char32_t *set,
    std::size_t, bool back = false);
void RTDECL(Scan)(Descriptor &result, const Descriptor &string,
    const Descriptor &set, const Descriptor *back /*can be null*/, int kind,
    const char *sourceFile = nullptr, int sourceLine = 0);

std::size_t RTDECL(Verify1)(
    const char *, std::size_t, const char *set, std::size_t, bool back = false);
std::size_t RTDECL(Verify2)(const char16_t *, std::size_t, const char16_t *set,
    std::size_t, bool back = false);
std::size_t RTDECL(Verify4)(const char32_t *, std::size_t, const char32_t *set,
    std::size_t, bool back = false);
void RTDECL(Verify)(Descriptor &result, const Descriptor &string,
    const Descriptor &set, const Descriptor *back /*can be null*/, int kind,
    const char *sourceFile = nullptr, int sourceLine = 0);
}
} // namespace language::Compability::runtime
#endif // FORTRAN_RUNTIME_CHARACTER_H_
