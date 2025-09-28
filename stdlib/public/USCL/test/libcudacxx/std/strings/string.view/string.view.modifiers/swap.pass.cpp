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

// constexpr void swap(basic_string_view& s) noexcept;

#include <uscl/std/cassert>
#include <uscl/std/string_view>
#include <uscl/std/type_traits>
#include <uscl/std/utility>

#include "literal.h"

template <class SV>
__host__ __device__ constexpr void test_swap()
{
  using CharT  = typename SV::value_type;
  using Traits = typename SV::traits_type;

  static_assert(cuda::std::is_same_v<void, decltype(SV{}.swap(cuda::std::declval<SV&>()))>);
  static_assert(noexcept(SV{}.swap(cuda::std::declval<SV&>())));

  const CharT* str1 = TEST_STRLIT(CharT, "Hello");
  const CharT* str2 = TEST_STRLIT(CharT, "World!");

  const auto str1_size = Traits::length(str1);
  const auto str2_size = Traits::length(str2);

  {
    SV sv1{str1};
    SV sv2{str2};

    assert(sv1.data() == str1);
    assert(sv1.size() == str1_size);
    assert(sv2.data() == str2);
    assert(sv2.size() == str2_size);

    sv1.swap(sv2);

    assert(sv1.data() == str2);
    assert(sv1.size() == str2_size);
    assert(sv2.data() == str1);
    assert(sv2.size() == str1_size);
  }
  {
    SV sv1;
    SV sv2{str2};

    assert(sv1.data() == nullptr);
    assert(sv1.size() == 0);
    assert(sv2.data() == str2);
    assert(sv2.size() == str2_size);

    sv1.swap(sv2);

    assert(sv1.data() == str2);
    assert(sv1.size() == str2_size);
    assert(sv2.data() == nullptr);
    assert(sv2.size() == 0);
  }
  {
    SV sv1{str1};
    SV sv2;

    assert(sv1.data() == str1);
    assert(sv1.size() == str1_size);
    assert(sv2.data() == nullptr);
    assert(sv2.size() == 0);

    sv1.swap(sv2);

    assert(sv1.data() == nullptr);
    assert(sv1.size() == 0);
    assert(sv2.data() == str1);
    assert(sv2.size() == str1_size);
  }
  {
    SV sv1;
    SV sv2;

    assert(sv1.data() == nullptr);
    assert(sv1.size() == 0);
    assert(sv2.data() == nullptr);
    assert(sv2.size() == 0);

    sv1.swap(sv2);

    assert(sv1.data() == nullptr);
    assert(sv1.size() == 0);
    assert(sv2.data() == nullptr);
    assert(sv2.size() == 0);
  }
}

__host__ __device__ constexpr bool test()
{
  test_swap<cuda::std::string_view>();
#if _CCCL_HAS_CHAR8_T()
  test_swap<cuda::std::u8string_view>();
#endif // _CCCL_HAS_CHAR8_T()
  test_swap<cuda::std::u16string_view>();
  test_swap<cuda::std::u32string_view>();
#if _CCCL_HAS_WCHAR_T()
  test_swap<cuda::std::wstring_view>();
#endif // _CCCL_HAS_WCHAR_T()

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());
  return 0;
}
