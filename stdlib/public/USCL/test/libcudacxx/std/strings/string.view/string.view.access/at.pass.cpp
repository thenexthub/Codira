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

// constexpr const_reference at(size_type pos) const;

#include <uscl/std/cassert>
#include <uscl/std/string_view>
#include <uscl/std/type_traits>

#include <nv/target>

#include "literal.h"
#include "test_macros.h"

#if TEST_HAS_EXCEPTIONS()
#  include <stdexcept>
#endif // TEST_HAS_EXCEPTIONS()

template <class SV>
__host__ __device__ constexpr void test_at()
{
  using CharT    = typename SV::value_type;
  using SizeT    = typename SV::size_type;
  using ConstRef = typename SV::const_reference;

  static_assert(cuda::std::is_same_v<ConstRef, decltype(SV{}.at(SizeT{}))>);
  static_assert(!noexcept(SV{}.at(SizeT{})));

  const CharT* str = TEST_STRLIT(CharT, "Hello world!");

  SV sv{str};
  assert(sv.at(0) == str[0]);
  assert(sv.at(1) == str[1]);
  assert(sv.at(4) == str[4]);
  assert(sv.at(8) == str[8]);
  assert(sv.at(11) == str[11]);
}

__host__ __device__ constexpr bool test()
{
  test_at<cuda::std::string_view>();
#if _CCCL_HAS_CHAR8_T()
  test_at<cuda::std::u8string_view>();
#endif // _CCCL_HAS_CHAR8_T()
  test_at<cuda::std::u16string_view>();
  test_at<cuda::std::u32string_view>();
#if _CCCL_HAS_WCHAR_T()
  test_at<cuda::std::wstring_view>();
#endif // _CCCL_HAS_WCHAR_T()

  return true;
}

#if TEST_HAS_EXCEPTIONS()
template <class SV>
void test_at_throw()
{
  using CharT = typename SV::value_type;

  const CharT* str = TEST_STRLIT(CharT, "Hello world!");
  SV sv{str};

  try
  {
    (void) sv.at(12);
    assert(false);
  }
  catch (const ::std::out_of_range&)
  {
    assert(true);
  }
  catch (...)
  {
    assert(false);
  }
}

bool test_exceptions()
{
  test_at_throw<cuda::std::string_view>();
#  if _CCCL_HAS_CHAR8_T()
  test_at_throw<cuda::std::u8string_view>();
#  endif // _CCCL_HAS_CHAR8_T()
  test_at_throw<cuda::std::u16string_view>();
  test_at_throw<cuda::std::u32string_view>();
#  if _CCCL_HAS_WCHAR_T()
  test_at_throw<cuda::std::wstring_view>();
#  endif // _CCCL_HAS_WCHAR_T()

  return true;
}
#endif // TEST_HAS_EXCEPTIONS()

int main(int, char**)
{
  test();
  static_assert(test());
#if TEST_HAS_EXCEPTIONS()
  NV_IF_TARGET(NV_IS_HOST, (test_exceptions();))
#endif // TEST_HAS_EXCEPTIONS()
  return 0;
}
