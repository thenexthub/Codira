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

#ifndef CODIRA_CHIR_TRANSFORMATION_DEVIRTUALIZATION_H
#define CODIRA_CHIR_TRANSFORMATION_DEVIRTUALIZATION_H

#include "Codira/CHIR/Analysis/AnalysisWrapper.h"
#include "Codira/CHIR/Analysis/DevirtualizationInfo.h"
#include "Codira/CHIR/Analysis/TypeAnalysis.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Value.h"
#include <unordered_map>

namespace Codira::CHIR {
class Devirtualization {
public:
    /**
     * @brief wrapper for type analysis.
     */
    using TypeAnalysisWrapper = AnalysisWrapper<TypeAnalysis, TypeDomain>;

    /**
     * @brief rewrite info if a invoke can be de-virtualize.
     */
    struct RewriteInfo {
        Invoke* invoke;
        FuncBase* realCallee;
        Type* thisType;
        std::vector<Type*> typeArgs;
        Apply* newApply = nullptr;
    };

    Devirtualization() = delete;

    /**
     * @brief constructor for Devirtualization pass.
     * @param typeAnalysisWrapper
     * @param devirtFuncInfo
     */
    explicit Devirtualization(TypeAnalysisWrapper* typeAnalysisWrapper, DevirtualizationInfo& devirtFuncInfo);

    /**
     * @brief main optimization pass entry.
     * @param funcs funcs to devirtualization.
     * @param builder CHIR builder for generating IR.
     * @param isDebug flag whether print debug log.
     */
    void RunOnFuncs(const std::vector<Func*>& funcs, CHIRBuilder& builder, bool isDebug);

    /**
     * @brief get functions containing invoke expression.
     * @param package user package to optimization.
     * @return return functions containing invoke expression.
     */
    static std::vector<Func*> CollectContainInvokeExprFuncs(const Ptr<const Package>& package);

    /// get optimized functions which are marked frozen.
    const std::vector<Func*>& GetFrozenInstFuns() const;

    /// after first devirt pass, do second devirtualization for frozen func.
    /// this function mainly get results from second type analysis.
    void AppendFrozenFuncState(const Func* func, std::unique_ptr<Results<TypeDomain>> analysisRes);

    /// function signature to determine a certain function.
    struct FuncSig {
        std::string name;
        std::vector<Type*> types;
        std::vector<Type*> typeArgs;
    };

private:
    void RunOnFunc(const Func* func, CHIRBuilder& builder);

    std::pair<FuncBase*, Type*> FindRealCallee(
        CHIRBuilder& builder, const TypeValue* typeState, const FuncSig& method) const;

    bool IsValidSubType(CHIRBuilder& builder, const Type* expected, Type* specific,
        std::unordered_map<const GenericType*, Type*>& replaceTable) const;

    bool IsInstantiationOf(CHIRBuilder& builder, const GenericType* generic, const Type* instantiated) const;

    void InstantiateFuncIfPossible(CHIRBuilder& builder, std::vector<RewriteInfo>& rewriteInfoList);

    void CollectCandidates(
        CHIRBuilder& builder, ClassType* specific, std::pair<FuncBase*, Type*>& res, const FuncSig& method) const;

    FuncBase* GetCandidateFromSpecificType(
        CHIRBuilder& builder, ClassType& specific, const FuncSig& method) const;

    static void RewriteToApply(CHIRBuilder& builder, std::vector<RewriteInfo>& rewriteInfos, bool isDebug);

    static bool RewriteToBuiltinOp(CHIRBuilder& builder, const RewriteInfo& info, bool isDebug);

    /**
     * check func whether has invoke expression, implement func for CollectContainInvokeExprFuncs
     */
    static bool CheckFuncHasInvoke(const BlockGroup& bg);

    static bool CheckAllGenericTypeVisible(
        const Type& thisType, const std::unordered_set<const GenericType*>& visibleSet);

    TypeAnalysisWrapper* analysisWrapper;
    DevirtualizationInfo& devirtFuncInfo;
    std::vector<RewriteInfo> rewriteInfos{};

    // frozen inst functions after devirt, these func need a devirt optimization too after first devirt opt
    std::vector<Func*> frozenInstFuns;
    // extra type state from outside
    std::unordered_map<const Func*, std::unique_ptr<Results<TypeDomain>>> frozenStates;

    std::unordered_map<std::string, Func*> frozenInstFuncMap;
};
} // namespace Codira::CHIR

#endif
