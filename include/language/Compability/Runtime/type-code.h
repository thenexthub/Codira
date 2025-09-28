//===-- language/Compability-rt/runtime/type-code.h --------------------*- C++ -*-===//
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

#ifndef FLANG_RT_RUNTIME_TYPE_CODE_H_
#define FLANG_RT_RUNTIME_TYPE_CODE_H_

#include "language/Compability/Common/Fortran-consts.h"
#include "language/Compability/Common/ISO_Fortran_binding_wrapper.h"
#include "language/Compability/Common/optional.h"
#include <utility>

namespace language::Compability::runtime {

using common::TypeCategory;

class TypeCode {
public:
  TypeCode() {}
  explicit RT_API_ATTRS TypeCode(ISO::CFI_type_t t) : raw_{t} {}
  RT_API_ATTRS TypeCode(TypeCategory, int kind);

  RT_API_ATTRS int raw() const { return raw_; }

  constexpr RT_API_ATTRS bool IsValid() const {
    return raw_ >= CFI_type_signed_char && raw_ <= CFI_TYPE_LAST;
  }
  constexpr RT_API_ATTRS bool IsInteger() const {
    return raw_ >= CFI_type_signed_char && raw_ <= CFI_type_ptrdiff_t;
  }
  constexpr RT_API_ATTRS bool IsReal() const {
    return raw_ >= CFI_type_half_float && raw_ <= CFI_type_float128;
  }
  constexpr RT_API_ATTRS bool IsComplex() const {
    return raw_ >= CFI_type_half_float_Complex &&
        raw_ <= CFI_type_float128_Complex;
  }
  constexpr RT_API_ATTRS bool IsCharacter() const {
    return raw_ == CFI_type_char || raw_ == CFI_type_char16_t ||
        raw_ == CFI_type_char32_t;
  }
  constexpr RT_API_ATTRS bool IsLogical() const {
    return raw_ == CFI_type_Bool ||
        (raw_ >= CFI_type_int_least8_t && raw_ <= CFI_type_int_least64_t);
  }
  constexpr RT_API_ATTRS bool IsDerived() const {
    return raw_ == CFI_type_struct;
  }
  constexpr RT_API_ATTRS bool IsIntrinsic() const {
    return IsValid() && !IsDerived();
  }

  RT_API_ATTRS language::Compability::common::optional<std::pair<TypeCategory, int>>
  GetCategoryAndKind() const;

  RT_API_ATTRS bool operator==(TypeCode that) const {
    if (raw_ == that.raw_) { // fast path
      return true;
    } else {
      // Multiple raw CFI_type_... codes can represent the same Fortran
      // type category + kind type parameter, e.g. CFI_type_int and
      // CFI_type_int32_t.
      auto thisCK{GetCategoryAndKind()};
      auto thatCK{that.GetCategoryAndKind()};
      return thisCK && thatCK && *thisCK == *thatCK;
    }
  }
  RT_API_ATTRS bool operator!=(TypeCode that) const { return !(*this == that); }

private:
  ISO::CFI_type_t raw_{CFI_type_other};
};
} // namespace language::Compability::runtime
#endif // FLANG_RT_RUNTIME_TYPE_CODE_H_
