/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Friday, December 29, 2023.
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

#ifndef LIBCUDACXX_TEST_STD_UTILITIES_MEMORY_SPECIALIZED_ALGORITHMS_OVERLOAD_COMPARE_ITERATOR_H
#define LIBCUDACXX_TEST_STD_UTILITIES_MEMORY_SPECIALIZED_ALGORITHMS_OVERLOAD_COMPARE_ITERATOR_H

#include <uscl/std/iterator>
#include <uscl/std/memory>

#include "test_macros.h"

// An iterator type that overloads operator== and operator!= without any constraints, which
// can trip up some algorithms if we compare iterators against types that we're not allowed to.
//
// See https://github.com/llvm/llvm-project/issues/69334 for details.
template <class Iterator>
struct overload_compare_iterator
{
  using value_type        = typename cuda::std::iterator_traits<Iterator>::value_type;
  using difference_type   = typename cuda::std::iterator_traits<Iterator>::difference_type;
  using reference         = typename cuda::std::iterator_traits<Iterator>::reference;
  using pointer           = typename cuda::std::iterator_traits<Iterator>::pointer;
  using iterator_category = typename cuda::std::iterator_traits<Iterator>::iterator_category;

  overload_compare_iterator() = default;

  __host__ __device__ explicit overload_compare_iterator(Iterator it)
      : it_(it)
  {}

  overload_compare_iterator(overload_compare_iterator const&)            = default;
  overload_compare_iterator(overload_compare_iterator&&)                 = default;
  overload_compare_iterator& operator=(overload_compare_iterator const&) = default;
  overload_compare_iterator& operator=(overload_compare_iterator&&)      = default;

  __host__ __device__ reference operator*() const noexcept
  {
    return *it_;
  }

  __host__ __device__ pointer operator->() const noexcept
  {
    return cuda::std::addressof(*it_);
  }

  __host__ __device__ overload_compare_iterator& operator++() noexcept
  {
    ++it_;
    return *this;
  }

  __host__ __device__ overload_compare_iterator operator++(int) const noexcept
  {
    overload_compare_iterator old(*this);
    ++(*this);
    return old;
  }

  __host__ __device__ bool operator==(overload_compare_iterator const& other) const noexcept
  {
    return this->it_ == other.it_;
  }

  __host__ __device__ bool operator!=(overload_compare_iterator const& other) const noexcept
  {
    return !this->operator==(other);
  }

  // Hostile overloads
  template <class Sentinel>
  __host__ __device__ friend bool operator==(overload_compare_iterator const& lhs, Sentinel const& rhs) noexcept
  {
    return static_cast<Iterator const&>(lhs) == rhs;
  }

  template <class Sentinel>
  __host__ __device__ friend bool operator!=(overload_compare_iterator const& lhs, Sentinel const& rhs) noexcept
  {
    return static_cast<Iterator const&>(lhs) != rhs;
  }

private:
  Iterator it_;
};

#endif // LIBCUDACXX_TEST_STD_UTILITIES_MEMORY_SPECIALIZED_ALGORITHMS_OVERLOAD_COMPARE_ITERATOR_H
