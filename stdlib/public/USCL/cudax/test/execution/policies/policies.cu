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

#include <uscl/std/execution>
#include <uscl/std/type_traits>

#include <uscl/experimental/execution.cuh>

#include <testing.cuh>

namespace cudax = cuda::experimental;

template <class T, class U>
using is_same = cuda::std::is_same<cuda::std::remove_cvref_t<T>, U>;

C2H_TEST("Execution policies", "[execution][policies]")
{
  namespace execution = cuda::std::execution;
  SECTION("Individual options")
  {
    cudax::execution::any_execution_policy pol = execution::seq;
    pol                                        = execution::par;
    pol                                        = execution::par_unseq;
    pol                                        = execution::unseq;
    CHECK(pol == execution::unseq);
  }
}
