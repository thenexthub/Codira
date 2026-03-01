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

#ifndef CODIRA_CHIR_ANALYSIS_MAYBE_INIT_ANALYSIS_H
#define CODIRA_CHIR_ANALYSIS_MAYBE_INIT_ANALYSIS_H

#include "Codira/CHIR/Analysis/MaybeUninitAnalysis.h"

namespace Codira::CHIR {

class MaybeInitAnalysis;

/**
 * @brief maybe init domain, indicate a state whether values have been init.
 */
class MaybeInitDomain : public GenKillDomain<MaybeInitDomain> {
    friend class MaybeInitAnalysis;

public:
    MaybeInitDomain() = delete;
    /**
     * @brief constructor of maybe init domain.
     * @param domainSize domain size of one function.
     * @param ctorInitInfo extra info for init function check.
     * @param allocateIdxMap allocate map to analysis in this pass.
     */
    MaybeInitDomain(size_t domainSize, const ConstructorInitInfo* ctorInitInfo,
        std::unordered_map<const Value*, size_t>* allocateIdxMap);

    /// constructor of maybe init domain.
    ~MaybeInitDomain() override
    {
    }

    /**
     * @brief check whether location is maybe inited.
     * @param location location to check status.
     * @return status if location is maybe inited.
     */
    std::optional<bool> IsMaybeInitedAllocation(const Value* location) const;

    /**
     * @brief extra info to indicate if status of value in init function or its super class.
     */
    enum class InitedMemberKind { SUPER_MEMBER, LOCAL_MEMBER, NA };

    /**
     * @brief check if member var is init in init function.
     * @param memberIndex index of member var.
     * @return info to indicate if member var is in super class or this class.
     */
    InitedMemberKind IsMaybeInitedMember(size_t memberIndex) const;

private:
    /// init local member to inited.
    void SetAllLocalMemberInited();
    /// extra info for init function check.
    const ConstructorInitInfo* ctorInitInfo;
    /// allocate map from location to index.
    std::unordered_map<const Value*, size_t>* allocateIdxMap;
};

/**
 * @brief partially specialized member to MaybeInitDomain of analysis
 */
template <> const std::string Analysis<MaybeInitDomain>::name;
template <> const std::optional<unsigned> Analysis<MaybeInitDomain>::blockLimit;
template <> const AnalysisKind GenKillDomain<MaybeInitDomain>::mustOrMaybe;

/**
 * @brief may be init analysis, analyse a status which values whether have been init.
 */
class MaybeInitAnalysis final : public GenKillAnalysis<MaybeInitDomain> {
public:
    MaybeInitAnalysis() = delete;

    /**
     * @brief may be init analysis constructor.
     * @param func function to analyse.
     * @param ctorInitInfo extra info for init function check.
     */
    MaybeInitAnalysis(const Func* func, const ConstructorInitInfo* ctorInitInfo);

    /// may be init analysis destructor.
    ~MaybeInitAnalysis() final
    {
    }

    /// return Bottom of MaybeInitDomain
    MaybeInitDomain Bottom() override;

    /**
     * @brief use input state to initialize entry state of functions.
     * @param state input entry state of analysing function.
     */
    void InitializeFuncEntryState(MaybeInitDomain& state) override;

    /**
     * @brief propagate state to next expression.
     * @param state current state of this function.
     * @param expression next expression to analyse.
     */
    void PropagateExpressionEffect(MaybeInitDomain& state, const Expression* expression) override;

    /**
     * @brief propagate state to next terminator.
     * @param state current state of this function.
     * @param expression next terminator to analyse.
     * @return blocks return after analysis.
     */
    std::optional<Block*> PropagateTerminatorEffect(MaybeInitDomain& state, const Terminator* expression) override;

private:
    void HandleAllocateExpr(MaybeInitDomain& state, const Allocate* allocate);

    void HandleStoreExpr(MaybeInitDomain& state, const Store* store);

    void HandleStoreElemRefExpr(MaybeInitDomain& state, const StoreElementRef* store) const;

    void HandleApplyExpr(MaybeInitDomain& state, const Apply* apply) const;

    const ConstructorInitInfo* ctorInitInfo;
    std::unordered_map<const Value*, size_t> allocateIdxMap{};
};
} // namespace Codira::CHIR

#endif
