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

// Include this first
#include <uscl/experimental/execution.cuh>

// Then include the test helpers
#include <nv/target>

#include "common/checked_receiver.cuh"
#include "common/utility.cuh"
#include "testing.cuh"

namespace
{
C2H_TEST("simple use of sequence executes both child operations", "[adaptors][sequence]")
{
  bool flag1{false};
  bool flag2{false};

  auto sndr1 = cudax_async::sequence(
    cudax_async::just() | cudax_async::then([&] {
      flag1 = true;
    }),
    cudax_async::just() | cudax_async::then([&] {
      flag2 = true;
    }));

  check_value_types<types<>>(sndr1);
  check_sends_stopped<false>(sndr1);
  NV_IF_ELSE_TARGET(NV_IS_HOST, //
                    ({ check_error_types<std::exception_ptr>(sndr1); }),
                    ({ check_error_types<>(sndr1); }));

  auto op = cudax_async::connect(std::move(sndr1), checked_value_receiver<>{});
  cudax_async::start(op);

  CHECK(flag1);
  CHECK(flag2);
}

} // namespace
