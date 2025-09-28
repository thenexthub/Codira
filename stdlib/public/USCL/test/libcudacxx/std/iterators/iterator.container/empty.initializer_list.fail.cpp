/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, November 8, 2023.
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

// <cuda/std/iterator>
// template <class E> constexpr bool empty(initializer_list<E> il) noexcept;

// UNSUPPORTED: clang-3.3, clang-3.4, clang-3.5, clang-3.6, clang-3.7, clang-3.8
// nvrtc will not generate warnings/failures on nodiscard attribute
// UNSUPPORTED: nvrtc

#include <uscl/std/initializer_list>
#include <uscl/std/iterator>

#include "test_macros.h"

int main(int, char**)
{
  cuda::std::initializer_list<int> c = {4};
  cuda::std::empty(c); // expected-error {{ignoring return value of function declared with 'nodiscard' attribute}}

  return 0;
}
