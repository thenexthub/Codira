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

#ifndef CODIRA_CHIR_ANALYSIS_GETORTHROW_RESULT_ANALYSIS_H
#define CODIRA_CHIR_ANALYSIS_GETORTHROW_RESULT_ANALYSIS_H

#include "Codira/CHIR/Analysis/Analysis.h"
#include "Codira/CHIR/Analysis/FlatSet.h"
#include "Codira/CHIR/Value.h"

namespace Codira::CHIR {
/**
 * @brief domain definition of get or throw optimization pass.
 */
class GetOrThrowResultAnalysis;
class GetOrThrowResultDomain final : public AbstractDomain<GetOrThrowResultDomain> {
public:
    friend class GetOrThrowResultAnalysis;
    GetOrThrowResultDomain() = delete;
    /**
     * @brief constructor for domain definition of get or throw optimization pass.
     * @param argIdxMap index map, value of domain.
     */
    GetOrThrowResultDomain(std::unordered_map<const Value*, size_t>* argIdxMap);
    ~GetOrThrowResultDomain()
    {
    }

    /// join two domains
    bool Join(const GetOrThrowResultDomain& rhs) override;

    /// output string of get or throw domain.
    std::string ToString() const override;

    /**
     * @brief check location's domain result.
     * @param location location value to check its domain.
     * @return return apply if location has its domain.
     */
    const Apply* CheckGetOrThrowResult(const Value* location) const;

private:
    /// results of get or throw analysis
    std::vector<FlatSet<const Apply>> getOrThrowResults;
    /// from location to index
    std::unordered_map<const Value*, size_t>* argIdxMap;
};

template <> const std::string Analysis<GetOrThrowResultDomain>::name;
template <> const std::optional<unsigned> Analysis<GetOrThrowResultDomain>::blockLimit;
class GetOrThrowResultAnalysis final : public Analysis<GetOrThrowResultDomain> {
public:
    GetOrThrowResultAnalysis() = delete;
    /**
     * @brief constructor of get or throw analysis.
     * @param func function to analyse.
     * @param isDebug flag whether print debug log.
     */
    GetOrThrowResultAnalysis(const Func* func, bool isDebug);
    ~GetOrThrowResultAnalysis()
    {
    }

    /// return Bottom of GetOrThrowResultDomain
    GetOrThrowResultDomain Bottom() override;

    /**
     * @brief use input state to initialize entry state of functions.
     * @param state input entry state of analysing function.
     */
    void InitializeFuncEntryState(GetOrThrowResultDomain& state) override;

    /**
     * @brief propagate state to next expression.
     * @param state current state of this function.
     * @param expression next expression to analyse.
     */
    void PropagateExpressionEffect(GetOrThrowResultDomain& state, const Expression* expression) override;

    /**
     * @brief propagate state to next terminator.
     * @param state current state of this function.
     * @param terminator next terminator to analyse.
     * @return blocks return after analysis.
     */
    std::optional<Block*> PropagateTerminatorEffect(
        GetOrThrowResultDomain& state, const Terminator* terminator) override;

private:
    /// from location to index
    std::unordered_map<const Value*, size_t> argIdxMap;
};
} // namespace Codira::CHIR

#endif
