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

#ifndef __CUDAX_GRAPH_DEPENDS_ON
#define __CUDAX_GRAPH_DEPENDS_ON

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/array>

#include <uscl/experimental/__graph/fwd.cuh>
#include <uscl/experimental/__graph/graph_node_ref.cuh>

#include <cuda_runtime_api.h>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental
{
//! \brief Builds an array of graph nodes that represent dependencies. It is for use as a
//!        parameter to the `graph_builder::add` function.
//!
//! \tparam _Nodes Variadic template parameter representing the types of the graph nodes.
//!         Each type must be either `graph_node_ref` or `cudaGraphNode_t`.
//! \param __nodes The graph nodes to add as dependencies to a new node.
//! \return A object of type `cuda::std::array<cudaGraphNode_t, sizeof...(_Nodes)>`
//!         containing the references to the provided graph nodes.
//!
//! \note A static assertion ensures that all provided arguments are convertible to
//!       `graph_node_ref`. If this condition is not met, a compilation error will occur.
// TODO graph_node_ref needs a graph argument if this function would accept cudaGraphNode_t
// TODO we should consider defining a type that also wraps a device and a graph and making it a graph_inserter,
//      and then we could return it here. It would serve as a non-advancing alternative to path_builder.
template <class... _Nodes>
_CCCL_NODEBUG_HOST_API constexpr auto depends_on(const _Nodes&... __nodes) noexcept
  -> ::cuda::std::array<cudaGraphNode_t, sizeof...(_Nodes)>
{
  return ::cuda::std::array<cudaGraphNode_t, sizeof...(_Nodes)>{{graph_node_ref(__nodes).get()...}};
}
} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif // __CUDAX_GRAPH_DEPENDS_ON
