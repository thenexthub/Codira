//===-- language/Compability/Common/type-kinds.h -----------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_COMMON_TYPE_KINDS_H_
#define LANGUAGE_COMPABILITY_COMMON_TYPE_KINDS_H_

#include "Fortran-consts.h"
#include "real.h"
#include <cinttypes>

namespace language::Compability::common {

static constexpr int maxKind{16};

// A predicate that is true when a kind value is a kind that could possibly
// be supported for an intrinsic type category on some target instruction
// set architecture.
static constexpr bool IsValidKindOfIntrinsicType(
    TypeCategory category, std::int64_t kind) {
  switch (category) {
  case TypeCategory::Integer:
  case TypeCategory::Unsigned:
    return kind == 1 || kind == 2 || kind == 4 || kind == 8 || kind == 16;
  case TypeCategory::Real:
  case TypeCategory::Complex:
    return kind == 2 || kind == 3 || kind == 4 || kind == 8 || kind == 10 ||
        kind == 16;
  case TypeCategory::Character:
    return kind == 1 || kind == 2 || kind == 4;
  case TypeCategory::Logical:
    return kind == 1 || kind == 2 || kind == 4 || kind == 8;
  default:
    return false;
  }
}

static constexpr int TypeSizeInBytes(TypeCategory category, std::int64_t kind) {
  if (IsValidKindOfIntrinsicType(category, kind)) {
    if (category == TypeCategory::Real || category == TypeCategory::Complex) {
      int precision{PrecisionOfRealKind(kind)};
      int bits{BitsForBinaryPrecision(precision)};
      if (bits == 80) { // x87 is stored in 16-byte containers
        bits = 128;
      }
      if (category == TypeCategory::Complex) {
        bits *= 2;
      }
      return bits >> 3;
    } else {
      return kind;
    }
  } else {
    return -1;
  }
}

} // namespace language::Compability::common
#endif // FORTRAN_COMMON_TYPE_KINDS_H_
