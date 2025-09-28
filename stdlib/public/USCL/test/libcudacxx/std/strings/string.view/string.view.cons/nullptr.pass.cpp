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

// <cuda/std/string_view>

// constexpr basic_string_view(nullptr_t) = delete;

#include <uscl/std/cstddef>
#include <uscl/std/string_view>
#include <uscl/std/type_traits>

template <class SV>
__host__ __device__ constexpr void test_nullptr_constructor()
{
  static_assert(!cuda::std::is_constructible_v<SV, cuda::std::nullptr_t>);
}

__host__ __device__ constexpr bool test()
{
  test_nullptr_constructor<cuda::std::string_view>();
#if _CCCL_HAS_CHAR8_T()
  test_nullptr_constructor<cuda::std::u8string_view>();
#endif // _CCCL_HAS_CHAR8_T()
  test_nullptr_constructor<cuda::std::u16string_view>();
  test_nullptr_constructor<cuda::std::u32string_view>();
#if _CCCL_HAS_WCHAR_T()
  test_nullptr_constructor<cuda::std::wstring_view>();
#endif // _CCCL_HAS_WCHAR_T()

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());
  return 0;
}
