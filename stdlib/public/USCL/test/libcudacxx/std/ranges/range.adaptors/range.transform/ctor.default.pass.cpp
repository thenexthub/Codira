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

// transform_view() requires cuda::std::default_initializable<V> &&
//                           cuda::std::default_initializable<F> = default;

#include <uscl/std/cassert>
#include <uscl/std/ranges>
#include <uscl/std/type_traits>

constexpr int buff[] = {1, 2, 3};

struct DefaultConstructibleView : cuda::std::ranges::view_base
{
  __host__ __device__ constexpr DefaultConstructibleView() noexcept
      : begin_(buff)
      , end_(buff + 3)
  {}
  __host__ __device__ constexpr int const* begin() const
  {
    return begin_;
  }
  __host__ __device__ constexpr int const* end() const
  {
    return end_;
  }

private:
  int const* begin_;
  int const* end_;
};

struct DefaultConstructibleFunction
{
  int state_;
  __host__ __device__ constexpr DefaultConstructibleFunction() noexcept
      : state_(100)
  {}
  __host__ __device__ constexpr int operator()(int i) const
  {
    return i + state_;
  }
};

struct NoDefaultCtrView : cuda::std::ranges::view_base
{
  NoDefaultCtrView() = delete;
  __host__ __device__ int* begin() const;
  __host__ __device__ int* end() const;
};

struct NoDefaultFunction
{
  NoDefaultFunction() = delete;
  __host__ __device__ constexpr int operator()(int i) const
  {
    return i;
  };
};

__host__ __device__ constexpr bool test()
{
  {
    cuda::std::ranges::transform_view<DefaultConstructibleView, DefaultConstructibleFunction> view{};
    assert(view.size() == 3);
    assert(view[0] == 101);
    assert(view[1] == 102);
    assert(view[2] == 103);
  }

  {
    cuda::std::ranges::transform_view<DefaultConstructibleView, DefaultConstructibleFunction> view = {};
    assert(view.size() == 3);
    assert(view[0] == 101);
    assert(view[1] == 102);
    assert(view[2] == 103);
  }

  static_assert(!cuda::std::is_default_constructible_v<
                cuda::std::ranges::transform_view<NoDefaultCtrView, DefaultConstructibleFunction>>);
  static_assert(!cuda::std::is_default_constructible_v<
                cuda::std::ranges::transform_view<DefaultConstructibleView, NoDefaultFunction>>);
  static_assert(
    !cuda::std::is_default_constructible_v<cuda::std::ranges::transform_view<NoDefaultCtrView, NoDefaultFunction>>);

  return true;
}

int main(int, char**)
{
  test();
#if defined(_CCCL_BUILTIN_ADDRESSOF)
  static_assert(test(), "");
#endif // _CCCL_BUILTIN_ADDRESSOF

  return 0;
}
