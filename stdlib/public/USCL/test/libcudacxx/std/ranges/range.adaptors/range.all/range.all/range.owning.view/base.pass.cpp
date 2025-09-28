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

// constexpr R& base() & noexcept { return r_; }
// constexpr const R& base() const& noexcept { return r_; }
// constexpr R&& base() && noexcept { return cuda::std::move(r_); }
// constexpr const R&& base() const&& noexcept { return cuda::std::move(r_); }

#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/ranges>

#include "test_macros.h"

struct Base
{
  __host__ __device__ int* begin() const;
  __host__ __device__ int* end() const;
};

__host__ __device__ constexpr bool test()
{
  using OwningView = cuda::std::ranges::owning_view<Base>;
  OwningView ov;
  decltype(auto) b1 = static_cast<OwningView&>(ov).base();
  decltype(auto) b2 = static_cast<OwningView&&>(ov).base();
  decltype(auto) b3 = static_cast<const OwningView&>(ov).base();
  decltype(auto) b4 = static_cast<const OwningView&&>(ov).base();

  static_assert(cuda::std::is_same_v<decltype(b1), Base&>);
  static_assert(cuda::std::is_same_v<decltype(b2), Base&&>);
  static_assert(cuda::std::is_same_v<decltype(b3), const Base&>);
  static_assert(cuda::std::is_same_v<decltype(b4), const Base&&>);

  assert(&b1 == &b2);
  assert(&b1 == &b3);
  assert(&b1 == &b4);

  static_assert(noexcept(static_cast<OwningView&>(ov).base()));
  static_assert(noexcept(static_cast<OwningView&&>(ov).base()));
  static_assert(noexcept(static_cast<const OwningView&>(ov).base()));
  static_assert(noexcept(static_cast<const OwningView&&>(ov).base()));

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
