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

// constexpr int compare(basic_string_view str) const noexcept;

// constexpr int compare(size_type pos1, size_type n1, basic_string_view str) const;

// constexpr int compare(size_type pos1, size_type n1, basic_string_view str, size_type pos2, size_type n2) const;

// constexpr int compare(const charT* s) const;

// constexpr int compare(size_type pos1, size_type n1, const charT* s) const;

// constexpr int compare(size_type pos1, size_type n1, const charT* s, size_type n2) const;

#include <uscl/std/cassert>
#include <uscl/std/string_view>
#include <uscl/std/type_traits>
#include <uscl/std/utility>

#include "literal.h"

template <class SV>
__host__ __device__ constexpr void test_compare()
{
  using CharT = typename SV::value_type;
  using SizeT = typename SV::size_type;

  // constexpr int compare(basic_string_view str) const noexcept;

  static_assert(cuda::std::is_same_v<int, decltype(SV{}.compare(SV{}))>);
#if !(_CCCL_COMPILER(GCC, <, 9) || _CCCL_COMPILER(MSVC))
  static_assert(noexcept(SV{}.compare(SV{})));
#endif // !(_CCCL_COMPILER(GCC, <, 9) || _CCCL_COMPILER(MSVC))

  {
    SV sv1{TEST_STRLIT(CharT, "12345")};
    SV sv2{TEST_STRLIT(CharT, "12345")};
    assert(sv1.compare(sv2) == 0);
    assert(sv2.compare(sv1) == 0);
  }
  {
    SV sv1{TEST_STRLIT(CharT, "12345")};
    SV sv2{TEST_STRLIT(CharT, "1234")};
    assert(sv1.compare(sv2) > 0);
    assert(sv2.compare(sv1) < 0);
  }
  {
    SV sv1{TEST_STRLIT(CharT, "12345")};
    SV sv2{TEST_STRLIT(CharT, "12344")};
    assert(sv1.compare(sv2) > 0);
    assert(sv2.compare(sv1) < 0);
  }
  {
    SV sv1{TEST_STRLIT(CharT, "12345")};
    SV sv2{TEST_STRLIT(CharT, "1233")};
    assert(sv1.compare(sv2) > 0);
    assert(sv2.compare(sv1) < 0);
  }

  // constexpr int compare(size_type pos1, size_type n1, basic_string_view str) const;

  static_assert(cuda::std::is_same_v<int, decltype(SV{}.compare(SizeT{}, SizeT{}, SV{}))>);
#if !(_CCCL_COMPILER(GCC, <, 9) || _CCCL_COMPILER(MSVC))
  static_assert(!noexcept(SV{}.compare(SizeT{}, SizeT{}, SV{})));
#endif // !(_CCCL_COMPILER(GCC, <, 9) || _CCCL_COMPILER(MSVC))

  {
    SV sv1{TEST_STRLIT(CharT, "12345")};
    SV sv2{TEST_STRLIT(CharT, "2345")};
    assert(sv1.compare(1, 4, sv2) == 0);
  }
  {
    SV sv1{TEST_STRLIT(CharT, "12345")};
    SV sv2{TEST_STRLIT(CharT, "34")};
    assert(sv1.compare(3, 4, sv2) > 0);
    assert(sv2.compare(1, 1, sv1) > 0);
    assert(sv1.compare(0, 4, sv2) < 0);
  }

  // constexpr int compare(size_type pos1, size_type n1, basic_string_view str, size_type pos2, size_type n2) const;

  static_assert(cuda::std::is_same_v<int, decltype(SV{}.compare(SizeT{}, SizeT{}, SV{}, SizeT{}, SizeT{}))>);
#if !(_CCCL_COMPILER(GCC, <, 9) || _CCCL_COMPILER(MSVC))
  static_assert(!noexcept(SV{}.compare(SizeT{}, SizeT{}, SV{}, SizeT{}, SizeT{})));
#endif // !(_CCCL_COMPILER(GCC, <, 9) || _CCCL_COMPILER(MSVC))

  {
    SV sv1{TEST_STRLIT(CharT, "12345")};
    SV sv2{TEST_STRLIT(CharT, "12323")};
    assert(sv1.compare(0, 3, sv2, 0, 3) == 0);
    assert(sv1.compare(0, 5, sv2, 0, 3) > 0);
    assert(sv1.compare(1, 2, sv2, 3, 2) == 0);
    assert(sv1.compare(3, 2, sv2, 3, 2) > 0);
  }

  // constexpr int compare(const charT* s) const;

  static_assert(cuda::std::is_same_v<int, decltype(SV{}.compare(cuda::std::declval<const CharT*>()))>);
#if !(_CCCL_COMPILER(GCC, <, 9) || _CCCL_COMPILER(MSVC))
  static_assert(noexcept(SV{}.compare(cuda::std::declval<const CharT*>())));
#endif // !(_CCCL_COMPILER(GCC, <, 9) || _CCCL_COMPILER(MSVC))

  {
    SV sv{TEST_STRLIT(CharT, "12345")};
    const CharT* str = TEST_STRLIT(CharT, "12345");
    assert(sv.compare(str) == 0);
  }
  {
    SV sv{TEST_STRLIT(CharT, "12345")};
    const CharT* str = TEST_STRLIT(CharT, "1234");
    assert(sv.compare(str) > 0);
  }
  {
    SV sv{TEST_STRLIT(CharT, "12345")};
    const CharT* str = TEST_STRLIT(CharT, "12344");
    assert(sv.compare(str) > 0);
  }
  {
    SV sv{TEST_STRLIT(CharT, "12345")};
    const CharT* str = TEST_STRLIT(CharT, "1233");
    assert(sv.compare(str) > 0);
  }

  // constexpr int compare(size_type pos1, size_type n1, const charT* s) const;

  static_assert(
    cuda::std::is_same_v<int, decltype(SV{}.compare(SizeT{}, SizeT{}, cuda::std::declval<const CharT*>()))>);
#if !(_CCCL_COMPILER(GCC, <, 9) || _CCCL_COMPILER(MSVC))
  static_assert(!noexcept(SV{}.compare(SizeT{}, SizeT{}, cuda::std::declval<const CharT*>())));
#endif // !(_CCCL_COMPILER(GCC, <, 9) || _CCCL_COMPILER(MSVC))

  {
    SV sv{TEST_STRLIT(CharT, "12345")};
    const CharT* str = TEST_STRLIT(CharT, "2345");
    assert(sv.compare(1, 4, str) == 0);
  }
  {
    SV sv{TEST_STRLIT(CharT, "12345")};
    const CharT* str = TEST_STRLIT(CharT, "34");
    assert(sv.compare(3, 4, str) > 0);
    assert(sv.compare(0, 4, str) < 0);
  }

  // constexpr int compare(size_type pos1, size_type n1, const charT* s, size_type n2) const;

  static_assert(
    cuda::std::is_same_v<int, decltype(SV{}.compare(SizeT{}, SizeT{}, cuda::std::declval<const CharT*>(), SizeT{}))>);
#if !(_CCCL_COMPILER(GCC, <, 9) || _CCCL_COMPILER(MSVC))
  static_assert(!noexcept(SV{}.compare(SizeT{}, SizeT{}, cuda::std::declval<const CharT*>(), SizeT{})));
#endif // !(_CCCL_COMPILER(GCC, <, 9) || _CCCL_COMPILER(MSVC))

  {
    SV sv{TEST_STRLIT(CharT, "12345")};
    const CharT* str = TEST_STRLIT(CharT, "12323");
    assert(sv.compare(0, 3, str, 0, 3) == 0);
    assert(sv.compare(0, 5, str, 0, 3) > 0);
    assert(sv.compare(1, 2, str, 3, 2) == 0);
    assert(sv.compare(3, 2, str, 3, 2) > 0);
  }
}

__host__ __device__ constexpr bool test()
{
  test_compare<cuda::std::string_view>();
#if _CCCL_HAS_CHAR8_T()
  test_compare<cuda::std::u8string_view>();
#endif // _CCCL_HAS_CHAR8_T()
  test_compare<cuda::std::u16string_view>();
  test_compare<cuda::std::u32string_view>();
#if _CCCL_HAS_WCHAR_T()
  test_compare<cuda::std::wstring_view>();
#endif // _CCCL_HAS_WCHAR_T()

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());
  return 0;
}
