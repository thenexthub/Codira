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

#ifndef CODIRA_CHIR_TRANSFORMATION_CONST_PROPAGATION_H
#define CODIRA_CHIR_TRANSFORMATION_CONST_PROPAGATION_H

#include "Codira/CHIR/Analysis/AnalysisWrapper.h"
#include "Codira/CHIR/Analysis/ConstAnalysis.h"
#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Transformation/DeadCodeElimination.h"
#include "Codira/CHIR/Utils.h"
#include "Codira/CHIR/Value.h"

namespace Codira::CHIR {
/**
 * CHIR Opt Pass: do optimization with analysis results of const value analysis.
 */
class ConstPropagation {
public:
    /**
     * @brief const analysis wrapper to call const analysis.
     */
    using ConstAnalysisWrapper = AnalysisWrapper<ConstAnalysis, ConstDomain>;

    /**
     * @brief constructor to do const propagation.
     * @param builder CHIR builder for generating IR.
     * @param constAnalysisWrapper const analysis wrapper which produce analysis results.
     * @param options global options from Codira inputs.
     */
    explicit ConstPropagation(CHIRBuilder& builder, ConstAnalysisWrapper* constAnalysisWrapper,
        const GlobalOptions& options);

    /**
     * @brief Main process to do const propagation.
     * @param package package to do optimization.
     * @param isDebug flag whether print debug log.
     * @param isCODELint flag whether CODELint is enabled.
     */
    void RunOnPackage(const Ptr<const Package>& package, bool isDebug, bool isCODELint);

    /**
     * @brief Main process to do const propagation per func.
     * @param func func to do optimization.
     * @param isDebug flag whether print debug log.
     * @param isCODELint flag whether CODELint is enabled.
     */
    void RunOnFunc(const Ptr<const Func>& func, bool isDebug, bool isCODELint = false);

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
     * This function will generate a literal value based on the constant information
     * from @p constVal. The type of the literal value is @p type.
     */
    Ptr<LiteralValue> GenerateConstExpr(
        const Ptr<Type>& type, const Ptr<const ConstValue>& constVal, bool isCODELint = false);

    /**
     * This function will rewrite an expression based on the @p rewriteInfo, which stores
     * the exrpession to be rewrited, the index of this expression and the new expression.
     */
    void RewriteToConstExpr(const RewriteInfo& rewriteInfo, bool isDebug) const;

    /**
     * This function will check if a unary expression can be simplified according to the rules
     * of arithmetic when there is *no constant information* about the operand of the expression.
     * If it can be simplified, the usages of the result of this unary expression will be replaced
     * by its operand.
     *
     * Here is a list of the operation this function handles.
     * a) NOT: `!(!b) => b`
     * b) BITNOT: `!(!x) => x`
     *
     * note: `-(-a) != a` as there might be an overflow while calculating `(-a)`.
     */
    void TrySimplifyingUnaryExpr(const Ptr<UnaryExpression>& unary, bool isDebug) const;

    /**
     * This function will check if a binary expression can be simplified according to the rules
     * of arithmetic when there is *no constant information* about the operand of the expression.
     * If it can be simplified, the usages of the result of this binary expression will be replaced
     * by its operand.
     *
     * Here is a list of the operation this function handles.
     * a) ADD: `0 + a => a`, 'a + 0 => a'
     * b) SUB: `a - 0 => a`
     * c) MUL: `1 * a => a`, `a * 1 => a`
     * d) DIV: `a / 1 => a`
     * e) EXP: `a ** 1 => a`
     * f) LSHIFT: `a << 0 => a`
     * g) RSHIFT: `a >> 0 => a`
     * h) BITAND: `a & a => a`
     * j) BITOR: `a | a => a`
     *
     * note: We don't rewrite `0 - a` to `-a` as CodeGen will rewrite `-a` to `0 - a`.
     */
    template <typename T>
    void TrySimplifyingBinaryExpr(const ConstDomain& state, const Ptr<BinaryExpression>& binary, bool isDebug);
    
    /**
     * This function will replaced all use of the result of the expression @p expr with the value
     * @p newVal. A debug message will also be printed if @p isDebug is true.
     */
    void ReplaceUsageOfExprResult(const Ptr<const Expression>& expr, const Ptr<Value>& newVal, bool isDebug) const;

    // ==================== Rewrite Terminator Expressions ==================== //

    void RewriteTerminator(Terminator* oldTerminator, LiteralValue* newValue, Block* newTarget, bool isDebug) const;

    // ==================== Rewrite Terminator Expressions ==================== //

    void RecordEffectMap(const Expression* expr, const Func* func) const;

private:
    CHIRBuilder& builder;
    ConstAnalysisWrapper* analysisWrapper;
    const GlobalOptions& opts;
    static OptEffectCHIRMap effectMap;
    std::vector<const Func*> funcsNeedRemoveBlocks;
};

} // namespace Codira::CHIR

#endif
