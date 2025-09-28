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

#ifndef _CCCL_C_PARALLEL_JIT_TEMPLATES_PREPROCESS
#  include <cuda/std/cstddef>
#  include <cuda/std/iterator>
#  include <cuda/std/optional>
#  include <cuda/std/type_traits>
#  include <cuda/std/utility>

#  include "../traits.h"
#  include <cccl/c/types.h>
#endif

#include "../mappings/iterator.h"

template <typename Tag, cuda::std::size_t Size, cuda::std::size_t Alignment>
struct alignas(Alignment) input_iterator_state_t
{
  char data[Size];
};

template <typename Tag, cccl_iterator_t_mapping Iterator>
struct input_iterator_t
{
  using iterator_category = cuda::std::random_access_iterator_tag;
  using difference_type   = cuda::std::size_t;
  using value_type        = typename decltype(Iterator)::ValueT;
  using reference         = value_type&;
  using pointer           = value_type*;

  __device__ value_type operator*() const
  {
    value_type result;
    Iterator.dereference(&state, &result);
    return result;
  }

  __device__ input_iterator_t& operator+=(difference_type diff)
  {
    Iterator.advance(&state, diff);
    return *this;
  }

  __device__ value_type operator[](difference_type diff) const
  {
    return *(*this + diff);
  }

  __device__ input_iterator_t operator+(difference_type diff) const
  {
    input_iterator_t result = *this;
    result += diff;
    return result;
  }

  input_iterator_state_t<Tag, Iterator.size, Iterator.alignment> state;
};

#ifndef _CCCL_C_PARALLEL_JIT_TEMPLATES_PREPROCESS
struct input_iterator_traits
{
  template <typename Tag, cccl_iterator_t_mapping Iterator>
  using type = input_iterator_t<Tag, Iterator>;

  static const constexpr auto name = "input_iterator_t";

  template <typename>
  static cuda::std::optional<specialization> special(cccl_iterator_t it)
  {
    if (it.type == cccl_iterator_kind_t::CCCL_POINTER)
    {
      return cuda::std::make_optional(specialization{cccl_type_enum_to_name(it.value_type.type, true), ""});
    }

    return cuda::std::nullopt;
  }
};
#endif
