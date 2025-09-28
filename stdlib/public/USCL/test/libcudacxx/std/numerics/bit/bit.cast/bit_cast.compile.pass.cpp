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

// <cuda/std/bit>
//
// template<class To, class From>
//   constexpr To bit_cast(const From& from) noexcept;

// This test makes sure that std::bit_cast fails when any of the following
// constraints are violated:
//
//      (1.1) sizeof(To) == sizeof(From) is true;
//      (1.2) is_trivially_copyable_v<To> is true;
//      (1.3) is_trivially_copyable_v<From> is true.
//
// Also check that it's ill-formed when the return type would be
// ill-formed, even though that is not explicitly mentioned in the
// specification (but it can be inferred from the synopsis).

#include <uscl/std/bit>
#include <uscl/std/type_traits>
#include <uscl/std/utility>

template <class To, class From, class = void>
struct bit_cast_is_valid : cuda::std::false_type
{};

template <class To, class From>
struct bit_cast_is_valid<To, From, decltype(cuda::std::bit_cast<To>(cuda::std::declval<const From&>()))>
    : cuda::std::is_same<To, decltype(cuda::std::bit_cast<To>(cuda::std::declval<const From&>()))>
{};

// Types are not the same size
namespace ns1
{
struct To
{
  char a;
};
struct From
{
  char a;
  char b;
};
static_assert(!bit_cast_is_valid<To, From>::value, "");
static_assert(!bit_cast_is_valid<From&, From>::value, "");
} // namespace ns1

// To is not trivially copyable
namespace ns2
{
struct To
{
  char a;
  __host__ __device__ To(To const&);
};
struct From
{
  char a;
};
static_assert(!bit_cast_is_valid<To, From>::value, "");
} // namespace ns2

// From is not trivially copyable
namespace ns3
{
struct To
{
  char a;
};
struct From
{
  char a;
  __host__ __device__ From(From const&);
};
static_assert(!bit_cast_is_valid<To, From>::value, "");
} // namespace ns3

// The return type is ill-formed
namespace ns4
{
struct From
{
  char a;
  char b;
};
static_assert(!bit_cast_is_valid<char[2], From>::value, "");
static_assert(!bit_cast_is_valid<int(), From>::value, "");
} // namespace ns4

int main(int, char**)
{
  return 0;
}
