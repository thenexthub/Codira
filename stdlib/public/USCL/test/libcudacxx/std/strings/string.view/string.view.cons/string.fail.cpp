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

// constexpr basic_string_view(const CharT* str);

#include <uscl/std/string_view>

template <class SV>
__host__ __device__ constexpr void test_str_constructor()
{
  using CharT = typename SV::value_type;

  const CharT* str = nullptr;

  [[maybe_unused]] SV sv{str};
}

__host__ __device__ constexpr bool test()
{
  test_str_constructor<cuda::std::string_view>();
#if _CCCL_HAS_CHAR8_T()
  test_str_constructor<cuda::std::u8string_view>();
#endif // _CCCL_HAS_CHAR8_T()
  test_str_constructor<cuda::std::u16string_view>();
  test_str_constructor<cuda::std::u32string_view>();
#if _CCCL_HAS_WCHAR_T()
  test_str_constructor<cuda::std::wstring_view>();
#endif // _CCCL_HAS_WCHAR_T()

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());
  return 0;
}
