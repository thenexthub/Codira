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

#ifndef CODIRA_CHIR_TRANSFORMATION_DEAD_CODE_ELIMINATION_H
#define CODIRA_CHIR_TRANSFORMATION_DEAD_CODE_ELIMINATION_H

#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/DiagAdapter.h"
#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/Package.h"
#include "Codira/Utils/TaskQueue.h"

namespace Codira::CHIR {
/**
 * CHIR Opt Pass: summary of useless code elimination pass.
 */
class DeadCodeElimination {
public:
    /**
     * @brief constructor for dead code elimination pass.
     * @param builder CHIR builder for generating IR.
     * @param diag codira error or warning reporter.
     * @param packageName this package name.
     */
    explicit DeadCodeElimination(CHIRBuilder& builder, DiagAdapter& diag, const std::string& packageName);

    /**
     * @brief process to do useless function elimination.
     * @param package package to do dead code elimination.
     * @param opts global options from Codira inputs.
     */
    void UselessFuncElimination(Package& package, const GlobalOptions& opts);

    /**
     * @brief process to do useless expr elimination.
     * @param package package to do dead code elimination.
     * @param isDebug flag whether print debug log.
     */
    void UselessExprElimination(const Package& package, bool isDebug) const;

    /**
     * @brief process to delete nothing type.
     * @param package package to do dead code elimination.
     * @param isDebug flag whether print debug log.
     */
    void NothingTypeExprElimination(const Package& package, bool isDebug);

    /**
     * @brief process to do unreachable block elimination.
     * @param package package to do dead code elimination.
     * @param isDebug flag whether print debug log.
     */
    void UnreachableBlockElimination(const Package& package, bool isDebug) const;

    /**
     * @brief process to do unreachable block elimination.
     * @param funcs functions to do dead code elimination.
     * @param isDebug flag whether print debug log.
     */
    void UnreachableBlockElimination(const std::vector<const Func*>& funcs, bool isDebug) const;
    /**
     * @brief process to report unreachable block warning.
     * @param package package to report warning.
     * @param threadsNum threads num join to do this pass.
     * @param maybeUnreachableBlocks may be unreachable blocks to report.
     */
    void UnreachableBlockWarningReporter(const Package& package,
        size_t threadsNum, const std::unordered_map<Block*, Terminator*>& maybeUnreachableBlocks);

    /**
     * @brief process to remove blocks which is marked unreachable.
     * @param package package to clear block marked unreachable.
     */
    void ClearUnreachableMarkBlock(const Package& package) const;

    /**
     * @brief process to report unused block warning.
     * @param package package to report warning.
     * @param opts global options from Codira inputs.
     */
    void ReportUnusedCode(const Package& package, const GlobalOptions& opts);

private:
    CHIRBuilder& builder;
    DiagAdapter& diag;
    const std::string& currentPackageName;
    const std::string GLOBAL_INIT_MANGLED_NAME = "_global_init";
    const std::string STD_CORE_FUTURE_MANGLED_NAME = "_CNat6Future";
    const std::string STD_CORE_EXECUTE_CLOSURE_MANGLED_NAME = "executeClosure";

    // =============== Functions for Useless Variable Check =============== //
    void UselessVariableCheckForFunc(const BlockGroup& funcBody, bool isDebug);
    bool CheckOneUsers(const std::vector<Expression*>& users) const;
    bool CheckTwoUsers(const std::vector<Expression*>& users) const;
    void UselessExprEliminationForFunc(const Func& func, bool isDebug) const;
    
    // =============== Functions for Nothing type Check =============== //
    void NothingTypeExprEliminationForFunc(BlockGroup& funcBody, bool isDebug);
    
    static bool CheckAllUsersIsNotUse(const Value& value, const std::vector<Expression*>& users);

    // =============== Functions for Useless Func Elimination =============== //
    bool CheckUselessFunc(const Func& func, const GlobalOptions& opts);

    // =============== Functions for Unreachable Block Elimination =============== //
    bool CheckUselessBlock(const Block& block) const;
    void BreakBranchConnection(const Block& block) const;
    void ClearUnreachableMarkBlockForFunc(const BlockGroup& body) const;
    void UnreachableBlockEliminationForFunc(const BlockGroup& body, bool isDebug) const;
    
    // =============== Functions for Useless IR Elimination =============== //
    bool CheckUselessExpr(const Expression& expr, bool isReportWarning = false) const;

    // =============== Functions for Debug Message Dump =============== //
    Ptr<Expression> GetUnreachableExpression(const CHIR::Block& block, bool& isNormal) const;
    void PrintUnreachableBlockWarning(
        const CHIR::Block& block, const CHIR::Terminator& terminator, bool& isPrinted);

    // =============== Functions for dce reporter =============== //
    void TryReportUnusedOnExpr(Expression& expr, const GlobalOptions& opts, bool blockUsed);
    void ReportUnusedFunc(const Func& func, const GlobalOptions& opts);
    void ReportUnusedGlobalVar(const GlobalVar& globalVar);
    void DiagUnusedVariable(const Debug& expr);
    void ReportUnusedLocalVariable(const Expression& expr, bool isDebug);
    void ReportUnusedExpression(Expression& expr);
    template <typename... Args> void DiagUnusedCode(
        const std::pair<bool, Codira::Range>& nodeRange, DiagKindRefactor diagKind, Args&& ... args);
    void DiagUnusedVariableForParam(const Debug& expr);
    void DiagUnusedVariableForLocalVar(const Debug& expr, bool isDebug);
    void DiagUnusedLambdaVariable(const Debug& expr);
    std::string GetLiteralFromExprKind(const ExprKind& kind) const;

    // ============== Functions for clean code in parallel ===========//
    void ReportUnusedCodeInFunc(const BlockGroup& body, const GlobalOptions& opts);
    void UnreachableBlockWarningReporterInSerial(
        const Package& package, const std::unordered_map<Block*, Terminator*>& maybeUnreachableBlocks);
    void UnreachableBlockWarningReporterInParallel(const Package& package,
        size_t threadsNum, const std::unordered_map<Block*, Terminator*>& maybeUnreachableBlocks);
};
} // namespace Codira::CHIR
#endif
