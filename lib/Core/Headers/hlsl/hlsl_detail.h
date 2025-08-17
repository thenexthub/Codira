//===----- hlsl_detail.h - HLSL definitions for intrinsics ----------------===//
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

#ifndef _HLSL_HLSL_DETAILS_H_
#define _HLSL_HLSL_DETAILS_H_

namespace hlsl {

namespace __detail {

template <typename T, typename U> struct is_same {
  static const bool value = false;
};

template <typename T> struct is_same<T, T> {
  static const bool value = true;
};

template <bool B, typename T> struct enable_if {};

template <typename T> struct enable_if<true, T> {
  using Type = T;
};

template <bool B, class T = void>
using enable_if_t = typename enable_if<B, T>::Type;

template <typename U, typename T, int N>
constexpr enable_if_t<sizeof(U) == sizeof(T), vector<U, N>>
bit_cast(vector<T, N> V) {
  return __builtin_bit_cast(vector<U, N>, V);
}

template <typename U, typename T>
constexpr enable_if_t<sizeof(U) == sizeof(T), U> bit_cast(T F) {
  return __builtin_bit_cast(U, F);
}

template <typename T> struct is_arithmetic {
  static const bool Value = __is_arithmetic(T);
};

template <typename T, int N>
using HLSL_FIXED_VECTOR =
    vector<__detail::enable_if_t<(N > 1 && N <= 4), T>, N>;

} // namespace __detail
} // namespace hlsl
#endif //_HLSL_HLSL_DETAILS_H_
