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

template <typename... Types>
struct mixin_next
{};

template <typename T>
struct mixin_next<T> : public T
{
  template <typename Context>
  constexpr mixin_next(Context& c)
      : T(c)
  {}
};

template <typename T, typename... Types>
struct mixin_next<T, Types...>
    : public T
    , public mixin_next<Types...>
{
  template <typename Context>
  constexpr mixin_next(Context& c)
      : T(c)
      , mixin_next<Types...>(c)
  {}
};

template <typename... Types>
struct mixin_all : public mixin_next<Types...>
{
  constexpr auto operator->()
  {
    return this;
  }
};

/*
Context: The context type that will be passed as a reference to the underlying node(s)
Types: The mixins are inherited into a parent type - A single mixin can have multiple functions if desired.

Usage: node_list<Context, NodeA, NodeB,...>{std::move(context)}->|NodeA::fun1|NodeB::fun2|...
*/
template <typename Context, typename... Types>
struct node_list
{
private:
  Context c;

public:
  constexpr node_list() = default;
  constexpr node_list(Context&& c_)
      : c(std::forward<Context>(c_))
  {}
  constexpr node_list(const Context& c_)
      : c(c_)
  {}

  template <typename T = void>
  constexpr mixin_all<Types...> operator->()
  {
    return mixin_all<Types...>{c};
  }
};
