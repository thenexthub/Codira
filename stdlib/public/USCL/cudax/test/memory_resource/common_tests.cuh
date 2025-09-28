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

#pragma once

#include <uscl/experimental/memory_resource.cuh>

#include <utility.cuh>

template <typename ResourceType>
void test_deallocate_async(ResourceType& resource)
{
  cudax::stream stream{cuda::device_ref{0}};
  test::pinned<int> i(0);
  cuda::atomic_ref atomic_i(*i);

  int* allocation = static_cast<int*>(resource.allocate_sync(sizeof(int)));

  cudax::launch(stream, test::one_thread_dims, test::spin_until_80{}, i.get());
  cudax::launch(stream, test::one_thread_dims, test::assign_42{}, allocation);
  cudax::launch(stream, test::one_thread_dims, test::verify_42{}, allocation);

  resource.deallocate(stream, allocation, sizeof(int));

  atomic_i.store(80);
  stream.sync();
}
