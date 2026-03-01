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

#ifndef PANDA_VERIF_PARSER_CALLABLE_H_
#define PANDA_VERIF_PARSER_CALLABLE_H_

#include <utility>
#include <tuple>

namespace ark::verifier {
/*

this is lightweight analogue of std::function.
Type-erased holder of function, closure or object with operator().

NB! it does not keep object contents, only pointers, so
a closure must be accessible during lifetime of
the callable<...> object.

Motivation: in many cases using type-erased callable object
based on std::function is very heavy-weight: extra allocations,
data copying, vtbls  and so on.
So here is lightweight counterpart, using static type-erasing
without extra storage other than two explicit private pointers.
*/

template <typename Signature>
class callable;

template <typename R, typename... Args>
class callable<R(Args...)> {
    struct CallableType {
        R operator()(Args...);
    };
    using MethodType = R (CallableType::*)(Args...);
    using FunctionType = R (*)(Args...);
    CallableType *object_ {nullptr};
    union MethodUnion {
        MethodType m {nullptr};
        FunctionType f;
        explicit MethodUnion(MethodType method) : m(method) {}
        explicit MethodUnion(FunctionType function) : f(function) {}
        MethodUnion() = default;
    } method_;

public:
    using Result = R;
    using Arguments = std::tuple<Args...>;

    callable() = default;
    callable(const callable &) = default;
    callable(callable &&) = default;
    callable &operator=(const callable &) = default;  // NOLINT(cppcoreguidelines-pro-type-union-access)
    callable &operator=(callable &&) = default;       // NOLINT(cppcoreguidelines-pro-type-union-access)
    ~callable() = default;

    template <typename T, typename = decltype(static_cast<R (T::*)(Args...) const>(&T::operator()))>
    constexpr callable(const T &obj)  // NOLINT(google-explicit-constructor)
        : object_ {reinterpret_cast<CallableType *>(&const_cast<T &>(obj))},
          method_ {reinterpret_cast<MethodType>(static_cast<R (T::*)(Args...) const>(&T::operator()))}
    {
    }

    template <typename T, typename = decltype(static_cast<R (T::*)(Args...)>(&T::operator()))>
    constexpr callable(T &obj)  // NOLINT(google-explicit-constructor)
        : object_ {reinterpret_cast<CallableType *>(&const_cast<T &>(obj))},
          method_ {reinterpret_cast<MethodType>(static_cast<R (T::*)(Args...)>(&T::operator()))}
    {
    }

    template <typename T>
    constexpr callable(const T &obj, R (T::*paramMethod)(Args...) const)
        : object_ {reinterpret_cast<CallableType *>(&const_cast<T &>(obj))},
          method_ {reinterpret_cast<MethodType>(paramMethod)}
    {
    }

    template <typename T>
    constexpr callable(T &obj, R (T::*paramMethod)(Args...))
        : object_ {reinterpret_cast<CallableType *>(&const_cast<T &>(obj))},
          method_ {reinterpret_cast<MethodType>(paramMethod)}
    {
    }

    constexpr callable(FunctionType func) : method_ {func} {}  // NOLINT(google-explicit-constructor)

    constexpr R operator()(Args... args) const
    {
        if (object_ == nullptr) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            return (method_.f)(args...);
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return (object_->*(method_.m))(args...);
    }

    operator bool() const  // NOLINT(google-explicit-constructor)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return (method_.m != nullptr) || (method_.f != nullptr);
    }
};
}  // namespace ark::verifier

#endif  // PANDA_VERIF_PARSER_CALLABLE_H_
