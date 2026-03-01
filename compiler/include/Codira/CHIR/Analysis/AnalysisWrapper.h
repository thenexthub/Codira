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

#ifndef CODIRA_CHIR_ANALYSIS_ANALYSISWRAPPER_H
#define CODIRA_CHIR_ANALYSIS_ANALYSISWRAPPER_H

#include "Codira/CHIR/Analysis/Engine.h"
#include "Codira/CHIR/Package.h"
#include "Codira/Utils/TaskQueue.h"

#include <future>

namespace Codira::CHIR {

/// template false type of value analysis.
template <typename T, typename U = void> struct IsValueAnalysis : std::false_type {};

/// template true type of value analysis.
template <typename T> struct IsValueAnalysis<T, std::void_t<typename T::isValueAnalysis>> : std::true_type {};

/**
 * @brief wrapper class of analysis pass, using to do parallel or check works.
 * @tparam TAnalysis analysis to wrapper.
 * @tparam TDomain domain of analysis.
 */
template <typename TAnalysis, typename TDomain,
    typename = std::enable_if_t<std::is_base_of_v<AbstractDomain<TDomain>, TDomain>>,
    typename = std::enable_if_t<std::is_base_of_v<Analysis<TDomain>, TAnalysis>>>
class AnalysisWrapper {
public:
    /**
     * @brief abstract class for CHIR analysis wrapper.
     * @param builder CHIR builder for generating IR.
     */
    explicit AnalysisWrapper(CHIRBuilder& builder) : builder(builder)
    {
    }

    /**
     * @brief main method to analysis from wrapper class.
     * @tparam Args the args type of analysis.
     * @param package package to do optimization.
     * @param isDebug flag whether print debug log.
     * @param threadNum thread num to do analysis
     * @param args args of analysis
     */
    template <typename... Args>
    void RunOnPackage(const Package* package, bool isDebug, size_t threadNum, Args&&... args)
    {
        if (threadNum == 1) {
            RunOnPackageInSerial(package, isDebug, std::forward<Args>(args)...);
        } else {
            RunOnPackageInParallel(package, isDebug, threadNum, std::forward<Args>(args)...);
        }
    }

    /**
     * @brief main method to analysis from wrapper class per function.
     * @tparam Args the args type of analysis.
     * @param func function CHIR IR to do optimization.
     * @param isDebug flag whether print debug log.
     * @param args args of analysis
     * @return result of analysis per function
     */
    template <typename... Args>
    std::unique_ptr<Results<TDomain>> RunOnFunc(const Func* func, bool isDebug, Args&&... args)
    {
        auto analysis = std::make_unique<TAnalysis>(func, builder, isDebug, std::forward<Args>(args)...);
        auto engine = Engine<TDomain>(func, std::move(analysis));
        return engine.IterateToFixpoint();
    }

    /**
     * @brief return result of analysis for certain function
     * @param func function to return analysis result
     * @return analysis result
     */
    Results<TDomain>* CheckFuncResult(const Func* func)
    {
        if (auto it = resultsMap.find(func); it != resultsMap.end()) {
            return it->second.get();
        } else {
            return nullptr;
        }
    }

    /**
     * @brief clear analysis result
     */
    void InvalidateAllAnalysisResults()
    {
        resultsMap.clear();
    }

    /**
     * @brief clear analysis result of certain function
     * @param func function to clear analysis result
     * @return whether clear is happened
     */
    bool InvalidateAnalysisResult(const Func* func)
    {
        if (auto it = resultsMap.find(func); it != resultsMap.end()) {
            resultsMap.erase(it);
            return true;
        } else {
            return false;
        }
    }

private:
    template <typename... Args>
    void RunOnPackageInSerial(const Package* package, bool isDebug, Args&&... args)
    {
        if constexpr (IsValueAnalysis<TAnalysis>::value) {
            SetUpGlobalVarState(*package, isDebug, std::forward<Args>(args)...);
        }
        for (auto func : package->GetGlobalFuncs()) {
            if (ShouldBeAnalysed(*func)) {
                if (auto res = RunOnFunc(func, isDebug, std::forward<Args>(args)...)) {
                    resultsMap.emplace(func, std::move(res));
                }
            }
        }
    }

    template <typename... Args>
    void RunOnPackageInParallel(const Package* package, bool isDebug, size_t threadNum, Args&&... args)
    {
        if constexpr (IsValueAnalysis<TAnalysis>::value) {
            SetUpGlobalVarState(*package, isDebug, std::forward<Args>(args)...);
        }
        Utils::TaskQueue taskQueue(threadNum);
        using ResTy = std::unique_ptr<Results<TDomain>>;
        std::vector<Codira::Utils::TaskResult<ResTy>> results;
        for (auto func : package->GetGlobalFuncs()) {
            if (ShouldBeAnalysed(*func)) {
                results.emplace_back(taskQueue.AddTask<ResTy>(
                    [func, isDebug, &args..., this]() { return RunOnFunc(func, isDebug, std::forward<Args>(args)...); },
                    // Roughly use the number of Blocks as the cost of task weight
                    func->GetBody()->GetBlocks().size()));
            }
        }

        taskQueue.RunAndWaitForAllTasksCompleted();

        for (auto& result : results) {
            if (auto res = result.get()) {
                resultsMap.emplace(res->func, std::move(res));
            }
        }
    }

    bool ShouldBeAnalysed(const Func& func)
    {
        if constexpr (IsValueAnalysis<TAnalysis>::value) {
            if (resultsMap.find(&func) != resultsMap.end()) {
                return false;
            }
        }
        return TAnalysis::Filter(func);
    }

    template <typename... Args> void SetUpGlobalVarState(const Package& package, bool isDebug, Args&&... args)
    {
        TAnalysis::InitialiseLetGVState(package, builder);
        for (auto gv : package.GetGlobalVars()) {
            if (auto init = gv->GetInitFunc();
                gv->TestAttr(Attribute::READONLY) && init && resultsMap.find(init) == resultsMap.end()) {
                // Multiple global vars may be initialised in the same function.
                // e.g. let (x, y) = (1, 2)
                resultsMap.emplace(init, RunOnFunc(init, isDebug, std::forward<Args>(args)...));
            }
        }
    }

    std::unordered_map<const Func*, std::unique_ptr<Results<TDomain>>> resultsMap;
    CHIRBuilder& builder;
};

} // namespace Codira::CHIR

#endif
