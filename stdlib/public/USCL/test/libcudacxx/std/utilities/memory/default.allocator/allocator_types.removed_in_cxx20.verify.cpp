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

// Check that the following nested types are removed in C++20:

// template <class T>
// class allocator
// {
// public:
//     typedef T*                                           pointer;
//     typedef const T*                                     const_pointer;
//     typedef typename add_lvalue_reference<T>::type       reference;
//     typedef typename add_lvalue_reference<const T>::type const_reference;
//
//     template <class U> struct rebind {typedef allocator<U> other;};
// ...
// };

// UNSUPPORTED: c++17

#include <uscl/std/__memory_>

template <typename T>
__host__ __device__ void check()
{
  typedef typename cuda::std::allocator<T>::pointer AP; // expected-error 3 {{no type named 'pointer'}}
  typedef typename cuda::std::allocator<T>::const_pointer ACP; // expected-error 3 {{no type named 'const_pointer'}}
  typedef typename cuda::std::allocator<T>::reference AR; // expected-error 3 {{no type named 'reference'}}
  typedef typename cuda::std::allocator<T>::const_reference ACR; // expected-error 3 {{no type named 'const_reference'}}
  typedef typename cuda::std::allocator<T>::template rebind<int>::other ARO; // expected-error 3 {{no member named
                                                                             // 'rebind'}}
}

__host__ __device__ void f()
{
  check<char>();
  check<char const>();
  check<void>();
}

int main(int, char**)
{
  return 0;
}
