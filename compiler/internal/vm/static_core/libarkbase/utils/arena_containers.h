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

#ifndef LIBPANDABASE_UTILS_ARENA_CONTAINERS_H_
#define LIBPANDABASE_UTILS_ARENA_CONTAINERS_H_

#include <deque>
#include <list>
#include <forward_list>
#include <stack>
#include <queue>
#include <vector>
#include <set>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "libarkbase/mem/arena_allocator.h"
#include "libarkbase/mem/arena_allocator_stl_adapter.h"

namespace ark {

template <class T, bool USE_OOM_HANDLER = false>
using ArenaVector = std::vector<T, ArenaAllocatorAdapter<T, USE_OOM_HANDLER>>;
template <class T, bool USE_OOM_HANDLER = false>
using ArenaDeque = std::deque<T, ArenaAllocatorAdapter<T, USE_OOM_HANDLER>>;
template <class T, bool USE_OOM_HANDLER = false, class ArenaContainer = ArenaDeque<T, USE_OOM_HANDLER>>
using ArenaStack = std::stack<T, ArenaContainer>;
template <class T, bool USE_OOM_HANDLER = false, class ArenaContainer = ArenaDeque<T, USE_OOM_HANDLER>>
using ArenaQueue = std::queue<T, ArenaContainer>;
template <class T, bool USE_OOM_HANDLER = false>
using ArenaList = std::list<T, ArenaAllocatorAdapter<T, USE_OOM_HANDLER>>;
template <class T, bool USE_OOM_HANDLER = false>
using ArenaForwardList = std::forward_list<T, ArenaAllocatorAdapter<T, USE_OOM_HANDLER>>;
template <class Key, class Compare = std::less<Key>, bool USE_OOM_HANDLER = false>
using ArenaSet = std::set<Key, Compare, ArenaAllocatorAdapter<Key, USE_OOM_HANDLER>>;
template <class Key, class T, class Compare = std::less<Key>, bool USE_OOM_HANDLER = false>
using ArenaMap = std::map<Key, T, Compare, ArenaAllocatorAdapter<std::pair<const Key, T>, USE_OOM_HANDLER>>;
template <class Key, class T, class Compare = std::less<Key>, bool USE_OOM_HANDLER = false>
using ArenaMultiMap = std::multimap<Key, T, Compare, ArenaAllocatorAdapter<std::pair<const Key, T>, USE_OOM_HANDLER>>;
template <class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>,
          bool USE_OOM_HANDLER = false>
using ArenaUnorderedMultiMap =
    std::unordered_multimap<Key, T, Hash, KeyEqual, ArenaAllocatorAdapter<std::pair<const Key, T>, USE_OOM_HANDLER>>;
template <class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>,
          bool USE_OOM_HANDLER = false>
using ArenaUnorderedMap =
    std::unordered_map<Key, T, Hash, KeyEqual, ArenaAllocatorAdapter<std::pair<const Key, T>, false>>;
template <class Key1, class Key2, class T, bool USE_OOM_HANDLER = false>
using ArenaDoubleUnorderedMap =
    ArenaUnorderedMap<Key1, ArenaUnorderedMap<Key2, T, std::hash<Key2>, std::equal_to<Key2>, USE_OOM_HANDLER>,
                      std::hash<Key1>, std::equal_to<Key1>, USE_OOM_HANDLER>;
template <class Key, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>, bool USE_OOM_HANDLER = false>
using ArenaUnorderedSet = std::unordered_set<Key, Hash, KeyEqual, ArenaAllocatorAdapter<Key, USE_OOM_HANDLER>>;
template <bool USE_OOM_HANDLER = false>
using ArenaStringT = std::basic_string<char, std::char_traits<char>, ArenaAllocatorAdapter<char, USE_OOM_HANDLER>>;
using ArenaString = ArenaStringT<false>;

}  // namespace ark

#endif  // LIBPANDABASE_UTILS_ARENA_CONTAINERS_H_
