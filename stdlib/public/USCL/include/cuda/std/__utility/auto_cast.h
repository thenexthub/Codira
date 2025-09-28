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

#ifndef _CUDA_STD___UTILITY_AUTO_CAST_H
#define _CUDA_STD___UTILITY_AUTO_CAST_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__type_traits/decay.h>

#if _CCCL_STD_VER >= 2023 && __cpp_auto_cast >= 202110L
#  define _LIBCUDACXX_AUTO_CAST(expr) auto(expr)
#elif _CCCL_STD_VER < 2020 && _CCCL_COMPILER(MSVC)
#  define _LIBCUDACXX_AUTO_CAST(expr) (::cuda::std::decay_t<decltype((expr))>) (expr)
#else
#  define _LIBCUDACXX_AUTO_CAST(expr) static_cast<::cuda::std::decay_t<decltype((expr))>>(expr)
#endif

#endif // _CUDA_STD___UTILITY_AUTO_CAST_H
