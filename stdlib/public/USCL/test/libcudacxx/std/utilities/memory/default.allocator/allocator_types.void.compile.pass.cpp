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

// Check that the nested types of cuda::std::allocator<void> are provided.
// After C++17, those are not provided in the primary template and the
// explicit specialization doesn't exist anymore, so this test is moot.

// REQUIRES: c++03 || c++11 || c++14 || c++17

// template <>
// class allocator<void>
// {
// public:
//     typedef void*                                 pointer;
//     typedef const void*                           const_pointer;
//     typedef void                                  value_type;
//
//     template <class _Up> struct rebind {typedef allocator<_Up> other;};
// };

// ADDITIONAL_COMPILE_DEFINITIONS: _LIBCUDACXX_DISABLE_DEPRECATION_WARNINGS

#include <uscl/std/__memory_>
#include <uscl/std/type_traits>

static_assert((cuda::std::is_same<cuda::std::allocator<void>::pointer, void*>::value), "");
static_assert((cuda::std::is_same<cuda::std::allocator<void>::const_pointer, const void*>::value), "");
static_assert((cuda::std::is_same<cuda::std::allocator<void>::value_type, void>::value), "");
static_assert((cuda::std::is_same<cuda::std::allocator<void>::rebind<int>::other, cuda::std::allocator<int>>::value),
              "");

int main(int, char**)
{
  return 0;
}
