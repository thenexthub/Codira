/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_VERIFIER_UTIL_FUNCTION_TRAITS_
#define PANDA_VERIFIER_UTIL_FUNCTION_TRAITS_

#include <tuple>

namespace ark::verifier {
template <int...>
struct Indices {
};

template <int N, int... S>
struct GenIndices : GenIndices<N - 1, N - 1, S...> {
};

template <int... S>
struct GenIndices<0, S...> {
    using Type = Indices<S...>;
};

template <class F>
struct FunctionSignatureHelper;

template <typename R, typename... Args>
struct FunctionSignatureHelper<R (*)(Args...)> : public FunctionSignatureHelper<R(Args...)> {
};

template <typename R, typename F, typename... Args>
struct FunctionSignatureHelper<R (F::*)(Args...) const> : public FunctionSignatureHelper<R(Args...)> {
};

template <typename R, typename F, typename... Args>
struct FunctionSignatureHelper<R (F::*)(Args...)> : public FunctionSignatureHelper<R(Args...)> {
};

template <typename R, typename... Args>
struct FunctionSignatureHelper<R(Args...)> {
    using ReturnType = R;
    using ArgsTuple = std::tuple<Args...>;

    static constexpr std::size_t ARITY = sizeof...(Args);

    template <std::size_t N>
    struct Argument {
        static_assert(N < ARITY, "invalid argument index");
        using Type = std::tuple_element_t<N, ArgsTuple>;
    };

    template <std::size_t N>
    using ArgType = typename Argument<N>::Type;
};

template <typename F>
class FunctionSignature : public FunctionSignatureHelper<decltype(&F::operator())> {
    using Base = FunctionSignatureHelper<decltype(&F::operator())>;

public:
    template <typename L, typename... Args>
    static typename Base::ReturnType Call(L f, std::tuple<Args...> &&args)
    {
        return call_helper(f, std::forward(args), typename GenIndices<sizeof...(Args)>::RetType {});
    }

private:
    template <typename L, typename A, int... S>
    static typename Base::ReturnType CallHelper(L f, A &&a, Indices<S...> /* unused */)
    {
        return f(std::forward(std::get<S>(a))...);
    }
};

template <typename BinOp>
class NAry {
public:
    using Sig = FunctionSignature<BinOp>;

    using RetType = std::decay_t<typename Sig::ReturnType>;
    using LhsType = std::decay_t<typename Sig::template Argument<0>::Type>;
    using RhsType = std::decay_t<typename Sig::template Argument<1>::Type>;
    static_assert(Sig::ARITY == 0x2, "only binary operation is expected");
    static_assert(std::is_same<LhsType, RhsType>::value, "argument types should be the same");
    static_assert(std::is_same<RetType, LhsType>::value, "return value type should be the same as arguments one");

    using Type = RetType;

    explicit NAry(BinOp op) : binop_ {op} {}

    auto operator()(Type lhs, Type rhs)
    {
        return binop_(lhs, rhs);
    }

    template <typename... Args>
    auto operator()(Type lhs, Args &&...args)
    {
        return binop_(lhs, operator()(std::forward<Args>(args)...));
    }

    template <typename... Args>
    auto operator()(std::tuple<Args...> &&args)
    {
        return CallHelper(std::forward<std::tuple<Args...>>(args), typename GenIndices<sizeof...(Args)>::Type {});
    }

private:
    template <typename A, int... S>
    auto CallHelper(A &&a, Indices<S...> /* unused */)
    {
        return operator()(std::forward<std::tuple_element_t<S, A>>(std::get<S>(a))...);
    }

    BinOp binop_;
};
}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_UTIL_FUNCTION_TRAITS_
