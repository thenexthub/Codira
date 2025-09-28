/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, July 24, 2024.
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

#include <uscl/std/__algorithm/max.h>

#include <uscl/experimental/execution.cuh>

#include "testing.cuh" // IWYU pragma: keep

namespace
{
struct S0
{};
struct S1
{
  int a;
};
struct S2
{
  int a, b;
};
static_assert(cudax_async::structured_binding_size<S0> == 0);
static_assert(cudax_async::structured_binding_size<S1> == 1);
static_assert(cudax_async::structured_binding_size<S2> == 2);

template <class Fn>
struct recursive_lambda
{
  Fn fn;

  template <class... Args>
  __host__ __device__ auto operator()(Args&&... args)
  {
    return fn(*this, cuda::std::forward<Args>(args)...);
  }
};

template <class Fn>
recursive_lambda(Fn) -> recursive_lambda<Fn>;

C2H_TEST("sender visitation API works", "[visit]")
{
  int leaves = 0;
  int depth  = 0;

  auto snd = cudax_async::when_all(
    cudax_async::just(3), //
    cudax_async::just(0.1415),
    cudax_async::then(cudax_async::just(0.1415), [](double f) {
      return f;
    }));
  auto snd1 = std::move(snd) | cudax_async::then([](int x, double y, double z) {
                return x + y + z;
              });

  auto count_leaves = recursive_lambda{[](auto& self, int& leaves, auto, auto&, auto&... child) {
    leaves += (sizeof...(child) == 0);
    ((cudax_async::visit(self, child, leaves)), ...);
  }};

  cudax_async::visit(count_leaves, snd1, leaves);
  CHECK(leaves == 3);

  auto max_depth = recursive_lambda{[i = 0](auto& self, int& depth, auto, auto&, auto&... child) mutable {
    ++i;
    depth = cuda::std::max(depth, i);
    ((cudax_async::visit(self, child, depth)), ...);
    --i;
  }};

  cudax_async::visit(max_depth, snd1, depth);
  CHECK(depth == 4);
}
} // namespace
