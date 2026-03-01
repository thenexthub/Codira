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

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUNC_03_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUNC_03_H
#include <set>
#include <stack>
#include <utility>
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
/**
 * G.FUN.03 避免在无关的函数之间重用名字，构成重载
 */
class StructuralRuleGFUNC03 : public StructuralRule {
public:
    explicit StructuralRuleGFUNC03(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGFUNC03() override = default;
    enum class Status {
        IN_DIFF_SCOPE,
        SUB_CLASS,
        NONE
    };
    struct FuncInfo {
        std::string identifier;
        std::vector<Ptr<AST::Ty>> paramTys;
        Ptr<AST::Block> block;
        FuncInfo(std::string identifier, std::vector<Ptr<AST::Ty>> paramTys, Ptr<AST::Block> block)
            : identifier(std::move(identifier)), paramTys(std::move(paramTys)), block(block)
        {
        }
        bool operator<(const FuncInfo& other) const
        {
            if (identifier == other.identifier) {
                return paramTys.size() <= other.paramTys.size();
            }
            return identifier < other.identifier;
        }
    };

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Codira::AST::Node> node) override;
    Status CheckFuncDecl(std::set<FuncInfo>& allFuncs, FuncInfo target);
    void PrintDiagnoseInfo(FuncInfo& func);

private:
    void CheckFuncOverload(Ptr<Codira::AST::Node> node);
    void FuncDeclProcessor(Ptr<Codira::AST::Node> node);
    void FileProcessor(Ptr<Codira::AST::Node> node);
    std::stack<AST::Block*> blocks;
    std::map<AST::Block*, std::set<FuncInfo>> localFuncInBlock;
    std::set<FuncInfo> allFuncs;
};
} // namespace Codira::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUNC_03_H
