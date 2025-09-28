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
#include <uscl/std/type_traits>

#include "test_macros.h"

#if _LIBCUDACXX_HAS_NVFP16()
static_assert(cuda::std::is_same<cuda::std::common_type<__half, __half>::type, __half>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__half, __half&>::type, __half>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__half&, __half>::type, __half>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__half, __half&&>::type, __half>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__half&&, __half>::type, __half>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__half&, __half&&>::type, __half>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__half&&, __half&>::type, __half>::value, "");

static_assert(cuda::std::is_same<cuda::std::common_type<__half, float>::type, float>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__half, float&>::type, float>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__half&, float>::type, float>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__half, float&&>::type, float>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__half&&, float>::type, float>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__half&, float&&>::type, float>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__half&&, float&>::type, float>::value, "");
#endif // _LIBCUDACXX_HAS_NVFP16()

#if _LIBCUDACXX_HAS_NVBF16()
static_assert(cuda::std::is_same<cuda::std::common_type<__nv_bfloat16, __nv_bfloat16>::type, __nv_bfloat16>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__nv_bfloat16, __nv_bfloat16&>::type, __nv_bfloat16>::value,
              "");
static_assert(cuda::std::is_same<cuda::std::common_type<__nv_bfloat16&, __nv_bfloat16>::type, __nv_bfloat16>::value,
              "");
static_assert(cuda::std::is_same<cuda::std::common_type<__nv_bfloat16, __nv_bfloat16&&>::type, __nv_bfloat16>::value,
              "");
static_assert(cuda::std::is_same<cuda::std::common_type<__nv_bfloat16&&, __nv_bfloat16>::type, __nv_bfloat16>::value,
              "");
static_assert(cuda::std::is_same<cuda::std::common_type<__nv_bfloat16&, __nv_bfloat16&&>::type, __nv_bfloat16>::value,
              "");
static_assert(cuda::std::is_same<cuda::std::common_type<__nv_bfloat16&&, __nv_bfloat16&>::type, __nv_bfloat16>::value,
              "");

static_assert(cuda::std::is_same<cuda::std::common_type<__nv_bfloat16, float>::type, float>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__nv_bfloat16, float&>::type, float>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__nv_bfloat16&, float>::type, float>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__nv_bfloat16, float&&>::type, float>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__nv_bfloat16&&, float>::type, float>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__nv_bfloat16&, float&&>::type, float>::value, "");
static_assert(cuda::std::is_same<cuda::std::common_type<__nv_bfloat16&&, float&>::type, float>::value, "");

static_assert(!cuda::std::__has_common_type<__nv_bfloat16, __half>, "");
static_assert(!cuda::std::__has_common_type<__nv_bfloat16, __half&>, "");
static_assert(!cuda::std::__has_common_type<__nv_bfloat16&, __half>, "");
static_assert(!cuda::std::__has_common_type<__nv_bfloat16, __half&&>, "");
static_assert(!cuda::std::__has_common_type<__nv_bfloat16&&, __half>, "");
static_assert(!cuda::std::__has_common_type<__nv_bfloat16&, __half&&>, "");
static_assert(!cuda::std::__has_common_type<__nv_bfloat16&&, __half&>, "");

#endif // _LIBCUDACXX_HAS_NVBF16()

int main(int argc, char** argv)
{
  return 0;
}
