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

#include "ExtractVariable.h"
#include "../../../CompilerCodiraProject.h"
#include "../TweakRule.h"
#include "../TweakUtils.h"

namespace ark {
const std::unordered_set<Codira::AST::ASTKind> INVALID_PARTIAL_EXPR = {
    ASTKind::IF_EXPR, ASTKind::DO_WHILE_EXPR, ASTKind::TRY_EXPR, ASTKind::BLOCK
};

const std::unordered_set<Codira::AST::ASTKind> CANNOT_EXTRACT_VAR_EXPR = {
    ASTKind::BLOCK, ASTKind::STR_INTERPOLATION_EXPR, ASTKind::INTERPOLATION_EXPR
};

class ExtractVariableRule : public TweakRule {
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        auto root = sel.selectionTree.root();
        if (!root) {
            return false;
        }
        if (root->selected != SelectionTree::Selection::Complete || !TweakUtils::CheckValidExpr(*root)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(ExtractVariable::ExtractVariableError::FAIL_GET_ROOT_EXPR))));
            return false;
        }
        if (root->selected == SelectionTree::Selection::Complete && root->node
            && CANNOT_EXTRACT_VAR_EXPR.count(root->node->astKind)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(ExtractVariable::ExtractVariableError::INVALID_CODE_SEGMENT))));
            return false;
        }
        if (root->node->IsExpr() && root->selected == SelectionTree::Selection::Complete) {
            return true;
        }
        if (INVALID_PARTIAL_EXPR.count(root->node->astKind) && root->selected == SelectionTree::Selection::Partial) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(ExtractVariable::ExtractVariableError::FAIL_GET_ROOT_EXPR))));
            return false;
        }

        bool isValid = true;

        SelectionTree::Walk(root, [&root, &extraOptions, &isValid]
            (const SelectionTree::SelectionTreeNode *treeNode) {
            if (!treeNode || !treeNode->node) {
                isValid = false;
                extraOptions.insert(std::make_pair("ErrorCode",
                    std::to_string(static_cast<int>(TweakRule::TweakError::ERROR_AST))));
                return SelectionTree::WalkAction::STOP_NOW;
            }
            if (treeNode == root) {
                return SelectionTree::WalkAction::WALK_CHILDREN;
            }

            if (treeNode->selected == SelectionTree::Selection::Complete && !treeNode->node->IsExpr()) {
                isValid = false;
                extraOptions.insert(std::make_pair("ErrorCode",
                    std::to_string(static_cast<int>(ExtractVariable::ExtractVariableError::INVALID_EXPR))));
                return SelectionTree::WalkAction::STOP_NOW;
            }

            return SelectionTree::WalkAction::SKIP_CHILDREN;
        });
        return isValid;
    }
};

bool ExtractVariable::Prepare(const Selection &sel)
{
    TweakRuleEngine ruleEngine;

    // add all check rules
    ruleEngine.AddRule(std::make_unique<ExtractVariableRule>());

    ruleEngine.CheckRules(sel, extraOptions);
    return true;
}

std::optional<Tweak::Effect> ExtractVariable::Apply(const Selection &sel)
{
    Effect effect;
    if (!sel.arkAst || !sel.arkAst->sourceManager || !sel.arkAst->file) {
        return std::nullopt;
    }
    std::string filePath = sel.arkAst->file->filePath;
    std::string varName = "newVariable";
    auto it = sel.extraOptions.find("suggestName");
    if (it != sel.extraOptions.end()) {
        varName = it->second;
    }

    // get extraction range
    Range range = GetExtractionRange(sel);
    if (range.start == Position{0, 0, 0} || range.start == range.end) {
        return std::nullopt;
    }
    std::vector<TextEdit> textEdits;
    // insert new variable decl
    textEdits.push_back(InsertDeclaration(sel, range, varName));
    // replace extracted expression with new variable
    textEdits.push_back(ReplaceExprWithVar(sel, range, varName));

    std::string uri = URI::URIFromAbsolutePath(filePath).ToString();
    effect.applyEdits.emplace(uri, std::move(textEdits));
    return std::move(effect);
}

std::map<std::string, std::string> ExtractVariable::ExtraOptions()
{
    return extraOptions;
}


Range ExtractVariable::GetExtractionRange(const Tweak::Selection &sel)
{
    Range range;
    if (sel.range.start.fileID == sel.range.end.fileID && sel.range.start == sel.range.end) {
        return range;
    }
    auto root = sel.selectionTree.root();
    if (!root || !root->node) {
        return range;
    }
    if (root->selected == SelectionTree::Selection::Complete) {
        range.start = root->node->begin;
        range.end = root->node->end;
        return range;
    }

    SelectionTree::Walk(root, [&root, &range](const SelectionTree::SelectionTreeNode *treeNode) {
        if (!treeNode || !treeNode->node) {
            return SelectionTree::WalkAction::STOP_NOW;
        }
        if (treeNode == root) {
            return SelectionTree::WalkAction::WALK_CHILDREN;
        }

        if (treeNode->selected == SelectionTree::Selection::Complete && treeNode->node->IsExpr()) {
            if (range.start.line == 0) {
                range.start = treeNode->node->begin;
            }
            if (range.end.line == 0) {
                range.end = treeNode->node->end;
            }
            if (treeNode->node->begin < range.start) {
                range.start = treeNode->node->begin;
            }
            if (treeNode->node->end > range.end) {
                range.end = treeNode->node->end;
            }
        }

        return SelectionTree::WalkAction::WALK_CHILDREN;
    });
    return range;
}

TextEdit ExtractVariable::InsertDeclaration(const Selection &sel, Range &range, std::string &varName)
{
    TextEdit textEdit;
    if (!sel.arkAst->sourceManager) {
        return textEdit;
    }
    auto root = sel.selectionTree.root();
    if (!root || !root->node) {
        return textEdit;
    }

    Range insertRange;
    std::string indent;
    bool isGlobal = false;
    FindInsertDeclPosition(sel, range, insertRange, indent, isGlobal);
    if (insertRange.end.IsZero()) {
        return textEdit;
    }

    insertRange = TransformFromChar2IDE(insertRange);
    std::ostringstream insertText;
    std::string sourceCode = sel.arkAst->sourceManager->GetContentBetween(range.start, range.end);
    if (isGlobal) {
        insertText << "\n";
    }

    std::string modifier = GetVarModifier(sel, range);
    if (modifier.find("const") != std::string::npos) {
        insertText << modifier;
    } else {
        insertText << modifier << "var ";
    }

    if (root->selected == SelectionTree::Selection::Complete && root->node->astKind == ASTKind::ASSIGN_EXPR) {
        insertText <<  varName << " = (" << sourceCode << ")\n" << indent;
    } else {
        insertText << varName << " = " << sourceCode << "\n" << indent;
    }

    textEdit.range = insertRange;
    textEdit.newText = insertText.str();
    return std::move(textEdit);
}

void ExtractVariable::FindInsertDeclPosition(const Selection &sel, Range &range,
    Range &insertRange, std::string &indent, bool &isGlobal)
{
    if (!sel.arkAst) {
        return;
    }
    // 1. use scopeName getting insert position
    FindInsertPositionByScopeName(sel, range, insertRange, isGlobal);
    if (!insertRange.end.IsZero()) {
        return;
    }
    // 2. compute insert position if getting insert position fail by scope name (maybe the following code can delete)
    int firstToken4CurLine = GetFirstTokenOnCurLine(sel.arkAst->tokens, range.start.line);
    if (firstToken4CurLine == -1) {
        insertRange.start = {range.start.fileID, range.start.line, 1};
        insertRange.end = insertRange.start;
    } else {
        Token firstToken = sel.arkAst->tokens[firstToken4CurLine];
        insertRange.start = firstToken.Begin();
        insertRange.end = insertRange.start;
        indent = std::string (firstToken.Begin().column > 0 ? firstToken.Begin().column - 1 : 0, ' ');
    }

    // deal do while
    auto targetDecl = sel.selectionTree.TargetDecl();
    if (!targetDecl) {
        return;
    }
    Walker(targetDecl, [&](auto node) {
        if (!node) {
            return VisitAction::STOP_NOW;
        }
        if (node->astKind == ASTKind::DO_WHILE_EXPR && node->begin <= range.start && node->end >= range.end) {
            auto doWhileExpr = DynamicCast<Codira::AST::DoWhileExpr*>(node.get());
            if (doWhileExpr && doWhileExpr->condExpr && doWhileExpr->condExpr->begin <= range.start
                && doWhileExpr->condExpr->end >= range.end) {
                insertRange = {doWhileExpr->begin, doWhileExpr->begin};
                indent = std::string (doWhileExpr->begin.column, ' ');
                return VisitAction::STOP_NOW;
            }
        }

        if (node->astKind == ASTKind::IF_EXPR && node->begin <= range.start && node->end >= range.end) {
            auto ifExpr = DynamicCast<Codira::AST::IfExpr*>(node.get());
            if (!ifExpr) {
                return VisitAction::STOP_NOW;
            }
            if (DealIfExpr(*ifExpr, range)) {
                insertRange = {ifExpr->begin, ifExpr->begin};
                indent = std::string (ifExpr->begin.column, ' ');
                return VisitAction::STOP_NOW;
            }
        }

        return VisitAction::WALK_CHILDREN;
    }).Walk();

    // deal same multi statement
    DealMultStatementOnSameLine(sel, range, firstToken4CurLine, insertRange);
}

void ExtractVariable::FindInsertPositionByScopeName(const Selection &sel, Range &range,
    Range &insertRange, bool &isGlobal)
{
    if (!sel.arkAst || !sel.arkAst->packageInstance || !sel.arkAst->packageInstance->ctx) {
        return;
    }
    auto root = sel.selectionTree.root();
    if (!root || !root->node) {
        return;
    }
    Ptr<Block> block = nullptr;
    if (sel.selectionTree.OuterInterpExpr()) {
        if (auto expr = DynamicCast<Expr*>(sel.selectionTree.OuterInterpExpr())) {
            block = TweakUtils::FindOuterScopeNode(
                *sel.arkAst->packageInstance->ctx, *expr, isGlobal, range);
            GetInsertRange(sel, range, isGlobal, block, insertRange);
            return;
        }
    }

    SelectionTree::Walk(root, [&sel, &isGlobal, &block, &range]
        (const SelectionTree::SelectionTreeNode *treeNode) {
            if (!treeNode || !treeNode->node) {
                return SelectionTree::WalkAction::STOP_NOW;
            }
            if (treeNode->selected == SelectionTree::Selection::Complete && treeNode->node->IsExpr()
                && !treeNode->node->scopeName.empty()) {
                if (auto expr = DynamicCast<Expr*>(treeNode->node)) {
                    block = TweakUtils::FindOuterScopeNode(
                        *sel.arkAst->packageInstance->ctx, *expr, isGlobal, range);
                }
                return SelectionTree::WalkAction::STOP_NOW;
            }

            return SelectionTree::WalkAction::WALK_CHILDREN;
        });
    GetInsertRange(sel, range, isGlobal, block, insertRange);
}

void ExtractVariable::GetInsertRange(
    const Tweak::Selection &sel, Range &range, bool isGlobal, Ptr<Block> &block, Range &insertRange)
{
    if (isGlobal && sel.arkAst && sel.arkAst->file) {
        insertRange = TweakUtils::FindGlobalInsertPos(*sel.arkAst->file, range);
        return;
    }
    if (block) {
        for (const auto &subNode : block->body) {
            if (subNode && subNode->begin <= range.start && subNode->end >= range.end) {
                insertRange = {subNode->begin, subNode->begin};
                return;
            }
        }
    }
}

bool ExtractVariable::DealIfExpr(IfExpr &ifExpr, Range &range)
{
    if (ifExpr.condExpr && ifExpr.condExpr->begin <= range.start
                && ifExpr.condExpr->end >= range.end) {
        return true;
    }
    if (ifExpr.elseBody && ifExpr.elseBody->astKind == ASTKind::IF_EXPR && ifExpr.elseBody->begin <= range.start
                && ifExpr.elseBody->end >= range.end) {
        auto elseIf = DynamicCast<Codira::AST::IfExpr>(ifExpr.elseBody.get());
        if (!elseIf) {
            return false;
        }
        return DealIfExpr(*elseIf, range);
    } else {
        return false;
    }
}

void ExtractVariable::DealMultStatementOnSameLine(const Tweak::Selection &sel, const Range &range,
    int firstToken4CurLine, Range &insertRange)
{
    int lastToken4CurLine = GetLastTokenOnCurLine(sel.arkAst->tokens, range.start.line);
    if (lastToken4CurLine == -1) {
        return;
    }
    std::vector<Token> curLineTokens =
        std::vector<Token>(sel.arkAst->tokens.begin() + firstToken4CurLine,
            sel.arkAst->tokens.begin() + lastToken4CurLine);
    if (curLineTokens.empty()) {
        return;
    }
    int selStart = sel.arkAst->GetCurTokenByStartColumn(range.start, 0,
        static_cast<int>(sel.arkAst->tokens.size()) - 1);
    if (selStart == -1 || selStart < firstToken4CurLine || selStart > lastToken4CurLine) {
        return;
    }
    int selToken = selStart - firstToken4CurLine;
    if (selToken < 0 || selToken >= curLineTokens.size()) {
        return;
    }
    int i = selToken;
    for (; i >= 0; i--) {
        if (curLineTokens[i].Value() == "{" || curLineTokens[i].Value() == ";" || curLineTokens[i].Value() == "=>") {
            break;
        }
    }
    if (i < 0) {
        return;
    }
    insertRange = {curLineTokens[i].End(), curLineTokens[i].End()};
}

std::string ExtractVariable::GetVarModifier(const Selection &sel, Range &range)
{
    std::string modifier;
    if (!sel.selectionTree.TopDecl()) {
        return modifier;
    }
    Position start = range.start;
    Position end = range.end;
    Walker(sel.selectionTree.TopDecl(), [&](auto node) {
        if (!node) {
            return VisitAction::STOP_NOW;
        }
        if (node->begin > end || node->end < start) {
            return VisitAction::SKIP_CHILDREN;
        }
        if (node->isInMacroCall) {
            return VisitAction::SKIP_CHILDREN;
        }
        if (node->TestAttr(Attribute::COMPILER_ADD)) {
            return VisitAction::SKIP_CHILDREN;
        }

        if (node->begin <= start && node->end >= end && !node->TestAttr(Attribute::COMPILER_ADD)) {
            MatchModifier(*node, modifier);
            return VisitAction::WALK_CHILDREN;
        }

        return VisitAction::WALK_CHILDREN;
    }).Walk();

    return modifier;
}

void ExtractVariable::MatchModifier(Codira::AST::Node &node, std::string &modifier)
{
    std::string targetModifier;
    Meta::match(node)(
        [&](Codira::AST::AssignExpr &assignExpr) {
            if (!assignExpr.leftValue || !assignExpr.leftValue->GetTarget()) {
                return;
            }
            auto decl = assignExpr.leftValue->GetTarget();
            if (decl->TestAttr(Attribute::STATIC)) {
                targetModifier = "static ";
            }
            if (decl->IsConst()) {
                targetModifier += "const ";
            }
            modifier = targetModifier;
            return;
        },
        [&](Codira::AST::VarDecl &varDecl) {
            if (varDecl.TestAttr(Attribute::STATIC)) {
                targetModifier = "static ";
            }
            if (varDecl.IsConst()) {
                targetModifier += "const ";
            }
            modifier = targetModifier;
            return;
        },
        [&]() {
            return;
        });
}

TextEdit ExtractVariable::ReplaceExprWithVar(const Selection &sel, Range &range, std::string &varName)
{
    TextEdit textEdit;
    textEdit.newText = varName;
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
