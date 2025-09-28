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

// constexpr void remove_prefix(size_type n) const;

#include <uscl/std/cassert>
#include <uscl/std/string_view>
#include <uscl/std/type_traits>

#include "literal.h"

template <class SV>
__host__ __device__ constexpr void test_remove_prefix()
{
  using CharT = typename SV::value_type;
  using SizeT = typename SV::size_type;

  static_assert(cuda::std::is_same_v<void, decltype(SV{}.remove_prefix(SizeT{}))>);
#if !(_CCCL_COMPILER(GCC, <, 9) || _CCCL_COMPILER(MSVC))
  static_assert(noexcept(SV{}.remove_prefix(SizeT{})));
#endif // !(_CCCL_COMPILER(GCC, <, 9) || _CCCL_COMPILER(MSVC))

  {
    const CharT* null_str = nullptr;
    SV sv{null_str, 0};

    sv.remove_prefix(0);
    assert(sv.data() == null_str);
    assert(sv.size() == 0);
  }
  {
    const CharT* str = TEST_STRLIT(CharT, "a");
    SV sv{str};

    sv.remove_prefix(0);
    assert(sv.data() == str);
    assert(sv.size() == 1);

    sv.remove_prefix(1);
    assert(sv.data() == str + 1);
    assert(sv.size() == 0);

    sv.remove_prefix(0);
    assert(sv.data() == str + 1);
    assert(sv.size() == 0);
  }
  {
    const CharT* str = TEST_STRLIT(CharT, "Hello world!");
    SV sv{str};

    sv.remove_prefix(0);
    assert(sv.data() == str);
    assert(sv.size() == 12);

    sv.remove_prefix(6);
    assert(sv.data() == str + 6);
    assert(sv.size() == 6);

    sv.remove_prefix(5);
    assert(sv.data() == str + 11);
    assert(sv.size() == 1);

    sv.remove_prefix(1);
    assert(sv.data() == str + 12);
    assert(sv.size() == 0);
  }
}

__host__ __device__ constexpr bool test()
{
  test_remove_prefix<cuda::std::string_view>();
#if _CCCL_HAS_CHAR8_T()
  test_remove_prefix<cuda::std::u8string_view>();
#endif // _CCCL_HAS_CHAR8_T()
  test_remove_prefix<cuda::std::u16string_view>();
  test_remove_prefix<cuda::std::u32string_view>();
#if _CCCL_HAS_WCHAR_T()
  test_remove_prefix<cuda::std::wstring_view>();
#endif // _CCCL_HAS_WCHAR_T()

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());
  return 0;
}
