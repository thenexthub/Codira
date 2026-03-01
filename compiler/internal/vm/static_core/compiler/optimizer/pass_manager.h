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
#include "libarkbase/utils/bit_field.h"
#include "libarkbase/utils/arena_containers.h"
#include "pass_manager_statistics.h"

namespace ark::compiler {
class Graph;
class Pass;
class Analysis;
class CatchInputs;
class LivenessAnalyzer;
class LiveRegisters;
class LoopAnalyzer;
class AliasAnalysis;
class DominatorsTree;
class Rpo;
class LinearOrder;
class BoundsAnalysis;
class MonitorAnalysis;
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ObjectTypePropagation;
class RegAllocVerifier;
class TypesAnalysis;

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
    static ArenaVector<T> Instantiate(ArenaAllocator *allocator, Args &&...args)
    {
        ArenaVector<T> vec(allocator->Adapter());
        vec.reserve(sizeof...(Types));
        ((vec.push_back(allocator->New<Types>((std::forward<Args>(args))...))), ...);
        return vec;
    }

    template <typename Type, std::size_t... INDEXES>
    static constexpr size_t GetIndex(std::index_sequence<INDEXES...> /* unused */)
    {
        static_assert(HasType<Type>());
        return (0 + ... +
                (std::is_same_v<Type, std::tuple_element_t<INDEXES, TupleType>> ? size_t(INDEXES) : size_t {}));
    }

    template <typename Type>
    static constexpr IdentifierType ID = GetIndex<std::decay_t<Type>>(std::index_sequence_for<Types...> {});
    static constexpr size_t SIZE = sizeof...(Types);
};

using PredefinedAnalyses =
    PassTypeList<LivenessAnalyzer, LoopAnalyzer, AliasAnalysis, DominatorsTree, Rpo, LinearOrder, BoundsAnalysis,
                 MonitorAnalysis, LiveRegisters, ObjectTypePropagation, RegAllocVerifier, TypesAnalysis, CatchInputs>;
}  // namespace details

class PassManager {
public:
    PANDA_PUBLIC_API PassManager(Graph *graph, PassManager *parentPm);

    PANDA_PUBLIC_API ArenaAllocator *GetAllocator();
    PANDA_PUBLIC_API ArenaAllocator *GetLocalAllocator();

    PANDA_PUBLIC_API bool RunPass(Pass *pass, size_t localMemSizeBeforePass);

    template <typename T, typename... Args>
    bool RunPass(Args... args)
    {
        auto localMemSizeBefore = GetLocalAllocator()->GetAllocatedSize();
        bool res = false;
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (details::PredefinedAnalyses::HasType<T>()) {
            static_assert(sizeof...(Args) == 0);
            res = RunPass(analyses_[details::PredefinedAnalyses::ID<T>], localMemSizeBefore);
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            T pass(graph_, std::forward<Args>(args)...);
            res = RunPass(&pass, localMemSizeBefore);
        }
        if (!IsCheckMode()) {
            if (g_options.IsCompilerResetLocalAllocator()) {
                ASSERT(GetLocalAllocator() != GetAllocator());
                GetLocalAllocator()->Resize(localMemSizeBefore);
            }
        }
        return res;
    }

    template <typename T>
    T &GetAnalysis()
    {
        static_assert(std::is_base_of_v<Analysis, std::decay_t<T>>);
        return *static_cast<T *>(analyses_[details::PredefinedAnalyses::ID<T>]);
    }
    std::string GetFileName(const char *passName = nullptr, const std::string &suffix = ".cfg");
    void DumpGraph(const char *passName);
    void DumpLifeIntervals(const char *passName);
    void InitialDumpVisualizerGraph();
    void DumpVisualizerGraph(const char *passName);
    template <bool FORCE_RUN>
    bool RunPassChecker(Pass *pass, bool result, bool isCodegen);
    bool RunPassChecker(Pass *pass, bool result, bool isCodegen);

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
        checkMode_ = v;
    }

    bool IsCheckMode() const
    {
        return checkMode_;
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    size_t GetExecutionCounter() const
    {
        return executionCounter_;
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void StartExecution()
    {
        executionCounter_++;
    }

private:
    Graph *graph_ {nullptr};
    ArenaVector<Optimization *> optimizations_;
    const ArenaVector<Analysis *> analyses_;

    PassManagerStatistics *stats_ {nullptr};
    inline static size_t executionCounter_ {0};

    // Whether passes are run by checker.
    bool checkMode_ {false};

    bool firstExecution_ {true};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_PASS_MANAGER_H
