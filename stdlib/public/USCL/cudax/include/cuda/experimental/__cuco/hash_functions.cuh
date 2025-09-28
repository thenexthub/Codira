/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, July 24, 2024.
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

#ifndef _CUDAX__CUCO_HASH_FUNCTIONS_CUH
#define _CUDAX__CUCO_HASH_FUNCTIONS_CUH

#include <uscl/__cccl_config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/experimental/__cuco/detail/hash_functions/murmurhash3.cuh>
#include <uscl/experimental/__cuco/detail/hash_functions/xxhash.cuh>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental::cuco
{

enum class hash_algorithm
{
  xxhash_32,
  xxhash_64,
  murmurhash3_32
#if _CCCL_HAS_INT128()
  ,
  murmurhash3_x86_128,
  murmurhash3_x64_128
#endif // _CCCL_HAS_INT128()
};

//! @brief A hash function class specialized for different hash algorithms.
//!
//! @tparam _Key The type of the values to hash
//! @tparam _S The hash strategy to use, defaults to `hash_algorithm::xxhash_32`
template <typename _Key, hash_algorithm _S = hash_algorithm::xxhash_32>
class hash;

template <typename _Key>
class hash<_Key, hash_algorithm::xxhash_32> : private __detail::_XXHash_32<_Key>
{
public:
  using __detail::_XXHash_32<_Key>::_XXHash_32;
  using __detail::_XXHash_32<_Key>::operator();
};

template <typename _Key>
class hash<_Key, hash_algorithm::xxhash_64> : private __detail::_XXHash_64<_Key>
{
public:
  using __detail::_XXHash_64<_Key>::_XXHash_64;
  using __detail::_XXHash_64<_Key>::operator();
};

template <typename _Key>
class hash<_Key, hash_algorithm::murmurhash3_32> : private __detail::_MurmurHash3_32<_Key>
{
public:
  using __detail::_MurmurHash3_32<_Key>::_MurmurHash3_32;
  using __detail::_MurmurHash3_32<_Key>::operator();
};

#if _CCCL_HAS_INT128()

template <typename _Key>
class hash<_Key, hash_algorithm::murmurhash3_x86_128> : private __detail::_MurmurHash3_x86_128<_Key>
{
public:
  using __detail::_MurmurHash3_x86_128<_Key>::_MurmurHash3_x86_128;
  using __detail::_MurmurHash3_x86_128<_Key>::operator();
};

template <typename _Key>
class hash<_Key, hash_algorithm::murmurhash3_x64_128> : private __detail::_MurmurHash3_x64_128<_Key>
{
public:
  using __detail::_MurmurHash3_x64_128<_Key>::_MurmurHash3_x64_128;
  using __detail::_MurmurHash3_x64_128<_Key>::operator();
};

#endif // _CCCL_HAS_INT128()

} // namespace cuda::experimental::cuco

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDAX__CUCO_HASH_FUNCTIONS_CUH
