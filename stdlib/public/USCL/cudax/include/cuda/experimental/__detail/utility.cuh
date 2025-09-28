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

#ifndef __CUDAX_DETAIL_UTILITY_H
#define __CUDAX_DETAIL_UTILITY_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__type_traits/is_callable.h>
#include <uscl/std/__type_traits/type_list.h>
#include <uscl/std/__utility/declval.h>
#include <uscl/std/__utility/move.h>

#include <uscl/experimental/__detail/type_traits.cuh>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental
{
// NOLINTBEGIN(misc-unused-using-decls)
using ::cuda::std::declval;
// NOLINTEND(misc-unused-using-decls)

struct _CCCL_TYPE_VISIBILITY_DEFAULT no_init_t
{
  _CCCL_HIDE_FROM_ABI explicit no_init_t() = default;
};

_CCCL_GLOBAL_CONSTANT no_init_t no_init{};

using uninit_t CCCL_DEPRECATED_BECAUSE("Use cuda::experimental::no_init_t instead") = no_init_t;

// TODO: CCCL_DEPRECATED_BECAUSE("Use cuda::experimental::no_init instead")
_CCCL_GLOBAL_CONSTANT no_init_t uninit{};
} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif // __CUDAX_DETAIL_UTILITY_H
