//===-- language/Compability/Common/Fortran-consts.h -------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_COMMON_FORTRAN_CONSTS_H_
#define LANGUAGE_COMPABILITY_COMMON_FORTRAN_CONSTS_H_

#include "enum-class.h"
#include <cstdint>

namespace language::Compability::common {

// Fortran has five kinds of standard intrinsic data types, the Unsigned
// extension, and derived types.
ENUM_CLASS(
    TypeCategory, Integer, Unsigned, Real, Complex, Character, Logical, Derived)
ENUM_CLASS(VectorElementCategory, Integer, Unsigned, Real)

ENUM_CLASS(IoStmtKind, None, Backspace, Close, Endfile, Flush, Inquire, Open,
    Print, Read, Rewind, Wait, Write)

// Defined I/O variants
ENUM_CLASS(
    DefinedIo, ReadFormatted, ReadUnformatted, WriteFormatted, WriteUnformatted)

// Fortran arrays may have up to 15 dimensions (See Fortran 2018 section 5.4.6).
static constexpr int maxRank{15};

// Floating-point rounding modes; these are packed into a byte to save
// room in the runtime's format processing context structure.  These
// enumerators are defined with the corresponding values returned from
// toolchain.get.rounding.
enum class RoundingMode : std::uint8_t {
  ToZero, // ROUND=ZERO, RZ - truncation
  TiesToEven, // ROUND=NEAREST, RN - default IEEE rounding
  Up, // ROUND=UP, RU
  Down, // ROUND=DOWN, RD
  TiesAwayFromZero, // ROUND=COMPATIBLE, RC - ties round away from zero
};

} // namespace language::Compability::common
#endif /* FORTRAN_COMMON_FORTRAN_CONSTS_H_ */
