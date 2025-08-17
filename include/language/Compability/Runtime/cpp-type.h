//===-- language/Compability/Runtime/cpp-type.h ------------------------*- C++ -*-===//
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

// Maps Fortran intrinsic types to C++ types used in the runtime.

#ifndef LANGUAGE_COMPABILITY_RUNTIME_CPP_TYPE_H_
#define LANGUAGE_COMPABILITY_RUNTIME_CPP_TYPE_H_

#include "language/Compability/Common/Fortran-consts.h"
#include "language/Compability/Common/float128.h"
#include "language/Compability/Common/float80.h"
#include "language/Compability/Common/uint128.h"
#include "language/Compability/Runtime/complex.h"
#include <cstdint>
#if __cplusplus >= 202302
#include <stdfloat>
#endif
#include <type_traits>

#if !defined HAS_FP16 && __STDCPP_FLOAT16_T__
#define HAS_FP16 1
#endif
#if !defined HAS_BF16 && __STDCPP_BFLOAT16_T__
#define HAS_BF16 1
#endif

namespace language::Compability::runtime {

using common::TypeCategory;

template <TypeCategory CAT, int KIND> struct CppTypeForHelper {
  using type = void;
};
template <TypeCategory CAT, int KIND>
using CppTypeFor = typename CppTypeForHelper<CAT, KIND>::type;

template <TypeCategory CAT, int KIND>
constexpr bool HasCppTypeFor{
    !std::is_void_v<typename CppTypeForHelper<CAT, KIND>::type>};

template <int KIND> struct CppTypeForHelper<TypeCategory::Integer, KIND> {
  using type = common::HostSignedIntType<8 * KIND>;
};

template <int KIND> struct CppTypeForHelper<TypeCategory::Unsigned, KIND> {
  using type = common::HostUnsignedIntType<8 * KIND>;
};

#if HAS_FP16
template <> struct CppTypeForHelper<TypeCategory::Real, 2> {
  using type = std::float16_t;
};
#endif
#if HAS_BF16
template <> struct CppTypeForHelper<TypeCategory::Real, 3> {
  using type = std::bfloat16_t;
};
#endif
template <> struct CppTypeForHelper<TypeCategory::Real, 4> {
#if __STDCPP_FLOAT32_T__
  using type = std::float32_t;
#else
  using type = float;
#endif
};
template <> struct CppTypeForHelper<TypeCategory::Real, 8> {
#if __STDCPP_FLOAT64_T__
  using type = std::float64_t;
#else
  using type = double;
#endif
};
#if HAS_FLOAT80
template <> struct CppTypeForHelper<TypeCategory::Real, 10> {
  using type = CppFloat80Type;
};
#endif
#if __STDCPP_FLOAT128_T__
using CppFloat128Type = std::float128_t;
#elif HAS_LDBL128
using CppFloat128Type = long double;
#elif HAS_FLOAT128
using CppFloat128Type = __float128;
#endif
#if __STDCPP_FLOAT128_t || HAS_LDBL128 || HAS_FLOAT128
template <> struct CppTypeForHelper<TypeCategory::Real, 16> {
  using type = CppFloat128Type;
};
#endif

template <int KIND> struct CppTypeForHelper<TypeCategory::Complex, KIND> {
  using type = rtcmplx::complex<CppTypeFor<TypeCategory::Real, KIND>>;
};

template <> struct CppTypeForHelper<TypeCategory::Character, 1> {
  using type = char;
};
template <> struct CppTypeForHelper<TypeCategory::Character, 2> {
  using type = char16_t;
};
template <> struct CppTypeForHelper<TypeCategory::Character, 4> {
  using type = char32_t;
};

template <int KIND> struct CppTypeForHelper<TypeCategory::Logical, KIND> {
  using type = common::HostSignedIntType<8 * KIND>;
};
template <> struct CppTypeForHelper<TypeCategory::Logical, 1> {
  using type = bool;
};

} // namespace language::Compability::runtime
#endif // FORTRAN_RUNTIME_CPP_TYPE_H_
