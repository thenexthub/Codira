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

#include "IntroduceConstant.h"
#include "../../../CompilerCodiraProject.h"
#include "../TweakRule.h"
#include "../TweakUtils.h"

namespace ark {
const std::unordered_set<Codira::AST::ASTKind> CANNOT_INTRODUCE_CONST_EXPR = {
    ASTKind::RETURN_EXPR, ASTKind::STR_INTERPOLATION_EXPR, ASTKind::INTERPOLATION_EXPR, ASTKind::RANGE_EXPR,
    ASTKind::ARRAY_LIT, ASTKind::BLOCK
};

/**
 * 1. selected range is an expr
 * 2. selected range only contain:
 *    LitConstExpr、global_const_variable's ref or global_const_func's call
 *    or an expression composed of LitConstExpr、global_const_variable's ref or global_const_func's call
 * 3. global_const_func_call's param must is itConstExpr、global_const_variable's ref or global_const_func's call
 */
class IntroduceConstantRule : public TweakRule {
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        auto root = sel.selectionTree.root();
        if (root->selected != SelectionTree::Selection::Complete || !TweakUtils::CheckValidExpr(*root)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceConstant::IntroduceConstantError::INVALID_EXPR))));
            return false;
        }

        bool isValid = true;
        SelectionTree::Walk(root, [&extraOptions, &isValid]
            (const SelectionTree::SelectionTreeNode *treeNode) {
                if (!treeNode || !treeNode->node) {
                    isValid = false;
                    extraOptions.insert(std::make_pair("ErrorCode",
                        std::to_string(static_cast<int>(TweakRule::TweakError::ERROR_AST))));
                    return SelectionTree::WalkAction::STOP_NOW;
                }

                if (treeNode->selected == SelectionTree::Selection::Partial) {
                    isValid = false;
                    extraOptions.insert(std::make_pair("ErrorCode",
                        std::to_string(static_cast<int>(
                            IntroduceConstant::IntroduceConstantError::PARTIAL_SELECTION))));
                    return SelectionTree::WalkAction::STOP_NOW;
                }

                // check leaf node
                if (CANNOT_INTRODUCE_CONST_EXPR.count(treeNode->node->astKind)
                    || (treeNode->Children.empty() && !CheckInvalidConstExpr(treeNode->node))) {
                    isValid = false;
                    extraOptions.insert(std::make_pair("ErrorCode",
                        std::to_string(static_cast<int>(
                            IntroduceConstant::IntroduceConstantError::INVALID_CONST_EXPR))));
                    return SelectionTree::WalkAction::STOP_NOW;
                }

                return SelectionTree::WalkAction::WALK_CHILDREN;
            });
        return isValid;
    }

    static bool CheckInvalidConstExpr(Ptr<Node> leafNode)
    {
        if (!leafNode || (leafNode->astKind != ASTKind::LIT_CONST_EXPR && leafNode->astKind != ASTKind::REF_EXPR)) {
            return false;
        }
        if (leafNode->astKind == ASTKind::LIT_CONST_EXPR) {
            return true;
        }
        auto refExpr = DynamicCast<Codira::AST::RefExpr*>(leafNode);
        if (!refExpr) {
            return false;
        }
        auto target = refExpr->GetTarget();
        if (!target) {
            return false;
        }
        if (!target->IsConst() || !target->TestAttr(Attribute::GLOBAL)) {
            return false;
        }
        return true;
    }
};

std::map<std::string, std::string> IntroduceConstant::ExtraOptions()
{
    return extraOptions;
}

bool IntroduceConstant::Prepare(const Selection &sel)
{
    TweakRuleEngine ruleEngine;

    // add all check rules
    ruleEngine.AddRule(std::make_unique<IntroduceConstantRule>());

    ruleEngine.CheckRules(sel, extraOptions);
    return true;
}

/**
 * const constVar = xxx.
 * insert position: global
 *
 * @param sel Selection
 * @return std::optional<Tweak::Effect>
 */
std::optional<Tweak::Effect> IntroduceConstant::Apply(const Selection &sel)
{
    Effect effect;
    if (!sel.arkAst || !sel.arkAst->sourceManager || !sel.arkAst->file) {
        return std::nullopt;
    }
    std::string filePath = sel.arkAst->file->filePath;
    std::string varName = "constVar";
    auto it = sel.extraOptions.find("suggestName");
    if (it != sel.extraOptions.end()) {
        varName = it->second;
    }

    // get extraction range
    auto root = sel.selectionTree.root();
    if (!root) {
        return std::nullopt;
    }
    Range range = {root->node->begin, root->node->end};
    std::vector<TextEdit> textEdits;
    // insert new variable decl
    textEdits.push_back(InsertDeclaration(sel, range, varName));
    // replace extracted expression with new variable
    textEdits.push_back(ReplaceExprWithVar(sel, range, varName));

    std::string uri = URI::URIFromAbsolutePath(filePath).ToString();
    effect.applyEdits.emplace(uri, std::move(textEdits));
    return std::move(effect);
}

TextEdit IntroduceConstant::InsertDeclaration(const Selection &sel, Range &range, std::string &varName)
{
    TextEdit textEdit;
    if (!sel.arkAst || !sel.arkAst->file) {
        return std::move(textEdit);
    }
    Range insertRange = TweakUtils::FindGlobalInsertPos(*sel.arkAst->file, range);
    insertRange = TransformFromChar2IDE(insertRange);
    std::string insertText;
    std::string sourceCode = sel.arkAst->sourceManager->GetContentBetween(range.start, range.end);
    insertText = "\n\nconst " + varName + " = " + sourceCode + "\n";
    textEdit.range = insertRange;
    textEdit.newText = insertText;
    return std::move(textEdit);
}

TextEdit IntroduceConstant::ReplaceExprWithVar(const Selection &sel, Range &range, std::string &varName)
{
    TextEdit textEdit;
    textEdit.newText = varName;
    // deal Unicode
    std::string charContent = sel.arkAst->sourceManager->GetContentBetween(
        {range.start.fileID, range.start.line, 1}, range.start);
    int size = range.end.column - range.start.column;
    range.start.column = CountUnicodeCharacters(charContent) + 1;
    if (range.end.line == range.start.line) {
        range.end.column = range.start.column + size;
    }
    textEdit.range = TransformFromChar2IDE(range);
    return std::move(textEdit);
}
} // namespace ark
