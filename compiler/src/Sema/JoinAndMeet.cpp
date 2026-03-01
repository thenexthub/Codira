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
 * This file implements utility functions for JoinAndMeet.
 */

#include "JoinAndMeet.h"

#include "TypeCheckUtil.h"

#include "Codira/AST/Node.h"
#include "Codira/AST/Types.h"
#include "Codira/Utils/CheckUtils.h"

using namespace Codira;
using namespace AST;
using namespace TypeCheckUtil;
using ErrMsg = std::stack<std::string>;
using ErrOrTy = std::variant<ErrMsg, Ptr<AST::Ty>>;

namespace {
enum class Uniformity {
    UNIFORMED,
    MIXED,
    ALL_IRRELEVANT
};

Uniformity CheckFuncUniformity(const std::set<Ptr<Ty>>& tys)
{
    size_t paramCnt = 0;
    bool anyFuncTy = false;
    bool anyNonFuncTy = false;
    for (auto& ty : tys) {
        if (auto funcTy = DynamicCast<FuncTy>(ty)) {
            size_t curCnt = funcTy->paramTys.size();
            if (!anyFuncTy) {
                paramCnt = curCnt;
            } else if (paramCnt != curCnt) {
                return Uniformity::MIXED;
            }
            anyFuncTy = true;
        } else {
            anyNonFuncTy = true;
        }
        if (anyFuncTy && anyNonFuncTy) {
            return Uniformity::MIXED;
        }
    }
    if (!anyFuncTy) {
        return Uniformity::ALL_IRRELEVANT;
    }
    return Uniformity::UNIFORMED;
}

Uniformity CheckTupleUniformity(const std::set<Ptr<Ty>>& tys)
{
    size_t argCnt = 0;
    bool anyTupleTy = false;
    bool anyNonTupleTy = false;
    for (auto& ty : tys) {
        if (auto tupleTy = DynamicCast<TupleTy>(ty)) {
            size_t curCnt = tupleTy->typeArgs.size();
            if (!anyTupleTy) {
                argCnt = curCnt;
            } else if (argCnt != curCnt) {
                return Uniformity::MIXED;
            }
            anyTupleTy = true;
        } else {
            anyNonTupleTy = true;
        }
        if (anyTupleTy && anyNonTupleTy) {
            return Uniformity::MIXED;
        }
    }
    if (!anyTupleTy) {
        return Uniformity::ALL_IRRELEVANT;
    }
    return Uniformity::UNIFORMED;
}
} // namespace

ErrOrTy JoinAndMeet::Join(bool sprsErr)
{
    // Hot fix.
    if (!IsInputValid()) {
        return {TypeManager::GetInvalidTy()};
    }
    auto jTy = BatchJoin(tySet);
    if (sprsErr || errMsg.empty()) {
        return {jTy};
    } else {
        return {errMsg};
    }
}

ErrOrTy JoinAndMeet::JoinAsVisibleTy()
{
    // Hot fix.
    if (!IsInputValid()) {
        return {TypeManager::GetInvalidTy()};
    }
    auto jTy = BatchJoin(tySet);
    this->isForcedToUserVisible = true;
    jTy = ToUserVisibleTy(jTy);
    AddFinalErrMsgs(*jTy, true);
    if (errMsg.empty()) {
        return {jTy};
    } else {
        return {errMsg};
    }
}

Ptr<AST::Ty> JoinAndMeet::BatchJoin(const std::set<Ptr<Ty>>& tys)
{
    auto isSubtype = [this](Ptr<Ty> ty1, Ptr<Ty> ty2) { return tyMgr.IsSubtype(ty1, ty2); };
    auto isSupertype = [this](Ptr<Ty> ty1, Ptr<Ty> ty2) { return tyMgr.IsSubtype(ty2, ty1); };
    auto doBatchJoin = [this](const std::set<Ptr<AST::Ty>>& tys) {return BatchJoin(tys);};
    auto doBatchMeet = [this](const std::set<Ptr<AST::Ty>>& tys) {return BatchMeet(tys);};
    DualMode joinMode = {.bound = tyMgr.GetAnyTy(),
        .coFunc = doBatchJoin,
        .contraFunc = doBatchMeet,
        .coSubtyFunc = isSubtype};
    std::set<Ptr<Ty>> realTys;
    std::function<void(Ptr<Ty>)> insertRealTy = [this, &realTys, &insertRealTy](Ptr<Ty> ty) {
        if (auto tyVar = DynamicCast<TyVar*>(ty); (tyVar && Utils::In(tyVar, ignoredTyVars)) || ty->IsNothing()) {
            return;
        }
        if (auto unionTy = DynamicCast<UnionTy>(ty)) {
            for (auto uty : unionTy->tys) {
                insertRealTy(uty);
            }
        } else {
            realTys.insert(ty);
        }
    };
    for (auto ty : tys) {
        insertRealTy(ty);
    }
    PData::CommitScope cs(tyMgr.constraints);
    if (auto ret = FindSmallestTy(realTys, isSupertype); ret && !ret->IsInvalid()) {
        return ret;
    }
    PData::Reset(tyMgr.constraints);
    if (auto funcTyJoin = JoinOrMeetFuncTy(joinMode, realTys)) {
        return funcTyJoin;
    }
    PData::Reset(tyMgr.constraints);
    if (auto tupleTyJoin = JoinOrMeetTupleTy(joinMode, realTys)) {
        return tupleTyJoin;
    }
    PData::Reset(tyMgr.constraints);
    auto common = tyMgr.GetAllCommonSuperTys(std::unordered_set<Ptr<Ty>>(realTys.begin(), realTys.end()));
    if (curFile) {
        Utils::EraseIf(common, [this](Ptr<Ty> ty) { return !impMgr->IsTyAccessible(*curFile, *ty); });
    }
    if (common.empty()) {
        return tyMgr.GetAnyTy();
    }
    auto ret = FindSmallestTy(std::set<Ptr<Ty>>(common.begin(), common.end()), isSubtype);
    // reset unnecessary constaints from finding possible supertypes (e.g. those claimed by conditional extensions),
    // and re-enforce necessary constraints by judging the common supertype again
    PData::Reset(tyMgr.constraints);
    if (ret->IsInvalid() || !LessThanAll(ret, realTys, isSupertype)) {
        PData::Reset(tyMgr.constraints);
    }
    return ret;
}

/**
 * returns:
 *      - the joined/met FuncTy if all tys are FuncTy and the LUB/GLB exists
 *      - AnyTy/Nothing if there exists any FuncTy but the LUB/GLB doesn't exist
 *      - nullptr if there is no FuncTy in tys
 */
Ptr<AST::Ty> JoinAndMeet::JoinOrMeetFuncTy(const DualMode& mode, const std::set<Ptr<Ty>>& tys)
{
    auto uniformity = CheckFuncUniformity(tys);
    switch (uniformity) {
        case Uniformity::ALL_IRRELEVANT:
            return nullptr;
        case Uniformity::MIXED:
            return mode.bound;
        default:
            break;
    }
    size_t paramCnt = RawStaticCast<FuncTy*>(*tys.begin())->paramTys.size();
    std::vector<Ptr<Ty>> paramTys(paramCnt);
    for (size_t i = 0; i < paramCnt; i++) {
        std::set<Ptr<Ty>> operandParamTys;
        for (auto ty : tys) {
            operandParamTys.insert(RawStaticCast<FuncTy*>(ty)->paramTys[i]);
        }
        paramTys[i] = mode.contraFunc(operandParamTys);
    }
    std::set<Ptr<Ty>> operandRetTys;
    for (auto ty : tys) {
        operandRetTys.insert(RawStaticCast<FuncTy*>(ty)->retTy);
    }
    auto retTy = mode.coFunc(operandRetTys);
    if (Ty::AreTysCorrect(paramTys) && Ty::IsTyCorrect(retTy)) {
        auto resultTy = tyMgr.GetFunctionTy(paramTys, retTy);
        CODEC_NULLPTR_CHECK(resultTy);
        for (auto ty : tys) {
            if (!mode.coSubtyFunc(ty, resultTy)) {
                return mode.bound;
            }
        }
        return resultTy;
    }
    return mode.bound;
}

/**
 * returns:
 *      - the joined/met TupleTy if all tys are TupleTy and the LUB/GLB exists
 *      - AnyTy/Nothing if there exists any TupleTy but the LUB/GLB doesn't exist
 *      - nullptr if there is no TupleTy in tys
 */
Ptr<AST::Ty> JoinAndMeet::JoinOrMeetTupleTy(const DualMode& mode, const std::set<Ptr<Ty>>& tys)
{
    auto uniformity = CheckTupleUniformity(tys);
    switch (uniformity) {
        case Uniformity::ALL_IRRELEVANT:
            return nullptr;
        case Uniformity::MIXED:
            return mode.bound;
        default:
            break;
    }
    size_t argCnt = RawStaticCast<TupleTy*>(*tys.begin())->typeArgs.size();
    std::vector<Ptr<Ty>> typeArgs(argCnt);
    for (size_t i = 0; i < argCnt; i++) {
        std::set<Ptr<Ty>> operandTyArgs;
        for (auto& ty : tys) {
            operandTyArgs.insert(RawStaticCast<TupleTy*>(ty)->typeArgs[i]);
        }
        typeArgs[i] = mode.coFunc(operandTyArgs);
    }
    if (Ty::AreTysCorrect(typeArgs)) {
        auto resultTy = tyMgr.GetTupleTy(typeArgs);
        for (auto ty : tys) {
            if (!mode.coSubtyFunc(ty, resultTy)) {
                return mode.bound;
            }
        }
        return resultTy;
    }
    return mode.bound;
}

ErrOrTy JoinAndMeet::Meet(bool sprsErr)
{
    // Hot fix.
    if (!IsInputValid()) {
        return {TypeManager::GetInvalidTy()};
    }
    auto mTy = BatchMeet(tySet);
    if (sprsErr || errMsg.empty()) {
        return {mTy};
    } else {
        return {errMsg};
    }
}

ErrOrTy JoinAndMeet::MeetAsVisibleTy()
{
    // Hot fix.
    if (!IsInputValid()) {
        return {TypeManager::GetInvalidTy()};
    }
    auto mTy = BatchMeet(tySet);
    this->isForcedToUserVisible = true;
    mTy = ToUserVisibleTy(mTy);
    AddFinalErrMsgs(*mTy, false);
    if (errMsg.empty()) {
        return {mTy};
    } else {
        return {errMsg};
    }
}

Ptr<AST::Ty> JoinAndMeet::BatchMeet(const std::set<Ptr<Ty>>& tys)
{
    auto isSubtype = [this](Ptr<Ty> ty1, Ptr<Ty> ty2) { return tyMgr.IsSubtype(ty1, ty2); };
    auto isSupertype = [this](Ptr<Ty> ty1, Ptr<Ty> ty2) { return tyMgr.IsSubtype(ty2, ty1); };
    auto doBatchJoin = [this](const std::set<Ptr<AST::Ty>>& tys) {return BatchJoin(tys);};
    auto doBatchMeet = [this](const std::set<Ptr<AST::Ty>>& tys) {return BatchMeet(tys);};
    DualMode meetMode = {.bound = TypeManager::GetInvalidTy(),
        .coFunc = doBatchMeet,
        .contraFunc = doBatchJoin,
        .coSubtyFunc = isSupertype};
    std::set<Ptr<Ty>> realTys;
    std::function<void(Ptr<Ty>)> insertRealTy = [this, &realTys, &insertRealTy](Ptr<Ty> ty) {
        if (auto tyVar = DynamicCast<TyVar*>(ty); tyVar && Utils::In(tyVar, ignoredTyVars)) {
            return;
        }
        if (auto unionTy = DynamicCast<UnionTy>(ty)) {
            insertRealTy(BatchJoin(unionTy->tys));
        }
        if (auto itsTy = DynamicCast<IntersectionTy>(ty)) {
            for (auto ity : itsTy->tys) {
                insertRealTy(ity);
            }
        } else {
            realTys.insert(ty);
        }
    };
    for (auto ty : tys) {
        insertRealTy(ty);
    }
    PData::CommitScope cs(tyMgr.constraints);
    if (auto ret = FindSmallestTy(realTys, isSubtype); ret && !ret->IsInvalid()) {
        return ret;
    }
    PData::Reset(tyMgr.constraints);
    if (auto funcTyJoin = JoinOrMeetFuncTy(meetMode, realTys)) {
        return funcTyJoin;
    }
    PData::Reset(tyMgr.constraints);
    if (auto tupleTyJoin = JoinOrMeetTupleTy(meetMode, realTys)) {
        return tupleTyJoin;
    }
    PData::Reset(tyMgr.constraints);
    return TypeManager::GetInvalidTy();
}

std::string JoinAndMeet::CombineErrMsg(ErrMsg& msgs)
{
    std::string res{};
    res.append("Traces:\n");

    while (!msgs.empty()) {
        res.append(msgs.top() + "\n");
        msgs.pop();
    }
    return res;
}

Ptr<Ty> JoinAndMeet::ToUserVisibleTy(Ptr<Ty> ty)
{
    CODEC_NULLPTR_CHECK(ty);
    if (ty->IsIntersection()) {
        auto iSecTy = RawStaticCast<IntersectionTy*>(ty);
        auto isSubtype = [this](Ptr<Ty> ty1, Ptr<Ty> ty2) { return tyMgr.IsSubtype(ty1, ty2); };
        Ptr<Ty> res = ToUserVisibleTy(FindSmallestTy(iSecTy->tys, isSubtype));
        // Given C1 <: I1 & I2 and C2 <: I1 & I2, then Join(C1, C2) gives I1 & I2.
        // Meet(I1, I2) gives Nothing but the result for the original Join.
        return res->IsNothing() ? tyMgr.GetAnyTy() : res;
    } else if (auto unionTy = DynamicCast<UnionTy>(ty)) {
        std::set<Ptr<Ty>> uTys;
        for (auto& t : unionTy->tys) {
            uTys.insert(ToUserVisibleTy(t));
        }
        auto res = BatchJoin(uTys);
        // Dual of the above comments.
        return res->IsAny() ? TypeManager::GetNothingTy() : res;
    } else if (ty->IsFunc()) {
        auto funcTy = RawStaticCast<FuncTy*>(ty);
        auto retTy = ToUserVisibleTy(funcTy->retTy);
        auto paramTys = funcTy->paramTys;
        std::transform(funcTy->paramTys.begin(), funcTy->paramTys.end(), paramTys.begin(),
            [this](Ptr<Ty> typ) { return ToUserVisibleTy(typ); });
        if (Ty::AreTysCorrect(paramTys) && Ty::IsTyCorrect(retTy)) {
            return tyMgr.GetFunctionTy(paramTys, retTy, {funcTy->isC, funcTy->isClosureTy, funcTy->hasVariableLenArg});
        } else {
            return TypeManager::GetInvalidTy();
        }
    } else if (ty->IsTuple()) {
        auto tupleTy = RawStaticCast<TupleTy*>(ty);
        auto elemTys = tupleTy->typeArgs;
        std::transform(tupleTy->typeArgs.begin(), tupleTy->typeArgs.end(), elemTys.begin(),
            [this](Ptr<Ty> typ) { return ToUserVisibleTy(typ); });
        if (Ty::AreTysCorrect(elemTys)) {
            return tyMgr.GetTupleTy(elemTys);
        } else {
            return TypeManager::GetInvalidTy();
        }
    } else {
        return ty;
    }
}

void JoinAndMeet::AddFinalErrMsgs(const Ty& ty, bool isJoin)
{
    auto getTysStr = [this]() {
        auto tyVec = Utils::SetToVec<Ptr<Ty>>(tySet);
        std::sort(tyVec.begin(), tyVec.end(), CompTyByNames);
        std::string tysStr;
        for (auto it = tyVec.begin(); it != std::prev(tyVec.end()); ++it) {
            tysStr += (it == tyVec.begin() ? std::string() : ", ") + "'" + Ty::ToString(*it) + "'";
        }
        if (tyVec.size() > 1) {
            CODEC_NULLPTR_CHECK(tyVec.back());
            tysStr += " and '" + tyVec.back()->String() + "'";
        }
        return tysStr;
    };

    if (ty.IsInvalid()) {
        CODEC_ASSERT(!tySet.empty());
        std::string newErrMsg = "The types " + getTysStr() + " do not have ";
        newErrMsg += isJoin ? "the smallest common supertype" : "the greatest common subtype";
        errMsg.push(newErrMsg);
        return;
    }
}

std::optional<std::string> JoinAndMeet::SetJoinedType(
    Ptr<Ty>& ty, std::variant<std::stack<std::string>, Ptr<Ty>>& joinRes)
{
    if (std::get_if<Ptr<Ty>>(&joinRes)) {
        ty = std::get<Ptr<Ty>>(joinRes);
        return {};
    } else {
        ty = TypeManager::GetInvalidTy();
        auto errMsgs = std::get<std::stack<std::string>>(joinRes);
        return {JoinAndMeet::CombineErrMsg(errMsgs)};
    }
}

std::optional<std::string> JoinAndMeet::SetMetType(Ptr<Ty>& ty, std::variant<std::stack<std::string>, Ptr<Ty>>& metRes)
{
    return SetJoinedType(ty, metRes);
}

bool JoinAndMeet::IsInputValid() const
{
    bool isValid = true;
    if (tySet.empty()) {
        isValid = false;
    }
    if (!Ty::AreTysCorrect(tySet)) {
        isValid = false;
    }
    return isValid;
}
