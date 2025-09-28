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

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <tuple>
#include <typeinfo>
#include <unordered_map>

template <typename ResultT, typename CleanupCallable>
class result_wrapper_t
{
  std::shared_ptr<ResultT> m_owner;

public:
  result_wrapper_t()
      : m_owner{}
  {}
  result_wrapper_t(ResultT v)
      : m_owner{std::make_shared<ResultT>(v)}
  {}

  result_wrapper_t(const result_wrapper_t&) = default;
  result_wrapper_t(result_wrapper_t&&)      = default;

  result_wrapper_t& operator=(const result_wrapper_t&) = default;
  result_wrapper_t& operator=(result_wrapper_t&&)      = default;

  ~result_wrapper_t() noexcept
  {
    if (!m_owner)
    {
      return;
    }
    try
    {
      if (m_owner.use_count() <= 1)
      {
        // release resources
        CleanupCallable{}(m_owner.get());
      }
    }
    catch (const std::exception& e)
    {
      std::cerr << "~result_wrapper_t ignores exception: " << e.what() << std::endl;
    }
  }

  ResultT& get()
  {
    return *m_owner.get();
  }
};

template <typename KeyT, typename ValueT>
class build_cache_t
{
  std::unordered_map<KeyT, ValueT> m_map;

public:
  build_cache_t()
      : m_map{} {};

  bool contains(const KeyT& key) const
  {
    // unorder_map::contains is C++20 feature
    return m_map.contains(key);
  }

  void insert(const KeyT& key, ValueT&& new_value)
  {
    m_map[key] = std::move(new_value);
  }

  ValueT& get(const KeyT& key)
  {
    assert(m_map.contains(key));
    return m_map[key];
  }
};

template <typename T, typename Tag>
class fixture
{
public:
  using OptionalT = typename std::optional<T>;

private:
  OptionalT v;

  fixture()
      : v{T{}}
  {}

public:
  OptionalT& get_value()
  {
    return v;
  }

  static auto& get_or_create()
  {
    static fixture singleton{};
    return singleton;
  }
};

struct KeyBuilder
{
  static std::string bool_as_key(bool v)
  {
    return (v) ? std::string("T") : std::string("F");
  }

  template <typename T>
  static std::string type_as_key()
  {
    return typeid(T).name();
  }

  template <std::size_t N>
  static std::string join(const std::string (&collection)[N])
  {
    constexpr std::string_view delimiter = "-";
    std::stringstream ss;

    for (std::size_t i = 0; i < N; ++i)
    {
      ss << collection[i];
      if (i + 1 < N)
      {
        ss << delimiter;
      }
    }

    return ss.str();
  }
};

template <typename TupleLike, std::size_t I = 0>
void adder_helper(std::stringstream& ss)
{
  constexpr std::size_t S = std::tuple_size_v<TupleLike>;
  if constexpr (I < S)
  {
    using SelectedType       = std::tuple_element_t<I, TupleLike>;
    constexpr std::size_t In = I + 1;

    ss << KeyBuilder::type_as_key<SelectedType>();
    if constexpr (In < S)
    {
      ss << "-";
    }
    adder_helper<TupleLike, In>(ss);
  }
}

template <typename... Ts>
std::optional<std::string> make_key()
{
  std::stringstream ss{};
  adder_helper<std::tuple<Ts...>, 0>(ss);
  return std::make_optional(ss.str());
}
