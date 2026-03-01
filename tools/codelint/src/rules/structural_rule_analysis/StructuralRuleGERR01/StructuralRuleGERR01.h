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

#ifndef CODIRACODECHECK_STRUCTURALRULEGERR01_H
#define CODIRACODECHECK_STRUCTURALRULEGERR01_H

#include <vector>
#include <string>
#include "Codira/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Codira::CodeCheck {
class StructuralRuleGERR01 : public StructuralRule {
public:
    explicit StructuralRuleGERR01(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine){};
    ~StructuralRuleGERR01() override = default;

protected:
    void MatchPattern(ASTContext& ctx, Ptr<AST::Node> node) override;
    void CheckFuncDecl(Ptr<AST::Node>& node);
    void CheckFuncDeclHelper(const Ptr<AST::FuncDecl>& funcDecl, const std::vector<AST::CommentGroup>& vector);
    void AnalyzeFunctionDecl(Ptr<AST::Node> node, const std::vector<AST::CommentGroup>& comments);
    void AnalyzeCatchBlock(std::vector<OwnedPtr<AST::Block>>& blocks);
    static bool CommentsIncludeExceptionInfo(
        const std::vector<AST::CommentGroup>& comments, const std::string& exception);
    void AnalyzeThrowExpr(const Ptr<AST::ThrowExpr>& throwExpr, const std::vector<AST::CommentGroup>& comments);
};
} // namespace Codira::CodeCheck

#endif
