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

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CON_03_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CON_03_H

#include "Codira/AST/Match.h"
#include "Codira/AST/Walker.h"
#include "Codira/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * G.CON.03 禁止使用非线程安全的函数来覆写线程安全的函数
 */
class StructuralRuleGCON03 : public StructuralRule {
public:
    enum class MutexState {
        NOT_MUTEX = 0,
        MUTEX_LOCK,
        MUTEX_UNLOCK
    };
    explicit StructuralRuleGCON03(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGCON03() override = default;

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override;

private:
    using PositionPair = std::pair<Position, Position>;
    std::set<std::pair<std::pair<std::string, std::string>, PositionPair>> nonThreadSafeOverrideFuncSet;
    std::set<std::pair<std::string, std::string>> threadSafeBaseFuncSet;
    std::set<Codira::Position> safeFuncSet;
    std::set<Codira::Position> checkedFuncSet;
    void OverrideFuncFinder(Ptr<Codira::AST::Node> node);
    void BaseClassFinder(Ptr<Codira::AST::Node> node);
    void CheckBaseClassFuncSafe(std::vector<OwnedPtr<Codira::AST::Decl>> &decls, const std::string &className);
    bool CoverDeclToFuncDecl(Ptr<Codira::AST::Decl> decl);
    static bool IsOverrideModifier(const std::set<Codira::AST::Modifier> &modifiers);
    bool IsThreadSafe(std::vector<OwnedPtr<Codira::AST::Node>> &nodes);
    bool IsFuncSafe(Ptr<Codira::AST::CallExpr> callExpr);
    static bool IsAssignSafe(Ptr<Codira::AST::AssignExpr> assignExpr);
    static bool IsIncOrDecSafe(Ptr<Codira::AST::IncOrDecExpr> incOrDecExpr);
    static MutexState IsReentrantMutex(Ptr<Codira::AST::CallExpr> callExpr);
    void ResultCheck(
        const std::set<std::pair<std::pair<std::string, std::string>, PositionPair>> &unsafeThreadOverrideFuncSet,
        const std::set<std::pair<std::string, std::string>> &safeThreadBaseFuncSet);
};
}
#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CON_03_H
