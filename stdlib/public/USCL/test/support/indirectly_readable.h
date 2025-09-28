/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, November 8, 2023.
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
#ifndef LIBCXX_TEST_SUPPORT_INDIRECTLY_READABLE_H
#define LIBCXX_TEST_SUPPORT_INDIRECTLY_READABLE_H

#include <uscl/std/type_traits>

template <class Token>
struct Common
{};

template <class Token>
struct T1 : Common<Token>
{};

template <class Token>
struct T2 : Common<Token>
{};

namespace cuda
{
namespace std
{
template <template <class> class T1Qual, template <class> class T2Qual, class Token>
struct basic_common_reference<T1<Token>, T2<Token>, T1Qual, T2Qual>
{
  using type = Common<Token>;
};
template <template <class> class T2Qual, template <class> class T1Qual, class Token>
struct basic_common_reference<T2<Token>, T1<Token>, T2Qual, T1Qual>
    : basic_common_reference<T1<Token>, T2<Token>, T1Qual, T2Qual>
{};
} // namespace std
} // namespace cuda

template <class Token>
struct IndirectlyReadable
{
  using value_type = T1<Token>;
  __host__ __device__ T2<Token>& operator*() const;
};

#endif // LIBCXX_TEST_SUPPORT_INDIRECTLY_READABLE_H
