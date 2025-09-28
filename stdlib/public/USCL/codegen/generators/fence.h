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

#ifndef FENCE_H
#define FENCE_H

#include <string>

#include "definitions.h"
#include <fmt/format.h>

inline std::string membar_scope(Scope sco)
{
  static std::map scope_map{
    std::pair{Scope::GPU, ".gl"},
    std::pair{Scope::System, ".sys"},
    std::pair{Scope::CTA, ".cta"},
  };

  return scope_map[sco];
}

inline void FormatFence(std::ostream& out)
{
  // Argument ID Reference
  // 0 - Membar scope tag
  // 1 - Membar scope
  const std::string intrinsic_membar = R"XXX(
static inline _CCCL_DEVICE void __cuda_atomic_membar({0})
{{ asm volatile("membar{1};" ::: "memory"); }})XXX";

  const std::map membar_scopes{
    std::pair{Scope::GPU, ".gl"},
    std::pair{Scope::System, ".sys"},
    std::pair{Scope::CTA, ".cta"},
  };

  for (const auto& sco : membar_scopes)
  {
    out << fmt::format(intrinsic_membar, scope_tag(sco.first), sco.second);
  }

  // Argument ID Reference
  // 0 - Fence scope tag
  // 1 - Fence scope
  // 2 - Fence order tag
  // 3 - Fence order
  const std::string intrinsic_fence = R"XXX(
static inline _CCCL_DEVICE void __cuda_atomic_fence({0}, {2})
{{ asm volatile("fence{1}{3};" ::: "memory"); }})XXX";

  const Scope fence_scopes[] = {
    Scope::CTA,
    Scope::Cluster,
    Scope::GPU,
    Scope::System,
  };

  const Semantic fence_semantics[] = {
    Semantic::Acq_Rel,
    Semantic::Seq_Cst,
  };

  for (const auto& sco : fence_scopes)
  {
    for (const auto& sem : fence_semantics)
    {
      out << fmt::format(intrinsic_fence, scope_tag(sco), semantic(sem), semantic_tag(sem), scope(sco));
    }
  }
  out << "\n"
      << R"XXX(
template <typename _Sco>
static inline _CCCL_DEVICE void __atomic_thread_fence_cuda(int __memorder, _Sco) {
  NV_DISPATCH_TARGET(
    NV_PROVIDES_SM_70, (
      switch (__memorder) {
        case __ATOMIC_SEQ_CST: __cuda_atomic_fence(_Sco{}, __atomic_cuda_seq_cst{}); break;
        case __ATOMIC_CONSUME: [[fallthrough]];
        case __ATOMIC_ACQUIRE: [[fallthrough]];
        case __ATOMIC_ACQ_REL: [[fallthrough]];
        case __ATOMIC_RELEASE: __cuda_atomic_fence(_Sco{}, __atomic_cuda_acq_rel{}); break;
        case __ATOMIC_RELAXED: break;
        default: assert(0);
      }
    ),
    NV_IS_DEVICE, (
      switch (__memorder) {
        case __ATOMIC_SEQ_CST: [[fallthrough]];
        case __ATOMIC_CONSUME: [[fallthrough]];
        case __ATOMIC_ACQUIRE: [[fallthrough]];
        case __ATOMIC_ACQ_REL: [[fallthrough]];
        case __ATOMIC_RELEASE: __cuda_atomic_membar(_Sco{}); break;
        case __ATOMIC_RELAXED: break;
        default: assert(0);
      }
    )
  )
}
)XXX";
}

#endif // FENCE_H
