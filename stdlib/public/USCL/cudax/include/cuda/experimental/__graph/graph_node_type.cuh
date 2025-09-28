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

#ifndef __CUDAX_GRAPH_GRAPH_NODE_TYPE
#define __CUDAX_GRAPH_GRAPH_NODE_TYPE

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <cuda_runtime_api.h>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental
{
//! \enum graph_node_type
//!
//! \brief Represents the types of nodes that can exist in a CUDA graph.
//!
//! This enumeration defines various node types that can be used in CUDA graphs
//! to represent different operations or functionalities.
//!
//! \var graph_node_type::kernel
//! Represents a kernel execution node.
//!
//! \var graph_node_type::memcpy
//! Represents a memory copy operation node.
//!
//! \var graph_node_type::memset
//! Represents a memory set operation node.
//!
//! \var graph_node_type::host
//! Represents a host function execution node.
//!
//! \var graph_node_type::graph
//! Represents a nested graph node.
//!
//! \var graph_node_type::empty
//! Represents an empty node with no operation.
//!
//! \var graph_node_type::wait_event
//! Represents a node that waits for an event.
//!
//! \var graph_node_type::event_record
//! Represents a node that records an event.
//!
//! \var graph_node_type::semaphore_signal
//! Represents a node that signals an external semaphore.
//!
//! \var graph_node_type::semaphore_wait
//! Represents a node that waits on an external semaphore.
//!
//! \var graph_node_type::malloc
//! Represents a node that performs memory allocation.
//!
//! \var graph_node_type::free
//! Represents a node that performs memory deallocation.
//!
//! \var graph_node_type::batch_memop
//! Represents a node that performs a batch memory operation.
//!
//! \var graph_node_type::conditional
//! Represents a conditional execution node.
enum class graph_node_type : int
{
  kernel           = cudaGraphNodeTypeKernel,
  memcpy           = cudaGraphNodeTypeMemcpy,
  memset           = cudaGraphNodeTypeMemset,
  host             = cudaGraphNodeTypeHost,
  graph            = cudaGraphNodeTypeGraph,
  empty            = cudaGraphNodeTypeEmpty,
  wait_event       = cudaGraphNodeTypeWaitEvent,
  event_record     = cudaGraphNodeTypeEventRecord,
  semaphore_signal = cudaGraphNodeTypeExtSemaphoreSignal,
  semaphore_wait   = cudaGraphNodeTypeExtSemaphoreWait,
  malloc           = cudaGraphNodeTypeMemAlloc,
  free             = cudaGraphNodeTypeMemFree,
// batch_memop      = CU_GRAPH_NODE_TYPE_BATCH_MEM_OP, // not exposed by the CUDA runtime

#if _CCCL_CUDACC_AT_LEAST(12, 8)
  conditional = cudaGraphNodeTypeConditional
#endif
};

} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif // __CUDAX_GRAPH_GRAPH_NODE_TYPE
