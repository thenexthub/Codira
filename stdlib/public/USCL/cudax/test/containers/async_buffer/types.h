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

#ifndef CUDAX_TEST_CONTAINER_VECTOR_TYPES_H
#define CUDAX_TEST_CONTAINER_VECTOR_TYPES_H

#include <thrust/equal.h>

#include <uscl/memory_resource>
#include <uscl/std/array>
#include <uscl/std/iterator>
#include <uscl/std/type_traits>

template <class It>
class forward_iterator
{
  It it_;

  template <class U>
  friend class forward_iterator;

public:
  typedef cuda::std::forward_iterator_tag iterator_category;
  typedef typename cuda::std::iterator_traits<It>::value_type value_type;
  typedef typename cuda::std::iterator_traits<It>::difference_type difference_type;
  typedef It pointer;
  typedef typename cuda::std::iterator_traits<It>::reference reference;

  __host__ __device__ constexpr forward_iterator()
      : it_()
  {}
  __host__ __device__ constexpr explicit forward_iterator(It it)
      : it_(it)
  {}

  template <class U>
  __host__ __device__ constexpr forward_iterator(const forward_iterator<U>& u)
      : it_(u.it_)
  {}

  template <class U, class = typename cuda::std::enable_if<cuda::std::is_default_constructible<U>::value>::type>
  __host__ __device__ constexpr forward_iterator(forward_iterator<U>&& other)
      : it_(other.it_)
  {
    other.it_ = U();
  }

  __host__ __device__ constexpr reference operator*() const
  {
    return *it_;
  }

  __host__ __device__ constexpr forward_iterator& operator++()
  {
    ++it_;
    return *this;
  }
  __host__ __device__ constexpr forward_iterator operator++(int)
  {
    return forward_iterator(it_++);
  }

  __host__ __device__ friend constexpr bool operator==(const forward_iterator& x, const forward_iterator& y)
  {
    return x.it_ == y.it_;
  }
  __host__ __device__ friend constexpr bool operator!=(const forward_iterator& x, const forward_iterator& y)
  {
    return x.it_ != y.it_;
  }

  __host__ __device__ friend constexpr It base(const forward_iterator& i)
  {
    return i.it_;
  }

  template <class T>
  void operator,(T const&) = delete;
};
static_assert(cuda::std::forward_iterator<forward_iterator<int*>>, "");
static_assert(!thrust::is_indirectly_trivially_relocatable_to<forward_iterator<int*>, int*>::value, "");

template <class It>
class sentinel_wrapper
{
public:
  explicit sentinel_wrapper() = default;
  __host__ __device__ constexpr explicit sentinel_wrapper(const It& it)
      : base_(base(it))
  {}
  __host__ __device__ friend constexpr bool operator==(const sentinel_wrapper& s, const It& i)
  {
    return s.base_ == base(i);
  }
#if TEST_STD_VER < 2020
  __host__ __device__ friend constexpr bool operator==(const It& i, const sentinel_wrapper& s)
  {
    return s.base_ == base(i);
  }
  __host__ __device__ friend constexpr bool operator!=(const sentinel_wrapper& s, const It& i)
  {
    return s.base_ != base(i);
  }
  __host__ __device__ friend constexpr bool operator!=(const It& i, const sentinel_wrapper& s)
  {
    return s.base_ != base(i);
  }
#endif
  __host__ __device__ friend constexpr It base(const sentinel_wrapper& s)
  {
    return It(s.base_);
  }

private:
  decltype(base(cuda::std::declval<It>())) base_;
};

template <class It>
class sized_sentinel
{
public:
  explicit sized_sentinel() = default;
  __host__ __device__ constexpr explicit sized_sentinel(const It& it)
      : base_(base(it))
  {}
  __host__ __device__ friend constexpr bool operator==(const sized_sentinel& s, const It& i)
  {
    return s.base_ == base(i);
  }
#if TEST_STD_VER < 2020
  __host__ __device__ friend constexpr bool operator==(const It& i, const sized_sentinel& s)
  {
    return s.base_ == base(i);
  }
  __host__ __device__ friend constexpr bool operator!=(const sized_sentinel& s, const It& i)
  {
    return s.base_ != base(i);
  }
  __host__ __device__ friend constexpr bool operator!=(const It& i, const sized_sentinel& s)
  {
    return s.base_ != base(i);
  }
#endif
  __host__ __device__ friend constexpr auto operator-(const sized_sentinel& s, const It& i)
  {
    return s.base_ - base(i);
  }
  __host__ __device__ friend constexpr auto operator-(const It& i, const sized_sentinel& s)
  {
    return base(i) - s.base_;
  }
  __host__ __device__ friend constexpr It base(const sized_sentinel& s)
  {
    return It(s.base_);
  }

private:
  decltype(base(cuda::std::declval<It>())) base_;
};

template <class T, size_t Capacity>
struct uncommon_range
{
  cuda::std::array<T, Capacity> data;
  forward_iterator<T*> end_{data.data() + Capacity};

  __host__ __device__ constexpr forward_iterator<T*> begin() noexcept
  {
    return forward_iterator<T*>{data.begin()};
  }

  __host__ __device__ constexpr sentinel_wrapper<forward_iterator<T*>> end() noexcept
  {
    return sentinel_wrapper<forward_iterator<T*>>{end_};
  }
};
static_assert(cuda::std::ranges::forward_range<uncommon_range<int, 4>>);
static_assert(!cuda::std::ranges::common_range<uncommon_range<int, 4>>);
static_assert(!cuda::std::ranges::sized_range<uncommon_range<int, 4>>);

template <class T, size_t Capacity>
struct sized_uncommon_range
{
  cuda::std::array<T, Capacity> data;
  forward_iterator<T*> end_{data.data() + Capacity};

  __host__ __device__ constexpr forward_iterator<T*> begin() noexcept
  {
    return forward_iterator<T*>{data.begin()};
  }

  __host__ __device__ constexpr sized_sentinel<forward_iterator<T*>> end() noexcept
  {
    return sized_sentinel<forward_iterator<T*>>{end_};
  }
};
static_assert(cuda::std::ranges::forward_range<sized_uncommon_range<int, 4>>);
static_assert(!cuda::std::ranges::common_range<sized_uncommon_range<int, 4>>);
static_assert(cuda::std::ranges::sized_range<sized_uncommon_range<int, 4>>);

#endif // CUDAX_TEST_CONTAINER_VECTOR_TYPES_H
