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
 * This file provide pattern match for AST Node.
 */

#ifndef CODIRA_BASIC_MATCH_H
#define CODIRA_BASIC_MATCH_H

#include <functional>

namespace Codira::Meta {
// The generic Signature falls back on the Signature of the call operator if present.
template <typename T> struct Signature : Signature<decltype(&std::decay_t<T>::operator())> {
};

// Pointer to member function fall back on the plain function Signatures.
template <typename T, typename Ret, typename... Args> struct Signature<Ret (T::*)(Args...)> : Signature<Ret(Args...)> {
};

// Pointer to member function (const) fall back on the plain function Signatures.
template <typename T, typename Ret, typename... Args>
struct Signature<Ret (T::*)(Args...) const> : Signature<Ret(Args...)> {
};

// Get Ith element of packed parameters.
template <size_t I, typename... Args> using IthElem = std::tuple_element_t<I, std::tuple<Args...>>;

// Actual implementation just for pure function Signature types.
template <typename Ret, typename... Args> struct Signature<Ret(Args...)> {
    using Type = Ret(Args...);
    template <size_t I> using Argument = IthElem<I, Args...>;
    using RetType = Ret;
    static constexpr size_t numArgs = sizeof...(Args);
};

// Return type of the function.
template <typename Callable> using RetT = typename Signature<std::decay_t<Callable>>::RetType;

// Check all the types whether same.
template <typename T, typename... Ts> using AreT = std::conjunction<std::is_same<T, Ts>...>;

// Matcher functor do the pattern match.
template <typename Node> class Matcher {
public:
    Matcher(Node& node) : node(node) {}
    ~Matcher(){};

    template <typename... Lambdas> auto operator()(Lambdas&&... lambdas) -> decltype(auto)
    {
        constexpr size_t numLambdas = sizeof...(Lambdas);
        // Check num of arguments.
        constexpr bool validNumArgs =
            (... && ((Signature<Lambdas>::numArgs == 0) or (Signature<Lambdas>::numArgs == 1)));
        static_assert(validNumArgs,
            "Can only match on lambdas with one argument(zero argument only in "
            "the default case).");
        constexpr int numArgs = (... + (Signature<Lambdas>::numArgs));
        static_assert(
            numArgs == numLambdas or numArgs == numLambdas - 1, "There can be only one default case in the match.");

        // Check return types match.
        using FirstRetType = RetT<IthElem<0, Lambdas...>>;
        static_assert(AreT<FirstRetType, RetT<Lambdas>...>::value, "All functions must have same return type.");

        return Invoke(lambdas...);
    }

private:
    // Invoke the function with 0 argument.
    template <typename Lambda>
    auto Invoke0(const Lambda& lambda) -> std::enable_if_t<Signature<Lambda>::numArgs == 0, RetT<Lambda>>
    {
        return lambda();
    }

    // Invoke the function with 1 argument.
    template <typename Lambda>
    auto Invoke0(Lambda& lambda) -> std::enable_if_t<Signature<Lambda>::numArgs == 1, RetT<Lambda>>
    {
        using TargetT = std::remove_reference_t<typename Signature<Lambda>::template Argument<0>>;
        // Add const if Type has const.
        if (auto* p = dynamic_cast<TargetT*>(&node)) {
            return std::invoke(lambda, *p);
        }
    }

    // Invoke the last lambda.
    template <typename Lambda> auto Invoke(Lambda& lambda) -> RetT<Lambda>
    {
        return Invoke0(lambda);
    }

    // Invoke all the lambdas.
    template <typename Lambda, typename... Lambdas>
    auto Invoke(const Lambda first, const Lambdas&... rest) -> RetT<Lambda>
    {
        // Check the default case should be the last one.
        constexpr int numFirstArgs = Signature<Lambda>::numArgs;
        constexpr size_t numRest = sizeof...(Lambdas);
        static_assert(
            numFirstArgs == 1 or (numFirstArgs == 0 and numRest == 0), "The default case should at the end of match.");
        using TargetT = std::remove_reference_t<typename Signature<Lambda>::template Argument<0>>;
        // Add const if Type has const.
        if (auto* p = dynamic_cast<TargetT*>(&node)) {
            return std::invoke(first, *p);
        } else {
            return Invoke(rest...);
        }
    }

    Node& node;
};

/**
 * The match is a dsl for AST Node pattern match. There are some constraint of the usage:
 * 1. All return types need to be same.
 * 2. The match case must have one argument.
 * 3. The defualt case(0 argument) must at the last of the match, it is optional.
 * @param node The Node reference.
 * ```
 * match(*n)(
 * [](Expr& e){},
 * [](Decl& d){},
 * ...
 * [](){}
 * );
 * ```
 */
template <typename Node> auto match(Node& node) -> decltype(auto)
{
    return Matcher(node);
}
}; // namespace Codira::Meta

#endif // CODIRA_BASIC_MATCH_H
