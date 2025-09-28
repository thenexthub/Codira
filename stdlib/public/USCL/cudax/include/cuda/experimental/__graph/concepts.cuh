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

#ifndef __CUDAX_GRAPH_CONCEPTS
#define __CUDAX_GRAPH_CONCEPTS

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__type_traits/disjunction.h>
#include <uscl/std/__type_traits/is_same.h>

#include <uscl/experimental/__graph/fwd.cuh>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental
{

// Concept to check if T is a graph dependency or contains them (either path_builder or graph_node_ref)
// TODO we might do something more abstract here rather than just checking specific types
template <typename T>
_CCCL_CONCEPT graph_dependency =
  ::cuda::std::is_same_v<::cuda::std::decay_t<T>, path_builder>
  || ::cuda::std::is_same_v<::cuda::std::decay_t<T>, graph_node_ref>;

// Concept to check if T can insert nodes into a graph
// TODO we might do something more abstract here rather than just checking specific types
template <typename T>
_CCCL_CONCEPT graph_inserter = ::cuda::std::is_same_v<::cuda::std::decay_t<T>, path_builder>;

} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif // __CUDAX_GRAPH_CONCEPTS
