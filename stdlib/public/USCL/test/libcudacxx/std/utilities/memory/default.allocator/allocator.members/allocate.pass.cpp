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

// allocator:
// constexpr T* allocate(size_t n);

// ADDITIONAL_COMPILE_DEFINITIONS: _LIBCUDACXX_DISABLE_DEPRECATION_WARNINGS

#include <uscl/std/__memory_>
#include <uscl/std/cassert>
#include <uscl/std/cstddef> // for cuda::std::max_align_t

#include "count_new.h"
#include "test_macros.h"

TEST_DIAG_SUPPRESS_MSVC(4324) // structure was padded due to alignment specifier

#if _LIBCUDACXX_HAS_ALIGNED_ALLOCATION()
static const bool UsingAlignedNew = true;
#else
static const bool UsingAlignedNew = false;
#endif

#ifdef __STDCPP_DEFAULT_NEW_ALIGNMENT__
TEST_GLOBAL_VARIABLE const cuda::std::size_t MaxAligned = __STDCPP_DEFAULT_NEW_ALIGNMENT__;
#else
TEST_GLOBAL_VARIABLE const cuda::std::size_t MaxAligned = cuda::std::alignment_of<cuda::std::max_align_t>::value;
#endif

TEST_GLOBAL_VARIABLE const cuda::std::size_t OverAligned = MaxAligned * 2;

TEST_GLOBAL_VARIABLE int AlignedType_constructed = 0;

template <cuda::std::size_t Align>
struct alignas(Align) AlignedType
{
  char data;
  __host__ __device__ AlignedType()
  {
    ++AlignedType_constructed;
  }
  __host__ __device__ AlignedType(AlignedType const&)
  {
    ++AlignedType_constructed;
  }
  __host__ __device__ ~AlignedType()
  {
    --AlignedType_constructed;
  }
};

template <cuda::std::size_t Align>
__host__ __device__ void test_aligned()
{
  typedef AlignedType<Align> T;
  AlignedType_constructed = 0;
  globalMemCounter.reset();
  cuda::std::allocator<T> a;
  const bool IsOverAlignedType = Align > MaxAligned;
  const bool ExpectAligned     = IsOverAlignedType && UsingAlignedNew;
  {
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(AlignedType_constructed == 0);
    globalMemCounter.last_new_size  = 0;
    globalMemCounter.last_new_align = 0;
    T* ap                           = a.allocate(3);
#if !TEST_CUDA_COMPILER(NVCC) && !TEST_COMPILER(NVRTC)
    DoNotOptimize(ap);
#else
    [[maybe_unused]] const auto meow = reinterpret_cast<uintptr_t>(ap) + 2;
#endif // !TEST_CUDA_COMPILER(NVCC) && !TEST_COMPILER(NVRTC)
    // assert(globalMemCounter.checkOutstandingNewEq(1));
    assert(globalMemCounter.checkNewCalledEq(1));
    assert(globalMemCounter.checkAlignedNewCalledEq(ExpectAligned));
    assert(globalMemCounter.checkLastNewSizeEq(3 * sizeof(T)));
    assert(globalMemCounter.checkLastNewAlignEq(ExpectAligned ? Align : 0));
    assert(AlignedType_constructed == 0);
    globalMemCounter.last_delete_align = 0;
    a.deallocate(ap, 3);
    // assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(globalMemCounter.checkDeleteCalledEq(1));
    assert(globalMemCounter.checkAlignedDeleteCalledEq(ExpectAligned));
    assert(globalMemCounter.checkLastDeleteAlignEq(ExpectAligned ? Align : 0));
    assert(AlignedType_constructed == 0);
  }
}

#if TEST_STD_VER >= 2020
template <cuda::std::size_t Align>
__host__ __device__ constexpr bool test_aligned_constexpr()
{
  typedef AlignedType<Align> T;
  cuda::std::allocator<T> a;
  T* ap = a.allocate(3);
  a.deallocate(ap, 3);

  return true;
}
#endif // TEST_STD_VER >= 2020

int main(int, char**)
{
  test_aligned<1>();
  test_aligned<2>();
  test_aligned<4>();
  test_aligned<8>();
  test_aligned<16>();
  test_aligned<MaxAligned>();
  test_aligned<OverAligned>();
  test_aligned<OverAligned * 2>();

#if defined(_CCCL_HAS_CONSTEXPR_ALLOCATION)
  static_assert(test_aligned_constexpr<1>());
  static_assert(test_aligned_constexpr<2>());
  static_assert(test_aligned_constexpr<4>());
  static_assert(test_aligned_constexpr<8>());
  static_assert(test_aligned_constexpr<16>());
  static_assert(test_aligned_constexpr<MaxAligned>());
  static_assert(test_aligned_constexpr<OverAligned>());
  static_assert(test_aligned_constexpr<OverAligned * 2>());
#endif // _CCCL_HAS_CONSTEXPR_ALLOCATION

  return 0;
}
