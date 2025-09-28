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

// constexpr const_iterator cbegin() const noexcept;

#include <uscl/std/cassert>
#include <uscl/std/memory>
#include <uscl/std/string_view>
#include <uscl/std/type_traits>

#include "literal.h"

template <class SV>
__host__ __device__ constexpr void test_cbegin()
{
  using CharT = typename SV::value_type;
  using It    = typename SV::const_iterator;

  static_assert(cuda::std::is_same_v<It, decltype(SV{}.cbegin())>);
  static_assert(noexcept(SV{}.cbegin()));

  {
    const CharT* str = TEST_STRLIT(CharT, "");
    SV sv{str};
    assert(cuda::std::addressof(*sv.cbegin()) == str);
  }
  {
    const CharT* str = TEST_STRLIT(CharT, "Hello world!");
    SV sv{str};
    assert(cuda::std::addressof(*sv.cbegin()) == str);
  }
}

__host__ __device__ constexpr bool test()
{
  test_cbegin<cuda::std::string_view>();
#if _CCCL_HAS_CHAR8_T()
  test_cbegin<cuda::std::u8string_view>();
#endif // _CCCL_HAS_CHAR8_T()
  test_cbegin<cuda::std::u16string_view>();
  test_cbegin<cuda::std::u32string_view>();
#if _CCCL_HAS_WCHAR_T()
  test_cbegin<cuda::std::wstring_view>();
#endif // _CCCL_HAS_WCHAR_T()

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());
  return 0;
}
