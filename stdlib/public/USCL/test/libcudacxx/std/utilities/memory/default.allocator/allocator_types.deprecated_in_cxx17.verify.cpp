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

// Check that the following nested types are deprecated in C++17:

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

// REQUIRES: c++17

#include <uscl/std/__memory_>

__host__ __device__ void f()
{
  {
    typedef cuda::std::allocator<char>::pointer Pointer; // expected-warning {{'pointer' is deprecated}}
    typedef cuda::std::allocator<char>::const_pointer ConstPointer; // expected-warning {{'const_pointer' is
                                                                    // deprecated}}
    typedef cuda::std::allocator<char>::reference Reference; // expected-warning {{'reference' is deprecated}}
    typedef cuda::std::allocator<char>::const_reference ConstReference; // expected-warning {{'const_reference' is
                                                                        // deprecated}}
    typedef cuda::std::allocator<char>::rebind<int>::other Rebind; // expected-warning {{'rebind<int>' is deprecated}}
  }
  {
    typedef cuda::std::allocator<char const>::pointer Pointer; // expected-warning {{'pointer' is deprecated}}
    typedef cuda::std::allocator<char const>::const_pointer ConstPointer; // expected-warning {{'const_pointer' is
                                                                          // deprecated}}
    typedef cuda::std::allocator<char const>::reference Reference; // expected-warning {{'reference' is deprecated}}
    typedef cuda::std::allocator<char const>::const_reference ConstReference; // expected-warning {{'const_reference' is
                                                                              // deprecated}}
    typedef cuda::std::allocator<char const>::rebind<int>::other Rebind; // expected-warning {{'rebind<int>' is
                                                                         // deprecated}}
  }
  {
    typedef cuda::std::allocator<void>::pointer Pointer; // expected-warning {{'pointer' is deprecated}}
    typedef cuda::std::allocator<void>::const_pointer ConstPointer; // expected-warning {{'const_pointer' is
                                                                    // deprecated}}
    // reference and const_reference are not provided by cuda::std::allocator<void>
    typedef cuda::std::allocator<void>::rebind<int>::other Rebind; // expected-warning {{'rebind<int>' is deprecated}}
  }
}

int main(int, char**)
{
  return 0;
}
