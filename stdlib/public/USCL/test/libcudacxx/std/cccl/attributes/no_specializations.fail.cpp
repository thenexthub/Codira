/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Sunday, April 14, 2024.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 *
 */

#include <uscl/std/__cccl/attributes.h>

#if _CCCL_HAS_ATTRIBUTE_NO_SPECIALIZATIONS()

// 1. Attribute applied to a template class

template <class T>
struct _CCCL_NO_SPECIALIZATIONS Struct
{
  static constexpr bool value = false;
};

// This should fail to compile
template <>
struct Struct<int>
{
  static constexpr bool value = false;
};

// 2. Attribute applied to a template variable

template <class T>
_CCCL_NO_SPECIALIZATIONS inline constexpr bool variable = false;

// This should fail to compile
template <>
inline constexpr bool variable<int> = false;

// 3. Attribute applied to a template function

template <class T>
_CCCL_NO_SPECIALIZATIONS __host__ __device__ T function()
{
  return T{0};
}

// This should fail to compile
template <>
__host__ __device__ int function<int>()
{
  return 1;
}

#else // ^^^ _CCCL_HAS_ATTRIBUTE_NO_SPECIALIZATIONS() ^^^ / vvv !_CCCL_HAS_ATTRIBUTE_NO_SPECIALIZATIONS() vvv

static_assert(false, "no_specializations attribute not supported");

#endif // ^^^ !_CCCL_HAS_ATTRIBUTE_NO_SPECIALIZATIONS() ^^^

int main(int, char**)
{
  return 0;
}
