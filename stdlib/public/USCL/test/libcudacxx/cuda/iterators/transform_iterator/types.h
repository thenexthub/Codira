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

#ifndef TEST_CUDA_TRANSFORM_ITERATOR_TYPES_H
#define TEST_CUDA_TRANSFORM_ITERATOR_TYPES_H

#include <uscl/std/utility>

#include "test_macros.h"

struct TimesTwo
{
  __host__ __device__ constexpr int operator()(int x) const
  {
    return x * 2;
  }
};

struct PlusOneMutable
{
  __host__ __device__ constexpr int operator()(int x)
  {
    return x + 1;
  }
};

struct PlusOne
{
  __host__ __device__ constexpr int operator()(int x) const
  {
    return x + 1;
  }
};

struct Increment
{
  __host__ __device__ constexpr int& operator()(int& x)
  {
    return ++x;
  }
};

struct IncrementRvalueRef
{
  __host__ __device__ constexpr int&& operator()(int& x)
  {
    return cuda::std::move(++x);
  }
};

struct PlusOneNoexcept
{
  __host__ __device__ constexpr int operator()(int x) noexcept
  {
    return x + 1;
  }
};

struct PlusWithMutableMember
{
  int val_ = 0;
  __host__ __device__ constexpr PlusWithMutableMember(const int val) noexcept
      : val_(val)
  {}
  __host__ __device__ constexpr int operator()(int x) noexcept
  {
    return x + val_++;
  }
};

#endif // TEST_CUDA_TRANSFORM_ITERATOR_TYPES_H
