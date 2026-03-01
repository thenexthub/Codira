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

#include "ExtractFunction.h"
#include "../../../logger/Logger.h"
#include "../../reference/FindReferencesImpl.h"
#include "../../../CompilerCodiraProject.h"
#include "../TweakRule.h"

namespace ark {
const std::unordered_set<Codira::TokenKind> COMPOUND_ASSIGN_OPERATORS = { TokenKind::EXP_ASSIGN, TokenKind::MUL_ASSIGN,
    TokenKind::DIV_ASSIGN, TokenKind::ADD_ASSIGN, TokenKind::SUB_ASSIGN, TokenKind::MOD_ASSIGN,
    TokenKind::LSHIFT_ASSIGN, TokenKind::RSHIFT_ASSIGN, TokenKind::AND_ASSIGN, TokenKind::BITXOR_ASSIGN,
    TokenKind::BITAND_ASSIGN, TokenKind::BITOR_ASSIGN, TokenKind::OR_ASSIGN, TokenKind::INCR, TokenKind::DECR
};

const std::unordered_set<Codira::AST::ASTKind> CANNOT_EXTRACT_FUNC_EXPR = {
    ASTKind::LAMBDA_EXPR, ASTKind::INTERPOLATION_EXPR
};

bool IsRefLoop(const Symbol& sym, const Node& self)
{
    if (!sym.node) {
        return false;
    }

    Ptr<Expr> condExpr = nullptr;
    Ptr<Expr> guardExpr = nullptr;
    // As for while (true) { while (break) {...} }, the break binds to the outer while loop.
    if (sym.node->astKind == ASTKind::WHILE_EXPR) {
        condExpr = RawStaticCast<WhileExpr*>(sym.node)->condExpr.get();
    } else if (sym.node->astKind == ASTKind::DO_WHILE_EXPR) {
        condExpr = RawStaticCast<DoWhileExpr*>(sym.node)->condExpr.get();
    } else if (sym.node->astKind == ASTKind::FOR_IN_EXPR) {
        condExpr = RawStaticCast<ForInExpr*>(sym.node)->inExpression.get();
        guardExpr = RawStaticCast<ForInExpr*>(sym.node)->patternGuard.get();
    } else {
        return sym.node->IsLoopExpr();
    }

    bool isInLoopCond = false;
    auto markIsInLoopCond = [&isInLoopCond, &self](Ptr<const Node> node) -> VisitAction {
        switch (node->astKind) {
            case ASTKind::JUMP_EXPR:
                if (&self == node) {
                    isInLoopCond = true;
                }
                return VisitAction::SKIP_CHILDREN;
            case ASTKind::WHILE_EXPR:
            case ASTKind::DO_WHILE_EXPR:
            case ASTKind::FOR_IN_EXPR:
            case ASTKind::FUNC_DECL:
            case ASTKind::LAMBDA_EXPR:
                return VisitAction::SKIP_CHILDREN;
            default:
                return VisitAction::WALK_CHILDREN;
        }
    };
    Walker w(condExpr, nullptr, markIsInLoopCond);
    w.Walk();
    if (guardExpr) {
        Walker w2(guardExpr, nullptr, markIsInLoopCond);
        w2.Walk();
    }
    return !isInLoopCond;
}

Symbol* GetSatisfiedSymbol(const ASTContext& ctx, const std::string& scopeName,
    const std::function<bool(AST::Symbol&)>& satisfy, const std::function<bool(AST::Symbol&)>& fail)
{
    std::string scopeGateName = ScopeManagerApi::GetScopeGateName(scopeName);
    while (!scopeGateName.empty()) {
        auto sym = ScopeManagerApi::GetScopeGate(ctx, scopeGateName);
        if (!sym) {
            std::string tmpScopeName = ScopeManagerApi::GetParentScopeName(scopeGateName);
            scopeGateName = ScopeManagerApi::GetScopeGateName(tmpScopeName);
            continue;
        }
        if (fail(*sym)) {
            return nullptr;
        }
        if (satisfy(*sym)) {
            return sym;
        }
        scopeGateName = ScopeManagerApi::GetScopeGateName(sym->scopeName);
    }
    return nullptr;
}

// find LoopExpr of JumpExpr by JumpExpr scopeName
Ptr<Expr> FindLoopExpr(const ASTContext& ctx, const JumpExpr& jumpExpr)
{
    auto sym = GetSatisfiedSymbol(ctx, jumpExpr.scopeName,
        [&jumpExpr](auto& sym) { return IsRefLoop(sym, jumpExpr); },
        [](auto& sym) { return sym.node->astKind == ASTKind::FUNC_BODY; });
    return sym ? DynamicCast<Expr*>(sym->node) : nullptr;
}

bool IsSatisfyJumpExpr(const Tweak::Selection &sel, const JumpExpr& jumpExpr,
    std::map<std::string, std::string> &extraOptions)
{
    if (!sel.arkAst || !sel.arkAst->packageInstance || !sel.arkAst->packageInstance->ctx) {
        return false;
    }
    auto loopExpr = FindLoopExpr(*sel.arkAst->packageInstance->ctx, jumpExpr);
    if (!loopExpr) {
        return false;
    }
    auto root = sel.selectionTree.root();
    bool isValid = false;
    SelectionTree::Walk(root, [&loopExpr, &extraOptions, &isValid]
        (const SelectionTree::SelectionTreeNode *node) {
            if (!node->node) {
                isValid = false;
                return SelectionTree::WalkAction::STOP_NOW;
            }

            if (node->node == loopExpr && node->selected == SelectionTree::Selection::Complete) {
                isValid = true;
                return SelectionTree::WalkAction::STOP_NOW;
            }

            return SelectionTree::WalkAction::WALK_CHILDREN;
        });
    if (!isValid) {
        extraOptions.insert(std::make_pair("ErrorCode",
            std::to_string(static_cast<int>(ExtractFunction::ExtractFunctionError::PARTIAL_JUMP_EXPR))));
        return false;
    }
    return true;
}

// compute whether the decl need extract to return value:
bool NeedExtractDecl2ReturnValue(Codira::AST::Decl *decl, const Tweak::Selection &sel)
{
    if (!decl) {
        return false;
    }
    ReferencesResult result;
    FindReferencesImpl::GetCurPkgUesage(decl, *sel.arkAst, result);
    for (const auto &reference : result.References) {
        Range refRange = reference.range;
        refRange.start = PosFromIDE2Char(refRange.start);
        refRange.end = PosFromIDE2Char(refRange.end);
        if (refRange.start.fileID != sel.arkAst->fileID) {
            return true;
        }
        if (refRange.start >= sel.range.end || refRange.end > sel.range.end) {
            return true;
        }
    }
    return false;
}

// compute whether the AssignExpr->leftValue need extract to return value
bool NeedExtractAssignExpr2ReturnValue(Codira::AST::AssignExpr *assignExpr, const Tweak::Selection &sel)
{
    if (!assignExpr || !assignExpr->leftValue) {
        return false;
    }
    if (assignExpr->leftValue->astKind == ASTKind::MEMBER_ACCESS) {
        return false;
    }
    auto target = assignExpr->leftValue->GetTarget();
    if (!target || (target->astKind != ASTKind::VAR_DECL && target->astKind != ASTKind::FUNC_PARAM)) {
        return false;
    }
    if (target->TestAnyAttr(Attribute::IN_CLASSLIKE, Attribute::IN_STRUCT,
            Attribute::IN_ENUM, Attribute::GLOBAL)) {
        return false;
    }
    if (target->begin.fileID == sel.range.start.fileID && target->begin >= sel.range.start
        && target->end <= sel.range.end) {
        return false;
    }
    ReferencesResult result;
    FindReferencesImpl::GetCurPkgUesage(target, *sel.arkAst, result);
    for (const auto &reference : result.References) {
        Range refRange = reference.range;
        refRange.start = PosFromIDE2Char(refRange.start);
        refRange.end = PosFromIDE2Char(refRange.end);
        if (refRange.start.fileID != sel.arkAst->fileID) {
            return true;
        }
        if (refRange.start >= sel.range.end || refRange.end > sel.range.end) {
            return true;
        }
    }
    return false;
}

bool LeftValueIsMemberVar(Codira::AST::AssignExpr *assignExpr)
{
    if (!assignExpr || !assignExpr->leftValue) {
        return false;
    }
    if (assignExpr->leftValue->astKind == ASTKind::MEMBER_ACCESS) {
        auto memberAccess = DynamicCast<MemberAccess*>(assignExpr->leftValue.get());
        if (!memberAccess || !memberAccess->baseExpr) {
            return false;
        }
        if (memberAccess->baseExpr->ToString() == "this") {
            return true;
        }
        return false;
    }
    if (assignExpr->leftValue->astKind == ASTKind::REF_EXPR) {
        auto refExpr = DynamicCast<RefExpr*>(assignExpr->leftValue.get());
        if (!refExpr || !refExpr->GetTarget()) {
            return false;
        }
        if (refExpr->GetTarget()->astKind == ASTKind::VAR_DECL
            && refExpr->GetTarget()->TestAnyAttr(Attribute::IN_CLASSLIKE,
                   Attribute::IN_STRUCT, Attribute::IN_ENUM)) {
            return true;
        }
    }
    return false;
}

/**
 * check the root's children, cannot extract function:
 * 1. selected code is GLOBAL_VAR/MEMBER_VAR complete definition
 * 2. exist partial selection node
 * 3. not contain an expr
 * 4. selected range is in constructor and selected range contain the member variable assign_expr
 */
class ExtractFunctionSelectionRule : public TweakRule {
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        if (!PreCheck(sel, extraOptions)) {
            return false;
        }

        auto root = sel.selectionTree.root();
        bool isValid = true;
        bool containExpr = false;
        bool isConstructor = false;
        bool containMemVarAssign = false;
        if (root->selected == SelectionTree::Selection::Complete && root->node
            && root->node->astKind == ASTKind::EXPR) {
            containExpr = true;
        }

        if (sel.selectionTree.SelectedScope() == SelectionTree::Scope::FUNC_BODY
            && sel.selectionTree.TargetDecl() && sel.selectionTree.TargetDecl()->TestAttr(Attribute::CONSTRUCTOR)) {
            isConstructor = true;
        }
        SelectionTree::Walk(root, [&root, &extraOptions, &isValid, &containExpr, &containMemVarAssign]
            (const SelectionTree::SelectionTreeNode *treeNode) {
            if (!treeNode->node) {
                isValid = false;
                extraOptions.insert(std::make_pair("ErrorCode",
                    std::to_string(static_cast<int>(TweakRule::TweakError::ERROR_AST))));
                return SelectionTree::WalkAction::STOP_NOW;
            }

            if (treeNode != root && treeNode->selected == SelectionTree::Selection::Partial) {
                extraOptions.insert(std::make_pair("ErrorCode",
                    std::to_string(static_cast<int>(ExtractFunction::ExtractFunctionError::PARTIAL_SELECTION))));
                isValid = false;
                return SelectionTree::WalkAction::STOP_NOW;
            }

            if (treeNode->selected == SelectionTree::Selection::Complete && treeNode->node->IsExpr()) {
                containExpr = true;
            }

            if (treeNode->selected == SelectionTree::Selection::Complete &&
                treeNode->node->astKind == ASTKind::ASSIGN_EXPR
                && !containMemVarAssign) {
                auto assignExpr = DynamicCast<AssignExpr*>(treeNode->node.get());
                containMemVarAssign = LeftValueIsMemberVar(assignExpr);
            }

            return SelectionTree::WalkAction::WALK_CHILDREN;
        });

        if (!containExpr) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(ExtractFunction::ExtractFunctionError::INVALID_CODE_SEGMENT))));
            return false;
        }

        if (isConstructor && containMemVarAssign) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(ExtractFunction::ExtractFunctionError::CONTAIN_MEMBER_VAR_INIT))));
            return false;
        }

        return isValid;
    }

    bool PreCheck(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const
    {
        auto root = sel.selectionTree.root();
        if (sel.selectionTree.SelectedScope() == SelectionTree::Scope::MEMBER_VAR) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(ExtractFunction::ExtractFunctionError::CONTAIN_MEMBER_VAR_INIT))));
            return false;
        }

        if (root->selected == SelectionTree::Selection::Complete && root->node
            && root->node->astKind == ASTKind::VAR_DECL) {
            auto decl = dynamic_cast<VarDecl*>(root->node.get());
            if (decl && decl->TestAnyAttr(Attribute::GLOBAL, Attribute::IN_STRUCT,
                            Attribute::IN_CLASSLIKE, Attribute::IN_ENUM)) {
                extraOptions.insert(std::make_pair("ErrorCode",
                    std::to_string(static_cast<int>(ExtractFunction::ExtractFunctionError::GLOBAL_MEMBER_VAR))));
                return false;
            }
        }

        if (sel.selectionTree.SelectedScope() == SelectionTree::Scope::FUNC_BODY
            && sel.selectionTree.TargetDecl()
            && sel.selectionTree.TargetDecl()->TestAttr(Attribute::ENUM_CONSTRUCTOR)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(ExtractFunction::ExtractFunctionError::INVALID_CODE_SEGMENT))));
            return false;
        }

        if (root->selected == SelectionTree::Selection::Complete && root->node
            && CANNOT_EXTRACT_FUNC_EXPR.count(root->node->astKind)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(ExtractFunction::ExtractFunctionError::INVALID_CODE_SEGMENT))));
            return false;
        }
        return true;
    }
};

/**
 * compute return statement whether is last statement of root:
 * 1. explicit return statement: find Return_Expr in selected range.
 * 2. implicit return statement: the last statement of method.
 * 3. cannot extract function if return statement is not the last statement.
 */
class ExtractFunctionReturnStatementRule : public TweakRule {
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        auto root = sel.selectionTree.root();

        bool isValid = true;
        Position returnExprEnd;
        SelectionTree::Walk(root, [&isValid, &extraOptions, &returnExprEnd]
            (const SelectionTree::SelectionTreeNode *treeNode) {
                if (!treeNode->node) {
                    isValid = false;
                    extraOptions.insert(std::make_pair("ErrorCode",
                        std::to_string(static_cast<int>(TweakRule::TweakError::ERROR_AST))));
                    return SelectionTree::WalkAction::STOP_NOW;
                }

                if (treeNode->selected == SelectionTree::Selection::Complete
                    &&
                    treeNode->node->astKind == ASTKind::RETURN_EXPR) {
                    returnExprEnd = treeNode->node->end;
                    return SelectionTree::WalkAction::STOP_NOW;
                }

                return SelectionTree::WalkAction::WALK_CHILDREN;
            });

        SelectionTree::Walk(root, [&isValid, &extraOptions, &returnExprEnd]
            (const SelectionTree::SelectionTreeNode *treeNode) {
                if (!treeNode->node) {
                    isValid = false;
                    extraOptions.insert(std::make_pair("ErrorCode",
                        std::to_string(static_cast<int>(TweakRule::TweakError::ERROR_AST))));
                    return SelectionTree::WalkAction::STOP_NOW;
                }

                if (treeNode->selected == SelectionTree::Selection::Complete
                    && !returnExprEnd.IsZero() && treeNode->node->end > returnExprEnd) {
                    isValid = false;
                    extraOptions.insert(std::make_pair("ErrorCode",
                        std::to_string(
                            static_cast<int>(ExtractFunction::ExtractFunctionError::MULTI_EXIT_POINT))));
                    return SelectionTree::WalkAction::STOP_NOW;
                }

                return SelectionTree::WalkAction::WALK_CHILDREN;
            });

        return isValid;
    }
};

/**
 * compute whether exist multi return value:
 * 1. find the decl definition in the selected range:
 *    the decl is a return value if the decl is used after the selected range.
 * 2. find the AssignExpr->leftValue in the selected range:
 *    the leftValue->target is a return value if the leftValue is defined outside the selected range
 *    && the leftValue->target is var_decl or func_param
 *    && the leftValue->target is used after the selected range
 *    && the leftValue->target is not member variable
 *    && the leftValue->target is not global variable
 * 3. cannot extract function if exist multi return value.
 */
class ExtractFunctionReturnValueRule : public TweakRule {
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        auto root = sel.selectionTree.root();
        bool isValid = true;
        std::unordered_set<std::string> returnValues;
        SelectionTree::Walk(root, [&isValid, &sel, &returnValues, &extraOptions]
            (const SelectionTree::SelectionTreeNode *treeNode) {
            if (!treeNode->node) {
                isValid = false;
                extraOptions.insert(std::make_pair("ErrorCode",
                    std::to_string(static_cast<int>(TweakRule::TweakError::ERROR_AST))));
                return SelectionTree::WalkAction::STOP_NOW;
            }

            // check1
            if (treeNode->selected == SelectionTree::Selection::Complete && treeNode->node->IsDecl()) {
                auto decl = dynamic_cast<Decl*>(treeNode->node.get());
                if (NeedExtractDecl2ReturnValue(decl, sel)) {
                    returnValues.insert(decl->identifier);
                }
            }

            // check2
            if (treeNode->selected == SelectionTree::Selection::Complete &&
                treeNode->node->astKind == ASTKind::ASSIGN_EXPR) {
                auto assignExpr = dynamic_cast<AssignExpr*>(treeNode->node.get());
                if (NeedExtractAssignExpr2ReturnValue(assignExpr, sel)) {
                    returnValues.insert(assignExpr->leftValue->GetTarget()->identifier);
                }
            }

            return SelectionTree::WalkAction::WALK_CHILDREN;
        });
        if (returnValues.size() > 1) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(ExtractFunction::ExtractFunctionError::MULTI_RETURN_VALUE))));
            return false;
        }

        return isValid;
    }
};

/**
 * cannot extracted function if contain partial selected branch:
 * 1. if-else: need
 * if (aa > 10) {
 *     aa++
 * } else {
 *     aa--
 * }
 * the following scenarios cannot be extracted:
 * 1-1. selection: if (aa > 10) { aa++ }
 * 1-2. selection: else { a-- }
 * 2. try-expr
 * 3. match-expr
 */
class ExtractFunctionBranchRule : public TweakRule {
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        auto root = sel.selectionTree.root();
        bool isValid = true;
        SelectionTree::Walk(root, [ &isValid, &extraOptions]
            (const SelectionTree::SelectionTreeNode *treeNode) {
                if (!treeNode->node) {
                    isValid = false;
                    extraOptions.insert(std::make_pair("ErrorCode",
                        std::to_string(static_cast<int>(TweakRule::TweakError::ERROR_AST))));
                    return SelectionTree::WalkAction::STOP_NOW;
                }

                if (treeNode->node->astKind == ASTKind::IF_EXPR) {
                    if (treeNode->selected == SelectionTree::Selection::Partial) {
                        isValid = false;
                        extraOptions.insert(std::make_pair("ErrorCode",
                            std::to_string(
                                static_cast<int>(ExtractFunction::ExtractFunctionError::PARTIAL_IF_EXPR))));
                        return SelectionTree::WalkAction::STOP_NOW;
                    }
                }

                if (treeNode->node->astKind == ASTKind::TRY_EXPR) {
                    if (treeNode->selected == SelectionTree::Selection::Partial) {
                        extraOptions.insert(std::make_pair("ErrorCode",
                            std::to_string(
                                static_cast<int>(ExtractFunction::ExtractFunctionError::PARTIAL_TRY_EXPR))));
                        return SelectionTree::WalkAction::STOP_NOW;
                    }
                }

                if (treeNode->node->astKind == ASTKind::MATCH_EXPR) {
                    if (treeNode->selected == SelectionTree::Selection::Partial) {
                        extraOptions.insert(std::make_pair("ErrorCode",
                            std::to_string(
                                static_cast<int>(ExtractFunction::ExtractFunctionError::PARTIAL_MATCH_EXPR))));
                        return SelectionTree::WalkAction::STOP_NOW;
                    }
                }

                return SelectionTree::WalkAction::WALK_CHILDREN;
            });

        return isValid;
    }
};

/**
 * need contain complete while/for... loop if continue/break is selected:
 * 1. find the JUMP_EXPR in selected range.
 * 2. get the LOOP_EXPR of the JUMP_EXPR by the JUMP_EXPR scopeName.
 * 3. cannot extract function if the JUMP_EXPR not within the selected range.
 */
class ExtractFunctionBreakContinueRule : public TweakRule {
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        auto root = sel.selectionTree.root();
        bool isValid = true;
        SelectionTree::Walk(root, [ &isValid, &extraOptions, &sel, this]
            (const SelectionTree::SelectionTreeNode *treeNode) {
                if (!treeNode->node) {
                    isValid = false;
                    extraOptions.insert(std::make_pair("ErrorCode",
                        std::to_string(static_cast<int>(TweakRule::TweakError::ERROR_AST))));
                    return SelectionTree::WalkAction::STOP_NOW;
                }

                if (treeNode->selected == SelectionTree::Selection::Complete &&
                    treeNode->node->astKind == ASTKind::JUMP_EXPR) {
                    auto jumpExpr = dynamic_cast<JumpExpr*>(treeNode->node.get());
                    if (!jumpExpr) {
                        extraOptions.insert(std::make_pair("ErrorCode",
                            std::to_string(static_cast<int>(TweakRule::TweakError::ERROR_AST))));
                        return SelectionTree::WalkAction::STOP_NOW;
                    }
                    if (!IsSatisfyJumpExpr(sel, *jumpExpr, extraOptions)) {
                        isValid = false;
                        return SelectionTree::WalkAction::STOP_NOW;
                    }
                }

                return SelectionTree::WalkAction::WALK_CHILDREN;
            });

        return isValid;
    }
};

bool ExtractFunction::Prepare(const Selection &sel)
{
    TweakRuleEngine ruleEngine;

    // add all check rules
    ruleEngine.AddRule(std::make_unique<ExtractFunctionSelectionRule>());
    ruleEngine.AddRule(std::make_unique<ExtractFunctionReturnStatementRule>());
    ruleEngine.AddRule(std::make_unique<ExtractFunctionReturnValueRule>());
    ruleEngine.AddRule(std::make_unique<ExtractFunctionBranchRule>());
    ruleEngine.AddRule(std::make_unique<ExtractFunctionBreakContinueRule>());

    ruleEngine.CheckRules(sel, extraOptions);
    return true;
}

std::optional<Tweak::Effect> ExtractFunction::Apply(const Selection &sel)
{
    Effect effect;
    if (!sel.arkAst || !sel.arkAst->sourceManager || !sel.arkAst->file) {
        return std::nullopt;
    }
    std::string filePath = sel.arkAst->file->filePath;
    std::string newFunctionName;
    auto it = sel.extraOptions.find("functionName");
    if (it != sel.extraOptions.end()) {
        newFunctionName = it->second;
    }
    if (!sel.arkAst->file) {
        return std::nullopt;
    }

    ExtractedFunction function;
    if (!newFunctionName.empty()) {
        function.name = newFunctionName;
    }
    GetExtractedFunction(sel, function);

    std::vector<TextEdit> textEdits;
    // insert new variable decl
    textEdits.push_back(InsertDeclaration(sel, function));
    // replace extracted expression with new variable
    textEdits.push_back(ReplaceBlockWithCall(sel, function));

    std::string uri = URI::URIFromAbsolutePath(filePath).ToString();
    effect.applyEdits.emplace(uri, std::move(textEdits));
    return std::move(effect);
}

void ExtractFunction::GetExtractedFunction(const Tweak::Selection &sel, ExtractedFunction &function)
{
    GetFunctionParamsList(sel, function);
    GetFunctionReturnValue(sel, function);
    GetFunctionBody(sel, function);
    GetFunctionModifier(sel, function);
}

/**
 * insert target scope:
 * 1. insert to interface-body if in_interface
 * 2. insert to class-body if in_class
 * 3. insert to struct-body if in_struct
 * 4. insert to enum-body if in_enum
 * 5. insert to extend-body if in_extend
 * 6. insert to global if in global_func
 *
 * @param sel Selection
 * @param function ExtractedFunction
 * @return
 */
TextEdit ExtractFunction::InsertDeclaration(const Tweak::Selection &sel, ExtractedFunction &function)
{
    TextEdit textEdit;
    Position insertPosition;
    Ptr<Codira::AST::Node> targetScope = sel.selectionTree.TopDecl();
    if (!targetScope && !sel.arkAst->file->decls.empty()) {
        auto lastDecl = sel.arkAst->file->decls.back().get();
        if (lastDecl) {
            insertPosition = lastDecl->end;
        }
    }
    if (targetScope && (targetScope->astKind == ASTKind::VAR_DECL || targetScope->astKind == ASTKind::FUNC_DECL)) {
        insertPosition = targetScope->end;
    }
    if (targetScope && targetScope->astKind != ASTKind::VAR_DECL && targetScope->astKind != ASTKind::FUNC_DECL) {
        Position lastCharStart = {targetScope->end.fileID, targetScope->end.line, targetScope->end.column - 1};
        std::string lastChar = sel.arkAst->sourceManager->GetContentBetween(lastCharStart, targetScope->end);
        if (lastChar == "}") {
            insertPosition = lastCharStart;
        } else {
            insertPosition = {targetScope->end.fileID, targetScope->end.line + 1, 1};
        }
    }

    if (insertPosition.IsZero()) {
        return std::move(textEdit);
    }
    if (!targetScope || targetScope->astKind == ASTKind::VAR_DECL || targetScope->astKind == ASTKind::FUNC_DECL) {
        textEdit.newText = "\n\n" + function.GetFunctionDeclaration() + "\n";
    } else {
        textEdit.newText = "\n" + function.GetFunctionDeclaration() + "\n";
    }
    Range insertRange = {insertPosition, insertPosition};
    insertRange = TransformFromChar2IDE(insertRange);
    textEdit.range = insertRange;
    return std::move(textEdit);
}

TextEdit ExtractFunction::ReplaceBlockWithCall(const Tweak::Selection &sel, ExtractedFunction &function)
{
    TextEdit textEdit;
    Range insertRange = function.replacedRange;
    if (function.replacedRange.start.IsZero()) {
        return std::move(textEdit);
    }
    insertRange = TransformFromChar2IDE(insertRange);
    textEdit.range = insertRange;
    std::ostringstream functionCall;
    functionCall << function.name << "(";
    if (!function.params.empty()) {
        auto it = function.params.begin();
        functionCall << it->name;
        for (++it; it != function.params.end(); ++it) {
            functionCall << ", " << it->name;
        }
    }
    functionCall << ")";

    if (function.returnValue.has_value()) {
        const auto& ret = function.returnValue.value();
        std::ostringstream oss;
        if (!ret.addReturnOnFuncBody) {
            oss << "return " << functionCall.str();
        } else if (ret.needDeclVar) {
            oss << (function.isConst ? "let " : "var ");
            oss << ret.name << " = " << functionCall.str();
        } else {
            oss << ret.name << " = " << functionCall.str();
        }
        textEdit.newText = oss.str();
    } else {
        textEdit.newText = functionCall.str();
    }
    return std::move(textEdit);
}

/**
 * get function param list, find ref_expr in selected range:
 * 1. need extracted to param if ref_expr->target is var_decl or func_param and outside selected range
 * 2. not need extract to param if ref_expr->target is member variable
 * 3. not need extract to param if ref_expr->target is global variable
 * 4. not need extract to param if ref_expr is assign_expr->leftValue and this ref_expr not be used before assign_expr
 *    in selected range. the ref_expr should be defined as a new variable in extracted function,ex:
 *    func test01() {
 *        var aa = 10
 *        var bb = 10
 *        aa = bb
 *    }
 *    selected range is: var bb = 10   aa = bb
 *    extracted function is:
 *    func extracted() {
 *        var aa: Int64
 *        var bb = 10
 *        aa = bb
 *    }
 *    the implementation is at AssignParam2MutVarAndRemove
 *
 * @param sel Selection
 * @param function ExtractedFunction
 */
void ExtractFunction::GetFunctionParamsList(const Tweak::Selection &sel, ExtractedFunction &function)
{
    auto root = sel.selectionTree.root();
    SelectionTree::Walk(root, [&sel, &function, this]
        (const SelectionTree::SelectionTreeNode *treeNode) {
            if (!treeNode->node) {
                return SelectionTree::WalkAction::STOP_NOW;
            }

            if (treeNode->selected == SelectionTree::Selection::Complete &&
                treeNode->node->astKind == ASTKind::REF_EXPR) {
                auto refExpr = dynamic_cast<RefExpr*>(treeNode->node.get());
                GetParam(function, refExpr, sel);
            }

            return SelectionTree::WalkAction::WALK_CHILDREN;
        });
}

void ExtractFunction::GetParam(ExtractedFunction& function, Codira::AST::RefExpr* refExpr, const Tweak::Selection &sel)
{
    if (!refExpr) {
        return;
    }
    auto target = refExpr->GetTarget();
    if (!target) {
        return;
    }
    if (target->astKind != ASTKind::VAR_DECL && target->astKind != ASTKind::FUNC_PARAM) {
        return;
    }
    if (target->begin.fileID == sel.range.start.fileID && target->begin >= sel.range.start
        && target->end <= sel.range.end) {
        return;
    }

    auto decl = dynamic_cast<VarDecl*>(target.get());
    if (!decl) {
        return;
    }
    if (decl->TestAnyAttr(Attribute::IN_CLASSLIKE, Attribute::IN_STRUCT, Attribute::IN_ENUM, Attribute::GLOBAL)) {
        return;
    }
    ExtractedFunction::Param param;
    param.name = refExpr->ref.identifier;
    if (decl->ty->HasGeneric()) {
        CollectGenerics(*decl->ty, function.generics);
    }
    param.type = GetVarDeclType(decl, sel.arkAst->sourceManager);
    function.params.emplace(std::move(param));
}

void ExtractFunction::CollectGenerics(Ty &ty, std::unordered_set<std::string> &generics)
{
    if (ty.IsGeneric()) {
        generics.insert(ty.name);
    }
    for (const auto &typeArg: ty.typeArgs) {
        if (typeArg && typeArg->HasGeneric()) {
            CollectGenerics(*typeArg, generics);
        }
    }
}

/**
 * compute the return value:
 * 1. find the decl definition in the selected range:
 *    the decl is a return value if the decl is used after the selected range.
 * 2. find the AssignExpr->leftValue in the selected range:
 *    the leftValue->target is a return value if the leftValue is defined outside the selected range
 *    && the leftValue->target is var_decl or func_param
 *    && the leftValue->target is used after the selected range
 *    && the leftValue->target is not member variable
 *    && the leftValue->target is not global variable
 *
 * @param sel Selection
 * @param function ExtractedFunction
 */
void ExtractFunction::GetFunctionReturnValue(const Tweak::Selection &sel, ExtractedFunction &function)
{
    auto root = sel.selectionTree.root();
    if (root->selected == SelectionTree::Selection::Complete && root->node->astKind == ASTKind::RETURN_EXPR) {
        ExtractedFunction::ReturnValue returnValue;
        returnValue.addReturnOnFuncBody = false;
        function.returnValue = std::move(returnValue);
        return;
    }

    const auto& children = root->Children;
    size_t childrenSize = children.size();

    for (int i = 0; i < childrenSize; ++i) {
        if (!children[i]->node) {
            return;
        }
        if (children[i]->node->astKind != ASTKind::RETURN_EXPR) {
            continue;
        }
        auto returnExpr = dynamic_cast<ReturnExpr*>(children[i]->node.get());
        if (returnExpr && children[i]->selected == SelectionTree::Selection::Complete &&
            (i + 1 >= childrenSize || children[i + 1]->selected != SelectionTree::Selection::Complete)) {
            ExtractedFunction::ReturnValue returnValue;
            returnValue.addReturnOnFuncBody = false;
            function.returnValue = std::move(returnValue);
            return;
        }
    }

    SelectionTree::Walk(root, [&sel, &function]
        (const SelectionTree::SelectionTreeNode *treeNode) {
            if (!treeNode->node) {
                return SelectionTree::WalkAction::STOP_NOW;
            }

            if (treeNode->selected == SelectionTree::Selection::Complete &&
                treeNode->node->astKind == ASTKind::VAR_DECL) {
                auto decl = dynamic_cast<VarDecl*>(treeNode->node.get());
                if (decl && NeedExtractDecl2ReturnValue(decl, sel)) {
                    ExtractedFunction::ReturnValue returnValue;
                    returnValue.addReturnOnFuncBody = true;
                    returnValue.needDeclVar = true;
                    returnValue.name = decl->identifier;
                    function.returnValue = std::move(returnValue);
                    return SelectionTree::WalkAction::STOP_NOW;
                }
            }

            if (treeNode->selected == SelectionTree::Selection::Complete &&
                treeNode->node->astKind == ASTKind::ASSIGN_EXPR) {
                auto assignExpr = dynamic_cast<AssignExpr*>(treeNode->node.get());
                if (assignExpr && NeedExtractAssignExpr2ReturnValue(assignExpr, sel)) {
                    ExtractedFunction::ReturnValue returnValue;
                    returnValue.addReturnOnFuncBody = true;
                    returnValue.name = assignExpr->leftValue->GetTarget()->identifier;
                    function.returnValue = std::move(returnValue);
                    return SelectionTree::WalkAction::STOP_NOW;
                }
            }

            return SelectionTree::WalkAction::WALK_CHILDREN;
        });
}

/**
 * extract function body:
 * 1. extract the code within block, ex:
 *    { var aa = 1 + 2; var bb = aa + 1; }
 *    extract code: var aa = 1 + 2; var bb = aa + 1;
 * 2. need assign func params to new mutable variable if param is changed in extracted function
 * 3. add return statement if have return value
 *
 * @param sel Selection
 * @param function ExtractedFunction
 */
void ExtractFunction::GetFunctionBody(const Tweak::Selection &sel, ExtractedFunction &function)
{
    auto root = sel.selectionTree.root();
    if (!root->node) {
        return;
    }
    const auto& children = root->Children;
    if (children.empty()) {
        function.replacedRange = {root->node->begin, root->node->end};
        function.body = "{\n"
                        + sel.arkAst->sourceManager->GetContentBetween(root->node->begin, root->node->end) + "\n}";
        return;
    }
    if (!sel.arkAst || !sel.arkAst->sourceManager) {
        return;
    }
    Position start;
    Position end = start;

    if (root->selected == SelectionTree::Selection::Complete && root->node->end.column > 1) {
        Position newStart = {root->node->begin.fileID, root->node->begin.line, root->node->begin.column + 1};
        Position newEnd = {root->node->end.fileID, root->node->end.line, root->node->end.column - 1};
        std::string rootStart = sel.arkAst->sourceManager->GetContentBetween(root->node->begin, newStart);
        std::string rootEnd = sel.arkAst->sourceManager->GetContentBetween(newEnd, root->node->end);
        if (rootStart == "{" && rootEnd == "}") {
            start = newStart;
            end = newEnd;
        } else {
            start = root->node->begin;
            end = root->node->end;
        }
    } else {
        bool isSelectedStart = true;
        for (const auto &child : children) {
            if (!child->node) {
                return;
            }
            if (child->selected == SelectionTree::Selection::Complete && isSelectedStart) {
                start = child->node->begin;
                isSelectedStart = false;
            }
            if (child->selected == SelectionTree::Selection::Complete && child->node->end > end) {
                end = child->node->end;
            }
        }
    }
    function.replacedRange = {start, end};
    std::string body = "{\n";
    std::string assignParma2Variable = AssignParam2MutVarAndRemove(sel, function);
    body += assignParma2Variable;
    body += sel.arkAst->sourceManager->GetContentBetween(start, end);
    auto returnValue = function.returnValue;
    if (returnValue.has_value() && returnValue.value().addReturnOnFuncBody) {
        body += "\nreturn " + returnValue.value().name;
    }
    body += "\n}";
    function.body = body;
}

void ExtractFunction::GetFunctionModifier(const Tweak::Selection &sel, ExtractedFunction &function)
{
    auto targetDecl = sel.selectionTree.TargetDecl();
    if (!targetDecl || targetDecl->astKind != ASTKind::FUNC_DECL) {
        return;
    }
    auto funcDecl = DynamicCast<Codira::AST::FuncDecl*>(targetDecl);
    if (!funcDecl) {
        return;
    }
    function.isStatic = funcDecl->TestAttr(Attribute::STATIC);
    function.isMut = funcDecl->TestAttr(Attribute::MUT);
    function.isConst = funcDecl->IsConst();
}

std::string ExtractedFunction::GetFunctionDeclaration() const
{
    std::string paramsStr;
    for (auto it = this->params.begin(); it != this->params.end(); ++it) {
        bool isLast = (std::next(it) == this->params.end());
        if (it->newName.has_value()) {
            paramsStr += it->newName.value() + ": " + it->type;
        } else {
            paramsStr += it->name + ": " + it->type;
        }
        if (!isLast) {
            paramsStr += ", ";
        }
    }
    std::string genericDeclaration;
    if (!generics.empty()) {
        genericDeclaration += "<";
        for (auto it = generics.begin(); it != generics.end(); ++it) {
            bool isLast = (std::next(it) == generics.end());
            genericDeclaration += *it;
            if (!isLast) {
                genericDeclaration += ", ";
            }
        }
        genericDeclaration += ">";
    }
    std::string modifier;
    if (this->isStatic) {
        modifier += "static ";
    }
    if (this->isConst) {
        modifier += "const ";
    }
    if (this->isMut) {
        modifier += "mut ";
    }
    return modifier + "func " + this->name + genericDeclaration + "(" + paramsStr + ") " + this->body;
}

/**
 * func param is not mutable:
 * 1. need assign func params to new mutable variable if param is changed in extracted function, ex:
 * func increment(aa: Int64): Int64 {
 *     var bb = aa
 *     bb++
 *     return bb
 * }
 * 2. remove the param if the assign_expr->leftValue not be used before assign_expr in selected range.
 *
 * @param sel Selection
 * @param function ExtractedFunction
 * @return
 */
std::string ExtractFunction::AssignParam2MutVarAndRemove(const Tweak::Selection &sel, ExtractedFunction &function)
{
    std::string mutParams;
    if (function.params.empty()) {
        return mutParams;
    }
    auto root = sel.selectionTree.root();
    std::string scopeName;
    SelectionTree::Walk(root, [&function, &mutParams, this, &sel, &scopeName]
        (const SelectionTree::SelectionTreeNode *treeNode) -> SelectionTree::WalkAction {
            if (!treeNode->node) {
                return SelectionTree::WalkAction::STOP_NOW;
            }

            if(scopeName.empty() && treeNode->selected == SelectionTree::Selection::Complete) {
                scopeName = treeNode->node->scopeName;
                size_t pos = scopeName.find_last_of('_');
                scopeName = (pos != std::string::npos) ? scopeName.substr(0, pos) : scopeName;
            }
            if (treeNode->selected == SelectionTree::Selection::Complete &&
                treeNode->node->astKind == ASTKind::ASSIGN_EXPR) {
                auto assignExpr = dynamic_cast<AssignExpr*>(treeNode->node.get());
                AddMutParamVariable(mutParams, assignExpr, function, sel, scopeName);
            }

            return SelectionTree::WalkAction::WALK_CHILDREN;
        });
    return mutParams;
}

bool ExtractFunction::ExistSameNameParam(const std::set<ExtractedFunction::Param> &params, const std::string &name)
{
    return std::find_if(params.begin(), params.end(),
               [&](const ExtractedFunction::Param& p) { return p.name == name; }) != params.end();
}

void ExtractFunction::AddMutParamVariable(std::string &mutParams, Codira::AST::AssignExpr* assignExpr,
    ExtractedFunction &function, const Tweak::Selection &sel, std::string &scopeName)
{
    if (!assignExpr || !assignExpr->leftValue || !assignExpr->leftValue->GetTarget()) {
        return;
    }
    if (assignExpr->leftValue->astKind == ASTKind::MEMBER_ACCESS) {
        return;
    }
    auto target = assignExpr->leftValue->GetTarget();
    if (target->astKind != ASTKind::VAR_DECL && target->astKind != ASTKind::FUNC_PARAM) {
        return;
    }

    bool needRemoveParam = false;
    std::string targetName = target->identifier;
    std::unordered_set<std::string> scopeNames = FindReferencesImpl::GetSelectedUesScopeNames(target, 
        *sel.arkAst, sel.range);
    if (function.returnValue.has_value() && targetName == function.returnValue->name) {
        scopeNames.insert(scopeName);
    }
    std::string assignExprSC = assignExpr->scopeName;
    auto notPrefix = std::find_if(scopeNames.begin(), scopeNames.end(),
        [&assignExprSC](const std::string &sc) {
        return sc.length() < assignExprSC.length() || sc.compare(0, assignExprSC.length(), assignExprSC) != 0;
    });
    if (notPrefix == scopeNames.end()) {
        needRemoveParam = true;
        ReferencesResult result;
        FindReferencesImpl::GetCurPkgUesage(target, *sel.arkAst, result);
        for (const auto &reference : result.References) {
            Range refRange = reference.range;
            refRange.start = PosFromIDE2Char(refRange.start);
            refRange.end = PosFromIDE2Char(refRange.end);
            if (refRange.start.fileID != sel.arkAst->fileID) {
                continue;
            }
            bool isReassign = refRange.start >= sel.range.start && refRange.end <= sel.range.end
                        && (refRange.end < assignExpr->begin || (refRange.start >= assignExpr->rightExpr->begin
                        && refRange.end <= assignExpr->rightExpr->end));
            if (isReassign) {
                needRemoveParam = false;
                break;
            }
        }
        if (COMPOUND_ASSIGN_OPERATORS.count(assignExpr->op)) {
            needRemoveParam = false;
        }
    }

    auto it = std::find_if(function.params.begin(), function.params.end(),
        [&targetName](const ExtractedFunction::Param& p) {
            return p.name == targetName;
        });
    if (it == function.params.end()) {
        return;
    }

    if (!needRemoveParam) {
        if (it->newName.has_value()) {
            return;
        }
        ExtractedFunction::Param newParam = *it;
        function.params.erase(it);
        int suffix = 1;
        std::string newName;
        do {
            newName = targetName + std::to_string(suffix);
            suffix++;
        } while (ExistSameNameParam(function.params, newName));

        newParam.newName = newName;
        function.params.insert(newParam);
        mutParams += "var " + targetName + " = " + newName + "\n";
    } else {
        mutParams += "var " + targetName + ": " + it->type + "\n";
        function.params.erase(it);
    }
}
} // namespace ark
