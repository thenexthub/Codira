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

#include "testing.cuh"

namespace ex = cudax::execution;

namespace
{
__host__ __device__ bool _on_device() noexcept
{
  NV_IF_ELSE_TARGET(NV_IS_HOST, //
                    ({ return false; }),
                    ({ return true; }));
}

auto const main_thread_id = ::std::this_thread::get_id();

void simple_start_on_thread_test()
{
  ex::thread_context ctx;
  auto sch  = ctx.get_scheduler();
  auto sndr = ex::on(sch, ex::just() | ex::then([] {
                            CUDAX_CHECK(::std::this_thread::get_id() != main_thread_id);
                          }))
            | ex::then([]() -> int {
                CUDAX_CHECK(::std::this_thread::get_id() == main_thread_id);
                return 42;
              });
  auto [result] = ex::sync_wait(std::move(sndr)).value();
  CUDAX_CHECK(result == 42);
}

void simple_continue_on_thread_test()
{
  ex::thread_context ctx;
  auto sch  = ctx.get_scheduler();
  auto sndr = ex::just() | ex::on(sch, ex::then([] {
                                    CUDAX_CHECK(::std::this_thread::get_id() != main_thread_id);
                                  }))
            | ex::then([]() -> int {
                CUDAX_CHECK(::std::this_thread::get_id() == main_thread_id);
                return 42;
              });
  auto [result] = ex::sync_wait(std::move(sndr)).value();
  CUDAX_CHECK(result == 42);
}

void simple_start_on_stream_test()
{
  cudax::stream str{cuda::device_ref(0)};
  auto sch  = cudax::stream_ref{str};
  auto sndr = ex::on(sch, ex::just(42) | ex::then([] __host__ __device__(int i) -> int {
                            return _on_device() ? i : -i;
                          }))
            | ex::then([] __host__ __device__(int i) -> int {
                return _on_device() ? -1 : i;
              });
  auto [result] = ex::sync_wait(std::move(sndr)).value();
  CUDAX_CHECK(result == 42);
}

void simple_continue_on_stream_test()
{
  cudax::stream str{cuda::device_ref(0)};
  auto sch  = cudax::stream_ref{str};
  auto sndr = ex::just(42) | ex::on(sch, ex::then([] __host__ __device__(int i) -> int {
                                      return _on_device() ? i : -i;
                                    }))
            | ex::then([] __host__ __device__(int i) -> int {
                return _on_device() ? -1 : i;
              });
  auto [result] = ex::sync_wait(std::move(sndr)).value();
  CUDAX_CHECK(result == 42);
}

C2H_TEST("simple on(sch, sndr) thread test", "[on]")
{
  simple_start_on_thread_test();
}

C2H_TEST("simple on(sndr, sch, closure) thread test", "[on]")
{
  simple_continue_on_thread_test();
}

C2H_TEST("simple on(sch, sndr) stream test", "[on][stream]")
{
  simple_start_on_stream_test();
}

C2H_TEST("simple on(sndr, sch, closure) stream test", "[on][stream]")
{
  simple_continue_on_stream_test();
}

} // namespace
