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
// <memory>

// template <class Alloc>
// struct allocator_traits
// {
//     static constexpr pointer allocate(allocator_type& a, size_type n);
//     ...
// };

// UNSUPPORTED: c++17

#include <uscl/std/__memory_>
#include <uscl/std/cstddef>

template <class T>
struct A
{
  typedef T value_type;
  value_type* allocate(cuda::std::size_t n);
  value_type* allocate(cuda::std::size_t n, const void* p);
};

void f()
{
  A<int> a;
  cuda::std::allocator_traits<A<int>>::allocate(a, 10); // expected-warning {{ignoring return value of function declared
                                                        // with 'nodiscard' attribute}}
  cuda::std::allocator_traits<A<int>>::allocate(a, 10, nullptr); // expected-warning {{ignoring return value of function
                                                                 // declared with 'nodiscard' attribute}}
}
