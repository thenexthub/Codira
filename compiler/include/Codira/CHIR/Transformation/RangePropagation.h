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

#ifndef CODIRA_CHIR_TRANSFORMATION_RANGE_PROPAGATION_H
#define CODIRA_CHIR_TRANSFORMATION_RANGE_PROPAGATION_H

#include "Codira/CHIR/Analysis/AnalysisWrapper.h"
#include "Codira/CHIR/Analysis/ValueRangeAnalysis.h"
#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Transformation/DeadCodeElimination.h"
#include "Codira/CHIR/Utils.h"
#include "Codira/CHIR/Value.h"

namespace Codira::CHIR {
/**
 * CHIR Opt Pass: do optimization with analysis results of range analysis.
 */
class RangePropagation {
public:
    /**
     * @brief range analysis wrapper to call range analysis.
     */
    using RangeAnalysisWrapper = AnalysisWrapper<RangeAnalysis, RangeDomain>;

    /**
     * @brief constructor to do range propagation.
     * @param builder CHIR builder for generating IR.
     * @param rangeAnalysisWrapper range analysis wrapper which produce analysis results.
     * @param diag reporter to print warning
     * @param enIncre flag whether is incremental compile.
     */
    explicit RangePropagation(
        CHIRBuilder& builder, RangeAnalysisWrapper* rangeAnalysisWrapper, DiagAdapter* diag, bool enIncre);

    /**
     * @brief Main process to do range propagation.
     * @param package package to do optimization.
     * @param isDebug flag whether print debug log.
     */
    void RunOnPackage(const Ptr<const Package>& package, bool isDebug);

    /**
     * @brief Main process to do const propagation per func.
     * @param func func to do optimization.
     * @param isDebug flag whether print debug log.
     */
    void RunOnFunc(const Ptr<const Func>& func, bool isDebug);

    /**
     * @brief Get effect map after this pass.
     * @return effect map affected by this pass.
     */
    const OptEffectCHIRMap& GetEffectMap() const;

    /**
     * @brief Get all funcs need to remove unreachable blocks.
     * @return functions
     */
    const std::vector<const Func*>& GetFuncsNeedRemoveBlocks() const;
private:
    struct RewriteInfo {
        Expression* oldExpr;
        size_t index; // the index of the oldExpr in its parent block
        LiteralValue* literalVal;

        RewriteInfo(Expression* oldExpr, size_t index, LiteralValue* literalVal)
            : oldExpr(oldExpr), index(index), literalVal(literalVal)
        {
        }
    };

    // ==================== Rewrite Non-terminator Expressions ==================== //

    /**
     * This function will generate a literal value based on the range information
     * from @p constVal. The type of the literal value is @p type.
     */
    Ptr<LiteralValue> GenerateConstExpr(const Ptr<Type>& type, const Ptr<const ValueRange>& rangeVal);

    /**
     * This function will rewrite an expression based on the @p rewriteInfo, which stores
     * the exrpession to be rewrited, the index of this expression and the new expression.
     */
    void RewriteToConstExpr(const RewriteInfo& rewriteInfo, bool isDebug) const;

    // ==================== Rewrite Terminator Expressions ==================== //

    /**
     * This function will rewrite a Branch terminator or a MultiBranch terminator to a GoTo
     * terminator. The new successor will be @p targetSucc.
     */
    void RewriteBranchTerminator(const Ptr<Terminator>& branch, const Ptr<Block>& targetSucc, bool isDebug);

    void RecordEffectMap(const Expression* expr, const Func* func) const;

    void CheckVarrayIndex(const Ptr<Intrinsic>& intrin, const RangeDomain& state) const;

    CHIRBuilder& builder;
    RangeAnalysisWrapper* analysisWrapper;
    DiagAdapter* diag;
    bool enIncre;
    static OptEffectCHIRMap effectMap;
    std::vector<const Func*> funcsNeedRemoveBlocks;
};

} // namespace Codira::CHIR
#endif
