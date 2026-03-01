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

#ifndef CPP_ABCKIT_BASE_CONCEPTS_H
#define CPP_ABCKIT_BASE_CONCEPTS_H

#include <type_traits>

namespace abckit::traits {

#ifdef _MSC_VER
#define ABCKIT_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define ABCKIT_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif  // _MSC_VER

/**
 * @brief Lightweight checker that ensures core<->target API conversion validity
 * It is designed to be empty class type.
 * In case of inheritance the empty base optimization will be applied
 * (see https://en.cppreference.com/w/cpp/language/ebo for the details)
 * In case of non-static member, it should be combined with ABCKIT_NO_UNIQUE_ADDRESS attribute
 * @tparam TargetT - View class for a target specialized C API (not core)
 */
template <class TargetT>
class TargetCheckCast {
private:
    /**
     * @brief Provides compile-time static contract checks
     */
    static void ConceptChecks();

    /**
     * @brief Checks that T::TargetCast() method is defined using
     * pre-C++20 concept idiom with SFINAE and decltype to check type expression valideness
     * @tparam T - type that is inspected againts the contract
     * @note A general case to be matched (to false) if more specialized case fail
     */
    template <typename T, typename = void>
    struct HasTargetCastMethod : public std::false_type {
    };

    /**
     * @brief Checks that T::TargetCast() method is defined using
     * pre-C++20 concept idiom with SFINAE and decltype to check type expression valideness
     * @tparam T - type that is inspected againts the contract
     * @note A more specialized case to be matched (to true) if the type expression below is valid for type T
     */
    template <typename T>
    struct HasTargetCastMethod<T, std::void_t<decltype(std::declval<const T &>().TargetCast())>>
        : public std::true_type {
    };

    template <typename T, typename = void>
    struct HasCoreViewT : public std::false_type {
    };

    template <typename T>
    struct HasCoreViewT<T, std::void_t<typename T::CoreViewT>> : public std::true_type {
    };

public:
    /**
     * @brief Checking constructor, performs runtime check for target cast validity
     * @note requires access to TargetT::TargetCast, provide such visibility at TargetT class
     */
    explicit TargetCheckCast()
    {
        [[maybe_unused]] auto ret = static_cast<const TargetT *>(this)->TargetCast();
    };

    /**
     * @brief Checking constructor, performs runtime check for target cast validity
     * use like `explicit TagetT::TargetT(const CoreT& core) : CoreT(core), typeChecker(this) { }`
     * @param that - Target APP view pointer that will be target checked
     * @note requires access to TargetT::TargetCast, provide such visibility at TargetT class
     */
    explicit TargetCheckCast(const TargetT *that)
    {
        [[maybe_unused]] auto ret = that->TargetCast();
    };

    /**
     * @brief No restrictions on destuctor
     */
    ~TargetCheckCast() = default;

    /**
     * @brief No restrictions on copy constructor
     */
    TargetCheckCast(const TargetCheckCast &) = default;

    /**
     * @brief No restrictions on move constructor
     */
    TargetCheckCast(TargetCheckCast &&) = default;

    /**
     * @brief No restrictions on copy assignment constructor
     * @return this `TargetCheckCast` -- default chaining idiom
     */
    TargetCheckCast &operator=(const TargetCheckCast &) = default;

    /**
     * @brief No restrictions on move assignment constructor
     * @return this `TargetCheckCast` -- default chaining idiom
     */
    TargetCheckCast &operator=(TargetCheckCast &&) = default;
};

/**
 * @brief Checking constructor, performs runtime check for target cast validity
 * @tparam TargetT - View class for a target specialized C API
 * @note requires access to TargetT::TargetCast, provide such visibility at TargetT class
 */
template <class TargetT>
inline void TargetCheckCast<TargetT>::ConceptChecks()
{
    static_assert(sizeof(TargetCheckCast) == 0, "Concept check should not affect class layout");
    static_assert(HasTargetCastMethod<TargetT>::value, "Define `TargetT::TargetCast() const` returning C API pointer");
    static_assert(HasCoreViewT<TargetT>::value, "Define `TargetT::CoreViewT` type referring base class");
    static_assert(sizeof(TargetT) == sizeof(typename TargetT::CoreViewT),
                  "Target View should have the same layout as Core");
}

}  // namespace abckit::traits

#endif  // CPP_ABCKIT_BASE_CONCEPTS_H
