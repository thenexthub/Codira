/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Tuesday, June 11, 2024.
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
// UNSUPPORTED: c++17, c++20

// <memory>

// template<class Allocator>
// [[nodiscard]] constexpr allocation_result<typename allocator_traits<Allocator>::pointer>
//   allocate_at_least(Allocator& a, size_t n);

#include <uscl/std/__memory_>
#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/cstddef>

// check that cuda::std::allocation_result exists and isn't restricted to pointers
using AllocResult = cuda::std::allocation_result<int>;

template <class T>
struct no_allocate_at_least
{
  using value_type = T;
  T t;

  constexpr T* allocate(cuda::std::size_t)
  {
    return &t;
  }
  constexpr void deallocate(T*, cuda::std::size_t) noexcept {}
};

template <class T>
struct has_allocate_at_least
{
  using value_type = T;
  T t1;
  T t2;

  constexpr T* allocate(cuda::std::size_t)
  {
    return &t1;
  }
  constexpr void deallocate(T*, cuda::std::size_t) noexcept {}
  constexpr cuda::std::allocation_result<T*> allocate_at_least(cuda::std::size_t)
  {
    return {&t2, 2};
  }
};

constexpr bool test()
{
  { // check that cuda::std::allocate_at_least forwards to allocator::allocate if no allocate_at_least exists
    no_allocate_at_least<int> alloc;
    cuda::std::same_as<cuda::std::allocation_result<int*>> decltype(auto) ret = cuda::std::allocate_at_least(alloc, 1);
    assert(ret.count == 1);
    assert(ret.ptr == &alloc.t);
  }

  { // check that cuda::std::allocate_at_least forwards to allocator::allocate_at_least if allocate_at_least exists
    has_allocate_at_least<int> alloc;
    cuda::std::same_as<cuda::std::allocation_result<int*>> decltype(auto) ret = cuda::std::allocate_at_least(alloc, 1);
    assert(ret.count == 2);
    assert(ret.ptr == &alloc.t2);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
