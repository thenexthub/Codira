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

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-macro-usage,
//             cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//             readability-else-after-return, readability-duplicate-include,
//             misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//             google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//             modernize-use-auto, llvm-namespace-comment,
//             cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//             readability-implicit-bool-conversion)

#ifndef COMMON_INTERFACES_OBJECTS_TRAITS_H
#define COMMON_INTERFACES_OBJECTS_TRAITS_H

#include "common_interfaces/objects/base_object.h"
#include <type_traits>
#include <vector>

namespace common::objects_traits {

template <typename U>
constexpr bool is_heap_object_v = std::is_base_of_v<BaseObject, std::remove_pointer_t<U>>;

// WriteBarrier: void (void*, size_t, U)
template <typename F>
constexpr bool is_write_barrier_callable_v =
    std::is_invocable_r_v<void, F, void*, size_t, BaseObject*>;

// ReadBarrier: U (void*, size_t)
template <typename F, typename U>
constexpr bool is_read_barrier_callable_v =
    is_heap_object_v<U> &&
    std::is_invocable_v<F, void*, size_t> &&
    std::is_convertible_v<std::invoke_result_t<F, void*, size_t>, U>;

// Allocator: U (size_t, CommonType)
template <typename F, typename U>
constexpr bool is_allocate_callable_v =
    is_heap_object_v<U> && std::is_invocable_r_v<U, F, size_t, ObjectType>;

// ---- enable_if_is_* traits ----
template <typename F>
using enable_if_is_write_barrier = std::enable_if_t<is_write_barrier_callable_v<F>, int>;

template <typename F, typename U>
using enable_if_is_read_barrier = std::enable_if_t<is_read_barrier_callable_v<F, U>, int>;

template <typename F, typename U>
using enable_if_is_allocate = std::enable_if_t<is_allocate_callable_v<F, U>, int>;

template <typename Container, typename T>
struct is_std_vector_of : std::false_type {};

template <typename Alloc, typename T>
struct is_std_vector_of<std::vector<T, Alloc>, T> : std::true_type {};

template <typename Container, typename T>
constexpr bool is_std_vector_of_v = is_std_vector_of<Container, T>::value;


template <typename Vec>
using get_allocator_type_t = typename std::decay_t<Vec>::allocator_type;

template <typename Alloc, typename U>
using rebind_alloc_t = typename std::allocator_traits<Alloc>::template rebind_alloc<U>;

template <typename Vec, typename NewT>
using vector_with_same_alloc_t =
    std::vector<NewT, rebind_alloc_t<get_allocator_type_t<Vec>, NewT>>;


} // namespace common::objects_traits


#endif //COMMON_INTERFACES_OBJECTS_TRAITS_H
// NOLINTEND(readability-identifier-naming, cppcoreguidelines-macro-usage,
//           cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//           readability-else-after-return, readability-duplicate-include,
//           misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//           google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//           modernize-use-auto, llvm-namespace-comment,
//           cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//           readability-implicit-bool-conversion)

