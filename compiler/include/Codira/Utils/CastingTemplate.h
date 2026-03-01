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

/**
 * @file
 *
 * This file defines template for type casting.
 */

#ifndef CODIRA_UTILS_CASTING_TEMPLATE_H
#define CODIRA_UTILS_CASTING_TEMPLATE_H

#include <type_traits>

#include "Codira/Utils/SafePointer.h"
#include "Codira/Utils/CheckUtils.h"

/**
 * Support customized casting template for 'AST::Node', 'AST::Ty', 'CHIRNode' types.
 * Mainly supply three template functions:
 * 1. 'Is<To*>(instance*)' or 'Is<To>(instance*)' or 'Is<To>(instance&)'
 *     return true if the given pointer is instance of the given type 'To'.
 * 2. 'StaticCast<To*>(instance*)' or 'StaticCast<To&>(instance)' or 'StaticCast<To>(instance)'
 *     return the casted value of the given pointer or reference (it will keep 'const' qualifier).
 * 3. 'DynamicCast<To*>(instance*)' or 'DynamicCast<To>(instance*)'
 *     return the runtime-casted pointer result of the given pointer (it will keep 'const' qualifier).
 */
namespace Codira {
// Do not add function in this template.
// NOTE: newly added type without customization will fail compiling.
template <typename To, typename Enable = void> struct TypeAs {
};

template <typename T> struct NodeType {
};

// Defined NodeType::kind register macro.
#define DEFINE_NODE_TYPE_KIND(TYPE, KIND)                                                                              \
    template <> struct NodeType<TYPE> {                                                                                \
        static const auto kind = KIND;                                                                                 \
    }

// Define type filter for intermidiate AST types.
template <typename T, typename... Args> struct ShouldInstantiate : public std::true_type {
};

template <typename T, typename U, typename... Args>
struct ShouldInstantiate<T, U, Args...>
    : public std::conditional_t<std::is_same_v<T, U>, std::false_type, ShouldInstantiate<T, Args...>> {
};

template <typename To, typename From> inline bool constexpr IsTypeOf(From node)
{
    using ToType = std::remove_pointer_t<To>;
    using FromType = std::remove_pointer_t<From>;
    if constexpr (std::is_base_of_v<ToType, FromType>) {
        return node != nullptr;
    } else {
        static_assert(std::is_base_of_v<FromType, ToType>, "casting irrelevant types");
        bool isSubType = node != nullptr && TypeAs<std::remove_cv_t<ToType>>::IsInstanceOf(*node);
#if defined(CMAKE_ENABLE_ASSERT) || !defined(NDEBUG)
        CODEC_ASSERT(isSubType == (dynamic_cast<const ToType*>(node) != nullptr) && "Error casting");
#endif
        return isSubType;
    }
}

template <typename To, typename From>
inline std::enable_if_t<!std::is_pointer_v<To> && !std::is_pointer_v<From>, bool> Is(const From& node)
{
    return IsTypeOf<To*, const From*>(&node);
}

template <typename To, typename From>
inline std::enable_if_t<!std::is_pointer_v<To> && std::is_pointer_v<From>, bool> Is(From node)
{
    return IsTypeOf<To*, From>(node);
}

template <typename To, typename From> inline std::enable_if_t<std::is_pointer_v<To>, bool> Is(From node)
{
    static_assert(std::is_pointer_v<From>);
    return IsTypeOf<To, From>(node);
}

template <typename To, typename From> inline bool Is(Ptr<From> node)
{
    return Is<To>(node.get());
}
template <typename To, typename From> bool Is(const OwnedPtr<From>& node)
{
    return Is<To>(node.get());
}

template <typename To, typename From> struct CastTo {
    using ToType = std::conditional_t<std::is_pointer_v<To>, std::remove_pointer_t<To>, std::remove_reference_t<To>>;
    using type = std::conditional_t<std::is_const_v<std::remove_pointer_t<From>>,
        std::conditional_t<std::is_pointer_v<From>, const ToType*, const ToType&>,
        std::conditional_t<std::is_pointer_v<From>, ToType*, ToType&>>;
};
template <typename To, typename From> using CastToT = typename CastTo<To, From>::type;

namespace Details {
template <class T, class U>
constexpr bool IsVirtualBaseImpl(...) { return true; }
template <class T, class U, std::void_t<decltype((const U*)(std::declval<T*>()))>* = nullptr>
constexpr bool IsVirtualBaseImpl(int) { return false; }
}
template <class T, class U>
struct IsVirtualBaseOf: public std::integral_constant<bool, std::is_base_of_v<T, U> &&
    Details::IsVirtualBaseImpl<T, U>(0) && !Details::IsVirtualBaseImpl<U, T>(0)> {};
template <class T, class U>
constexpr bool IsVirtualBaseOfV = IsVirtualBaseOf<T, U>::value;

// 'To' can be normal type or pointer type.
// 'From' type must be pointer type.
template <typename To, typename From> inline CastToT<To, From> DynamicCast(From node)
{
    static_assert(std::is_pointer_v<From> && !std::is_reference_v<To>);
    if (!IsTypeOf<To>(node)) {
        return nullptr;
    }
    if constexpr (IsVirtualBaseOfV<std::remove_pointer_t<std::remove_cv_t<From>>, std::remove_pointer_t<To>>) {
        return dynamic_cast<CastToT<To, From>>(node);
    } else {
        return static_cast<CastToT<To, From>>(node);
    }
}

// 'To' can be normal type or pointer type.
// 'From' type must be pointer type.
template <typename To, typename From> inline CastToT<To, From*> DynamicCast(Ptr<From> node)
{
    static_assert(!std::is_reference_v<To>);
    if (!IsTypeOf<To>(node.get())) {
        return nullptr;
    }
    if constexpr (IsVirtualBaseOfV<std::remove_pointer_t<std::remove_cv_t<From>>, std::remove_pointer_t<To>>) {
        return dynamic_cast<CastToT<To, From*>>(node.get());
    } else {
        return static_cast<CastToT<To, From*>>(node.get());
    }
}

///@{
/**
 * Cast from virtual base class to derived class. \ref VirtualCast asserts the class must succeed. Otherwise, use
 * \ref DynamicCast
 */
template <typename To, typename From>
CastToT<To, From*> VirtualCast(From* node)
{
    static_assert(IsVirtualBaseOfV<std::remove_pointer_t<std::remove_cv_t<From>>, std::remove_pointer_t<To>>);
    auto r = dynamic_cast<CastToT<To, From*>>(node);
    CODEC_ASSERT(!node || r);
    return r;
}
template <typename To, typename From>
CastToT<To, From*> VirtualCast(Ptr<From> node)
{
    static_assert(IsVirtualBaseOfV<std::remove_pointer_t<std::remove_cv_t<From>>, std::remove_pointer_t<To>>);
    auto r = dynamic_cast<CastToT<To, From*>>(node.get());
    CODEC_ASSERT(!node || r);
    return r;
}
template <typename To, typename From>
CastToT<To, From> VirtualCast(From& node)
{
    static_assert(IsVirtualBaseOfV<From, std::remove_reference_t<To>>);
    return dynamic_cast<CastToT<To, From>>(node);
}
///@}

// 'From' can only be pointer type with or without 'const'.
// 'To' can be normal type or pointer type.
template <typename To, typename From>
inline std::enable_if_t<std::is_pointer_v<From>, CastToT<To, From>> StaticCast(From node)
{
    static_assert(std::is_pointer_v<From> && !std::is_reference_v<To>);
#if defined(CMAKE_ENABLE_ASSERT) || !defined(NDEBUG)
    auto ptr = dynamic_cast<CastToT<To, From>>(node);
    CODEC_ASSERT(ptr != nullptr && "Error casting");
#endif
    static_assert(!IsVirtualBaseOfV<std::remove_pointer_t<std::remove_cv_t<From>>, std::remove_pointer_t<To>>,
        "static_cast cannot cast via virtual base, use dynamic_cast instead.");
    return static_cast<CastToT<To, From>>(node);
}

// 'From' can only be normal type with or without 'const' (reference is defined in parameter).
// 'To' can be normal type or reference type.
template <typename To, typename From>
inline std::enable_if_t<!std::is_pointer_v<From>, CastToT<To, From>> StaticCast(From& node)
{
    static_assert(!std::is_pointer_v<From> && !std::is_pointer_v<To>);
#if (defined(CMAKE_ENABLE_ASSERT) || !defined(NDEBUG)) && !defined(__APPLE__)
    auto ptr = dynamic_cast<CastToT<To, From*>>(&node);
    CODEC_ASSERT(ptr != nullptr && "Error casting");
#endif
    static_assert(!IsVirtualBaseOfV<std::remove_cv_t<From>, std::remove_reference_t<To>>,
        "static_cast cannot cast via virtual base, use dynamic_cast instead.");
    return static_cast<CastToT<To, From>>(node);
}

template <typename To, typename From>
inline CastToT<To, From*> StaticCast(Ptr<From> node)
{
    static_assert(!std::is_reference_v<To>);
#if (defined(CMAKE_ENABLE_ASSERT) || !defined(NDEBUG)) && !defined(__APPLE__)
    auto ptr = dynamic_cast<CastToT<To, From*>>(node.get());
    CODEC_ASSERT(ptr != nullptr && "Error casting");
#endif
    static_assert(!IsVirtualBaseOfV<std::remove_pointer_t<std::remove_cv_t<From>>, std::remove_pointer_t<To>>,
        "static_cast cannot cast via virtual base, use dynamic_cast instead.");
    return static_cast<CastToT<To, From*>>(node.get());
}

template <typename To, typename From> inline To RawStaticCast(From src)
{
    return static_cast<To>(src);
}

template <typename To, typename From> inline To RawStaticCast(Ptr<From> src)
{
    return static_cast<To>(src.get());
}
} // namespace Codira

#endif // CODIRA_UTILS_CASTING_TEMPLATE_H
