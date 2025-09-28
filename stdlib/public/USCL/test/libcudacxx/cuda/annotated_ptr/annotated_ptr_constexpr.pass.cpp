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
// UNSUPPORTED: nvrtc

// error: expression must have a constant value annotated_ptr.h: note #2701-D: attempt to access run-time storage
// UNSUPPORTED: clang-14, gcc-12, gcc-11, gcc-10, gcc-9, gcc-8, gcc-7, msvc-19.29
// UNSUPPORTED: msvc && nvcc-12.0

#include <uscl/annotated_ptr>

#include "test_macros.h"

__host__ __device__ constexpr bool test_public_methods()
{
  using namespace cuda;
  using annotated_ptr                       = cuda::annotated_ptr<const int, access_property::persisting>;
  using annotated_smem_ptr [[maybe_unused]] = cuda::annotated_ptr<const int, access_property::shared>;
  annotated_ptr a{}; // default constructor
  annotated_ptr b{a}; // copy constructor
  annotated_ptr c{cuda::std::move(a)}; // move constructor
  NV_IF_TARGET(NV_IS_DEVICE, (annotated_smem_ptr d{nullptr};)) // pointer constructor
  b         = a; // copy assignment
  b         = cuda::std::move(a); // move assignment
  auto diff = a - b;
  auto pred = static_cast<bool>(a);
  auto prop = a.__property();
  unused(c);
  unused(diff);
  unused(pred);
  unused(prop);
  return true;
}

__host__ __device__ constexpr bool test_interleave_values()
{
  using namespace cuda;
  constexpr auto normal = __l2_interleave(__l2_evict_t::_L2_Evict_Unchanged, __l2_evict_t::_L2_Evict_Unchanged, 1.0f);
  constexpr auto streaming  = __l2_interleave(__l2_evict_t::_L2_Evict_First, __l2_evict_t::_L2_Evict_Unchanged, 1.0f);
  constexpr auto persisting = __l2_interleave(__l2_evict_t::_L2_Evict_Last, __l2_evict_t::_L2_Evict_Unchanged, 1.0f);
  constexpr auto normal_demote =
    __l2_interleave(__l2_evict_t::_L2_Evict_Normal_Demote, __l2_evict_t::_L2_Evict_Unchanged, 1.0f);
  static_assert(normal == __l2_interleave_normal);
  static_assert(streaming == __l2_interleave_streaming);
  static_assert(persisting == __l2_interleave_persisting);
  static_assert(normal_demote == __l2_interleave_normal_demote);
  return true;
}

int main(int, char**)
{
  static_assert(test_interleave_values());
  static_assert(test_public_methods());
  return 0;
}
