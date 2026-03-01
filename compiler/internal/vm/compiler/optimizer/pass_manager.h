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

#ifndef COMPILER_OPTIMIZER_PASS_MANAGER_H
#define COMPILER_OPTIMIZER_PASS_MANAGER_H

#include <tuple>
#include "compiler_options.h"
#include "pass.h"
#include "utils/bit_field.h"
#include "utils/arena_containers.h"
#include "pass_manager_statistics.h"

namespace panda::compiler {
class Graph;
class Pass;
class Analysis;
class LivenessAnalyzer;
class LoopAnalyzer;
class DominatorsTree;
class Rpo;
class LinearOrder;
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)

namespace details {
template <typename... Types>
class PassTypeList {
public:
    using IdentifierType = size_t;
    using TupleType = std::tuple<std::decay_t<Types>...>;

    template <typename T>
    static constexpr bool HasType()
    {
        return std::disjunction_v<std::is_same<T, Types>...>;
    }

    template <typename T, typename... Args>
    static ArenaVector<T> Instantiate(ArenaAllocator *allocator, Args &&... args)
    {
        ArenaVector<T> vec(allocator->Adapter());
        vec.reserve(sizeof...(Types));
        ((vec.push_back(allocator->New<Types>((std::forward<Args>(args))...))), ...);
        return vec;
    }

    template <typename Type, std::size_t... Indexes>
    static constexpr size_t GetIndex(std::index_sequence<Indexes... /* unused */>)
    {
        static_assert(HasType<Type>());
        return (0 + ... +
                (std::is_same_v<Type, std::tuple_element_t<Indexes, TupleType>> ? size_t(Indexes) : size_t {}));
    }

    template <typename Type>
    static constexpr IdentifierType ID = GetIndex<std::decay_t<Type>>(std::index_sequence_for<Types...> {});
    static constexpr size_t SIZE = sizeof...(Types);
};

using PredefinedAnalyses =
    PassTypeList<LivenessAnalyzer, LoopAnalyzer, DominatorsTree, Rpo, LinearOrder>;
}  // namespace details

class PassManager {
public:
    PassManager(Graph *graph, PassManager *parent_pm);

    ArenaAllocator *GetAllocator();
    ArenaAllocator *GetLocalAllocator();

    bool RunPass(Pass *pass, size_t local_mem_size_before_pass);

    template <typename T, typename... Args>
    bool RunPass(Args... args)
    {
        auto local_mem_size_before = GetLocalAllocator()->GetAllocatedSize();
        bool res = false;
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (details::PredefinedAnalyses::HasType<T>()) {
            static_assert(sizeof...(Args) == 0);
            res = RunPass(ANALYSES[details::PredefinedAnalyses::ID<T>], local_mem_size_before);
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            T pass(graph_, std::forward<Args>(args)...);
            res = RunPass(&pass, local_mem_size_before);
        }
        if (!IsCheckMode()) {
            if (options.IsCompilerResetLocalAllocator()) {
                ASSERT(GetLocalAllocator() != GetAllocator());
                GetLocalAllocator()->Resize(local_mem_size_before);
            }
        }
        return res;
    }

    template <typename T>
    T &GetAnalysis()
    {
        static_assert(std::is_base_of_v<Analysis, std::decay_t<T>>);
        return *static_cast<T *>(ANALYSES[details::PredefinedAnalyses::ID<T>]);
    }
    std::string GetFileName(const char *pass_name = nullptr, const std::string &suffix = ".cfg");
    void DumpGraph(const char *pass_name);
    void DumpLifeIntervals(const char *pass_name);

    Graph *GetGraph()
    {
        return graph_;
    }

    void Finalize() const;

    PassManagerStatistics *GetStatistics()
    {
        return stats_;
    }

    void SetCheckMode(bool v)
    {
        check_mode_ = v;
    }

    bool IsCheckMode() const
    {
        return check_mode_;
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    size_t GetExecutionCounter() const
    {
        return execution_counter;
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void StartExecution()
    {
        execution_counter++;
    }

private:
    Graph *graph_ {nullptr};
    ArenaVector<Optimization *> optimizations_;
    const ArenaVector<Analysis *> ANALYSES;

    PassManagerStatistics *stats_ {nullptr};
    inline static size_t execution_counter {0};

    // Whether passes are run by checker.
    bool check_mode_ {false};

    bool first_execution_ {true};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_PASS_MANAGER_H
