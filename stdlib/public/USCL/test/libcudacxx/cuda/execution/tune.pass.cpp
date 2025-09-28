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

#include <uscl/__execution/tune.h>

struct get_reduce_tuning_query_t
{};

template <class Derived>
struct reduce_tuning
{
  [[nodiscard]] _CCCL_NODEBUG_API constexpr auto query(const get_reduce_tuning_query_t&) const noexcept -> Derived
  {
    return static_cast<const Derived&>(*this);
  }
};

template <int BlockThreads>
struct reduce : reduce_tuning<reduce<BlockThreads>>
{
  template <class T>
  struct type
  {
    struct max_policy
    {
      struct reduce_policy
      {
        static constexpr int block_threads = BlockThreads / sizeof(T);
      };
    };
  };
};

struct get_scan_tuning_query_t
{};

struct scan_tuning
{
  [[nodiscard]] _CCCL_NODEBUG_API constexpr auto query(const get_scan_tuning_query_t&) const noexcept
  {
    return *this;
  }

  struct type
  {
    struct max_policy
    {
      struct reduce_policy
      {
        static constexpr int block_threads = 1;
      };
    };
  };
};

__host__ __device__ void test()
{
  constexpr int nominal_block_threads = 256;
  constexpr int block_threads         = nominal_block_threads / sizeof(int);

  using env_t           = decltype(cuda::execution::__tune(reduce<nominal_block_threads>{}, scan_tuning{}));
  using tuning_t        = cuda::std::execution::__query_result_t<env_t, cuda::execution::__get_tuning_t>;
  using reduce_tuning_t = cuda::std::execution::__query_result_t<tuning_t, get_reduce_tuning_query_t>;
  using scan_tuning_t   = cuda::std::execution::__query_result_t<tuning_t, get_scan_tuning_query_t>;
  using reduce_policy_t = reduce_tuning_t::type<int>;
  using scan_policy_t   = scan_tuning_t::type;

  static_assert(reduce_policy_t::max_policy::reduce_policy::block_threads == block_threads);
  static_assert(scan_policy_t::max_policy::reduce_policy::block_threads == 1);
}

int main(int, char**)
{
  test();

  return 0;
}
