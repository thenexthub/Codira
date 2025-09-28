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

// UNSUPPORTED: msvc-19.16

// unique_ptr

#include <uscl/std/__memory_>
#include <uscl/std/iterator>

static_assert(cuda::std::indirectly_readable<cuda::std::unique_ptr<int>>);
static_assert(cuda::std::indirectly_writable<cuda::std::unique_ptr<int>, int>);
static_assert(!cuda::std::weakly_incrementable<cuda::std::unique_ptr<int>>);
static_assert(cuda::std::indirectly_movable<cuda::std::unique_ptr<int>, cuda::std::unique_ptr<int>>);
static_assert(cuda::std::indirectly_movable_storable<cuda::std::unique_ptr<int>, cuda::std::unique_ptr<int>>);
static_assert(cuda::std::indirectly_copyable<cuda::std::unique_ptr<int>, cuda::std::unique_ptr<int>>);
static_assert(cuda::std::indirectly_copyable_storable<cuda::std::unique_ptr<int>, cuda::std::unique_ptr<int>>);
static_assert(cuda::std::indirectly_swappable<cuda::std::unique_ptr<int>, cuda::std::unique_ptr<int>>);

static_assert(!cuda::std::indirectly_readable<cuda::std::unique_ptr<void>>);
static_assert(!cuda::std::indirectly_writable<cuda::std::unique_ptr<void>, void>);
static_assert(!cuda::std::weakly_incrementable<cuda::std::unique_ptr<void>>);
static_assert(!cuda::std::indirectly_movable<cuda::std::unique_ptr<void>, cuda::std::unique_ptr<void>>);
static_assert(!cuda::std::indirectly_movable_storable<cuda::std::unique_ptr<void>, cuda::std::unique_ptr<void>>);
static_assert(!cuda::std::indirectly_copyable<cuda::std::unique_ptr<void>, cuda::std::unique_ptr<void>>);
static_assert(!cuda::std::indirectly_copyable_storable<cuda::std::unique_ptr<void>, cuda::std::unique_ptr<void>>);

int main(int, char**)
{
  return 0;
}
