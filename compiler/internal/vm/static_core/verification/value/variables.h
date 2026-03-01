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

#ifndef PANDA_VERIFIER_VARIABLES_HPP
#define PANDA_VERIFIER_VARIABLES_HPP

#include "verification/util/lazy.h"

#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"

#include "util/str.h"
#include "verification/util/obj_pool.h"

#include <memory>

namespace ark::verifier {

class Variables {
    struct Wrapper {
        static void Init(Wrapper &v, size_t idx)
        {
            v.idx = idx;
        }
        size_t idx;
    };

    using PoolType = ObjPool<Wrapper, PandaVector, void (*)(Wrapper &, size_t)>;

public:
    class Var {
    public:
        Var() = default;
        // NOLINTNEXTLINE(google-explicit-constructor)
        Var(PoolType::Accessor &&a) : accessor_ {std::move(a)} {}
        Var(const Var &) = default;
        Var(Var &&) = default;
        Var &operator=(const Var &a) = default;
        Var &operator=(Var &&a) = default;
        ~Var() = default;

        bool operator==(const Var &v)
        {
            return (*accessor_).idx == (*v.accessor_).idx;
        }

        bool operator!=(const Var &v)
        {
            return (*accessor_).idx != (*v.accessor_).idx;
        }

        PandaString Image(const PandaString &prefix = "V") const
        {
            return prefix + NumToStr((*accessor_).idx);
        }

    private:
        PoolType::Accessor accessor_;
    };

    using VarIdx = size_t;

    Variables() = default;
    NO_COPY_SEMANTIC(Variables);
    DEFAULT_MOVE_SEMANTIC(Variables);
    ~Variables() = default;

    Var NewVar()
    {
        return {varPool_.New()};
    }

    size_t AmountOfUsedVars() const
    {
        return varPool_.Count() - varPool_.FreeCount();
    }

    auto AllVariables()
    {
        return [fetcher = varPool_.AllObjects()]() mutable -> std::optional<Var> {
            if (auto v = fetcher()) {
                return {Var {std::move(*v)}};
            }
            return std::nullopt;
        };
    }

private:
    PoolType varPool_ {Wrapper::Init};
};

}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_VARIABLES_HPP
