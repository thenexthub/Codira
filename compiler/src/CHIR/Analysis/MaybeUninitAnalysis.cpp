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

#include "Codira/CHIR/Analysis/MaybeUninitAnalysis.h"

#include "Codira/CHIR/Analysis/Utils.h"
#include "Codira/CHIR/CHIRCasting.h"

namespace Codira::CHIR {

MaybeUninitDomain::MaybeUninitDomain(size_t domainSize, const ConstructorInitInfo* ctorInitInfo,
    std::unordered_map<const Value*, size_t>* allocateIdxMap)
    : GenKillDomain(domainSize),
      ctorInitInfo(ctorInitInfo),
      allocateIdxMap(allocateIdxMap),
      maybeInitedPos(std::vector<std::set<unsigned>>(domainSize))
{
}

bool MaybeUninitDomain::Join(const MaybeUninitDomain& rhs)
{
    for (size_t i = 0; i < maybeInitedPos.size(); ++i) {
        maybeInitedPos[i].insert(rhs.maybeInitedPos[i].cbegin(), rhs.maybeInitedPos[i].cend());
    }
    return GenKillDomain::Join(rhs);
}

std::optional<bool> MaybeUninitDomain::IsMaybeUninitedAllocation(const Value* location) const
{
    if (auto it = allocateIdxMap->find(location); it != allocateIdxMap->end()) {
        return IsTrueAt(it->second);
    } else {
        return std::nullopt;
    }
}

const std::set<unsigned>& MaybeUninitDomain::GetMaybeInitedPos(const Value* location) const
{
    return maybeInitedPos[allocateIdxMap->at(location)];
}

MaybeUninitDomain::UninitedMemberKind MaybeUninitDomain::IsMaybeUninitedMember(size_t memberIndex) const
{
    CODEC_ASSERT(ctorInitInfo);
    if (memberIndex < ctorInitInfo->superMemberNums) {
        return IsTrueAt(states.size() - 1) ? UninitedMemberKind::SUPER_MEMBER : UninitedMemberKind::NA;
    } else if (memberIndex < ctorInitInfo->superMemberNums + ctorInitInfo->localMemberNums) {
        return IsTrueAt(memberIndex - ctorInitInfo->superMemberNums) ? UninitedMemberKind::LOCAL_MEMBER
                                                                     : UninitedMemberKind::NA;
    }

    CODEC_ABORT();
    return UninitedMemberKind::NA;
}

const std::set<unsigned>& MaybeUninitDomain::GetMaybeInitedPos(size_t memberIndex) const
{
    CODEC_ASSERT(memberIndex >= ctorInitInfo->superMemberNums);
    return maybeInitedPos[memberIndex - ctorInitInfo->superMemberNums];
}

std::vector<size_t> MaybeUninitDomain::GetMaybeUninitedLocalMembers() const
{
    CODEC_ASSERT(ctorInitInfo);
    std::vector<size_t> uninitedLocalMembers;
    for (size_t i = 0; i < ctorInitInfo->localMemberNums; ++i) {
        if (IsTrueAt(i)) {
            uninitedLocalMembers.emplace_back(i + ctorInitInfo->superMemberNums);
        }
    }
    return uninitedLocalMembers;
}

void MaybeUninitDomain::SetAllLocalMemberInited()
{
    for (size_t i = 0; i < ctorInitInfo->localMemberNums; ++i) {
        Kill(i);
    }
}

template <> const std::string Analysis<MaybeUninitDomain>::name = "maybe-uninit";
template <> const std::optional<unsigned> Analysis<MaybeUninitDomain>::blockLimit = std::nullopt;
template <> const AnalysisKind GenKillDomain<MaybeUninitDomain>::mustOrMaybe = AnalysisKind::MAYBE;

void SaveAllocateMap(
    const BlockGroup& body, size_t& allocateIdx, std::unordered_map<const Value*, size_t>& allocateIdxMap)
{
    for (auto bb : body.GetBlocks()) {
        for (auto expr : bb->GetExpressions()) {
            auto kind = expr->GetExprKind();
            if ((kind == ExprKind::ALLOCATE || kind == ExprKind::ALLOCATE_WITH_EXCEPTION) &&
                expr->GetResult()->GetDebugExpr()) {
                allocateIdxMap.emplace(expr->GetResult(), allocateIdx++);
            }
            if (kind == ExprKind::LAMBDA) {
                SaveAllocateMap(*StaticCast<Lambda*>(expr)->GetBody(), allocateIdx, allocateIdxMap);
            }
        }
    }
}

MaybeUninitAnalysis::MaybeUninitAnalysis(const Func* func, const ConstructorInitInfo* ctorInitInfo)
    : GenKillAnalysis(func), ctorInitInfo(ctorInitInfo)
{
    size_t allocateIdx = ctorInitInfo->localMemberNums;
    SaveAllocateMap(*func->GetBody(), allocateIdx, allocateIdxMap);

    domainSize = ctorInitInfo->localMemberNums + allocateIdx;
    if (ctorInitInfo->superClassDef) {
        ++domainSize;
    }
}

MaybeUninitDomain MaybeUninitAnalysis::Bottom()
{
    return MaybeUninitDomain(domainSize, ctorInitInfo, &allocateIdxMap);
}

void MaybeUninitAnalysis::InitializeFuncEntryState(MaybeUninitDomain& state)
{
    // set all bits to 1 (uninitialised)
    state.kind = ReachableKind::REACHABLE;
    state.GenAll();
}

void MaybeUninitAnalysis::PropagateExpressionEffect(MaybeUninitDomain& state, const Expression* expression)
{
    if (expression->GetExprKind() == ExprKind::ALLOCATE) {
        return HandleAllocateExpr(state, StaticCast<const Allocate*>(expression));
    } else if (expression->GetExprKind() == ExprKind::STORE) {
        return HandleStoreExpr(state, StaticCast<const Store*>(expression));
    } else if (expression->GetExprKind() == ExprKind::STORE_ELEMENT_REF) {
        return HandleStoreElemRefExpr(state, StaticCast<const StoreElementRef*>(expression));
    } else if (expression->GetExprKind() == ExprKind::APPLY) {
        return HandleApplyExpr(state, StaticCast<const Apply*>(expression));
    }
}

std::optional<Block*> MaybeUninitAnalysis::PropagateTerminatorEffect(
    MaybeUninitDomain& state, const Terminator* expression)
{
    (void)state;
    (void)expression;
    return std::nullopt;
}

void MaybeUninitAnalysis::HandleAllocateExpr(MaybeUninitDomain& state, const Allocate* allocate)
{
    auto res = allocate->GetResult();
    if (auto it = allocateIdxMap.find(res); it != allocateIdxMap.end()) {
        auto allcoateIdx = it->second;
        state.Gen(allcoateIdx);
    }
}

void MaybeUninitAnalysis::HandleStoreExpr(MaybeUninitDomain& state, const Store* store)
{
    auto location = store->GetLocation();
    if (auto it = allocateIdxMap.find(location); it != allocateIdxMap.end()) {
        auto allcoateIdx = it->second;
        state.Kill(allcoateIdx);
        auto debugLoc = store->GetDebugLocation();
        state.maybeInitedPos[allcoateIdx].emplace(debugLoc.GetBeginPos().line);
    }
}

void MaybeUninitAnalysis::HandleStoreElemRefExpr(MaybeUninitDomain& state, const StoreElementRef* store) const
{
    // modify by codemp, adapt for var init func create
    auto parentFunc = store->GetTopLevelFunc();
    CODEC_NULLPTR_CHECK(parentFunc);
    auto memberIdxOpt = IsInitialisingMemberVar(*parentFunc, *store);
    if (!memberIdxOpt.has_value()) {
        return;
    }
    auto memberIdx = memberIdxOpt.value();
    CODEC_ASSERT(memberIdx < ctorInitInfo->superMemberNums + ctorInitInfo->localMemberNums);
    if (memberIdx < ctorInitInfo->superMemberNums) {
        return;
    }
    auto memberStateIdx = memberIdx - ctorInitInfo->superMemberNums;
    state.Kill(memberStateIdx);
    auto& debugLoc = store->GetDebugLocation();
    state.maybeInitedPos[memberStateIdx].emplace(debugLoc.GetBeginPos().line);
}

void MaybeUninitAnalysis::HandleApplyExpr(MaybeUninitDomain& state, const Apply* apply) const
{
    if (!func->IsConstructor()) {
        return;
    }
    // Check if it is a call to super init function of this class
    if (apply->IsSuperCall()) {
        state.Kill(domainSize - 1);
        return;
    }
    // Check if it is a call to another init function of this class/struct
    auto callee = apply->GetCallee();
    if (callee->IsFuncWithBody()) {
        auto calleeFunc = VirtualCast<Func*>(callee);
        if (calleeFunc->IsConstructor() &&
            calleeFunc->GetOuterDeclaredOrExtendedDef() == ctorInitInfo->thisCustomDef &&
            apply->GetArgs()[0] == func->GetParam(0)) {
            state.SetAllLocalMemberInited();
            if (ctorInitInfo->superClassDef) {
                state.Kill(domainSize - 1);
            }
        }
    }
}
}  // namespace Codira::CHIR
