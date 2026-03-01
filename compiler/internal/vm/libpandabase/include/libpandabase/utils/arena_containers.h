/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef LIBPANDABASE_UTILS_ARENA_CONTAINERS_H
#define LIBPANDABASE_UTILS_ARENA_CONTAINERS_H

#include <deque>
#include <list>
#include <stack>
#include <queue>
#include <vector>
#include <set>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "mem/arena_allocator.h"
#include "mem/arena_allocator_stl_adapter.h"

namespace panda {

template <class T, bool use_oom_handler = false>
using ArenaVector = std::vector<T, ArenaAllocatorAdapter<T, use_oom_handler>>;
template <class T, bool use_oom_handler = false>
using ArenaDeque = std::deque<T, ArenaAllocatorAdapter<T, use_oom_handler>>;
template <class T, bool use_oom_handler = false, class ArenaContainer = ArenaDeque<T, use_oom_handler>>
using ArenaStack = std::stack<T, ArenaContainer>;
template <class T, bool use_oom_handler = false, class ArenaContainer = ArenaDeque<T, use_oom_handler>>
using ArenaQueue = std::queue<T, ArenaContainer>;
template <class T, bool use_oom_handler = false>
using ArenaList = std::list<T, ArenaAllocatorAdapter<T, use_oom_handler>>;
template <class Key, class Compare = std::less<Key>, bool use_oom_handler = false>
using ArenaSet = std::set<Key, Compare, ArenaAllocatorAdapter<Key, use_oom_handler>>;
template <class Key, class T, class Compare = std::less<Key>, bool use_oom_handler = false>
using ArenaMap = std::map<Key, T, Compare, ArenaAllocatorAdapter<std::pair<const Key, T>, use_oom_handler>>;
template <class Key, class T, class Compare = std::less<Key>, bool use_oom_handler = false>
using ArenaMultiMap = std::multimap<Key, T, Compare, ArenaAllocatorAdapter<std::pair<const Key, T>, use_oom_handler>>;
template <class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>,
          bool use_oom_handler = false>
using ArenaUnorderedMultiMap =
    std::unordered_multimap<Key, T, Hash, KeyEqual, ArenaAllocatorAdapter<std::pair<const Key, T>, use_oom_handler>>;
template <class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>,
          bool use_oom_handler = false>
using ArenaUnorderedMap =
    std::unordered_map<Key, T, Hash, KeyEqual, ArenaAllocatorAdapter<std::pair<const Key, T>, false>>;
template <class Key1, class Key2, class T, bool use_oom_handler = false>
using ArenaDoubleUnorderedMap =
    ArenaUnorderedMap<Key1, ArenaUnorderedMap<Key2, T, std::hash<Key2>, std::equal_to<Key2>, use_oom_handler>,
                      std::hash<Key1>, std::equal_to<Key1>, use_oom_handler>;
template <class Key, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>, bool use_oom_handler = false>
using ArenaUnorderedSet = std::unordered_set<Key, Hash, KeyEqual, ArenaAllocatorAdapter<Key, use_oom_handler>>;
template <bool use_oom_handler = false>
using ArenaStringT = std::basic_string<char, std::char_traits<char>, ArenaAllocatorAdapter<char, use_oom_handler>>;
using ArenaString = ArenaStringT<false>;

}  // namespace panda

#endif  // LIBPANDABASE_UTILS_ARENA_CONTAINERS_H
