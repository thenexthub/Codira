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

#include <uscl/experimental/execution.cuh>

#include "common/checked_receiver.cuh"
#include "testing.cuh" // IWYU pragma: keep

namespace
{

struct my_domain
{};

C2H_TEST("basic test of write_attrs", "[write_attrs]")
{
  auto sndr = cudax_async::just(42) | cudax_async::write_attrs(cudax_async::prop{cudax_async::get_domain, my_domain{}});
  [[maybe_unused]] auto domain = cudax_async::get_domain(cudax_async::get_env(sndr));
  STATIC_REQUIRE(std::is_same_v<decltype(domain), my_domain>);

  // Check that the sender can be connected and started
  auto op = cudax_async::connect(std::move(sndr), checked_value_receiver{42});
  cudax_async::start(op);
}

} // namespace
