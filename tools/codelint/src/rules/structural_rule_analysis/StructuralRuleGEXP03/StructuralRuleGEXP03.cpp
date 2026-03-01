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

#include "StructuralRuleGEXP03.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

/*
 * The function that find the &&, ||, ?, ?? node and its right operand, then make a side effect check.
 */

void StructuralRuleGEXP03::RightOperandFinder(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        match (*node)(
            // find &&, ||, ?? node and check its right operand.
            [this](const BinaryExpr& binaryExpr) {
                if ((binaryExpr.op == TokenKind::AND || binaryExpr.op == TokenKind::OR ||
                        binaryExpr.op == TokenKind::COALESCING) &&
                    binaryExpr.rightExpr != nullptr) {
                    SideEffectChecker(binaryExpr.rightExpr.get());
                }
            });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGEXP03::FindMemberAccess(const MemberAccess& memberAccess)
{
    std::string name;
    if (memberAccess.baseExpr != nullptr && memberAccess.baseExpr->ty != nullptr) {
        name = memberAccess.baseExpr->ty->name + "." + memberAccess.field;
    }
    Position start, end;
    if (memberAccess.begin.IsZero() && memberAccess.callOrPattern != nullptr) {
        start = memberAccess.callOrPattern->begin;
        end = memberAccess.callOrPattern->end;
    } else {
        start = memberAccess.begin;
        end = memberAccess.end;
    }
    // If the accessed symbol is not a function name
    if (memberAccess.target != nullptr) {
        auto it = sideEffectSet.find(memberAccess.target->identifier.Begin());
        if (it != sideEffectSet.end() && it->fileID == memberAccess.target->identifier.Begin().fileID) {
            Diagnose(start, end, CodeCheckDiagKind::G_EXP_03_side_effect_information, name);
        }
    } else if (!memberAccess.targets.empty()) { // if the accessed symbol is a function name
        for (auto& item : memberAccess.targets) {
            auto it = sideEffectSet.find(item->identifier.Begin());
            if (it != sideEffectSet.end() && it->fileID == item->identifier.Begin().fileID) {
                Diagnose(start, end, CodeCheckDiagKind::G_EXP_03_side_effect_information, name);
            }
        }
    }
}

std::string StructuralRuleGEXP03::GetOperandName(const AssignExpr& assignExpr)
{
    auto& desugarExpr = assignExpr.desugarExpr;
    std::string name;
    /**
     * For opertor overloading cases, the leftValue of assignExpr may be empty
     * But we can obtain it via desugarExpr. Here is a example
     * '''
     * extend Int64 {
     *   public operator func + (rhs: Int64): Int64 {
     *      return this + Int64(rhs)
     *   }
     * }
     * var a = 1
     * var b = Option<Int64>.None
     * b ?? a++
     * '''
     */
    if (!assignExpr.leftValue && desugarExpr && desugarExpr->astKind == ASTKind::ASSIGN_EXPR) {
        auto assignDesugar = StaticCast<AssignExpr*>(desugarExpr.get().get());
        name = this->diagEngine->GetSourceManager().GetContentBetween(assignDesugar->leftValue->GetBegin().fileID,
            assignDesugar->leftValue->GetBegin(), assignDesugar->leftValue->GetEnd());
    } else {
        name = this->diagEngine->GetSourceManager().GetContentBetween(
            assignExpr.leftValue->GetBegin().fileID, assignExpr.leftValue->GetBegin(), assignExpr.leftValue->GetEnd());
    }
    // Remove trailing whitespace.
    name.erase(name.find_last_not_of(" \t\n\r\f\v") + 1);
    return name;
}

/*
 * The function that check side effect and throw error.
 */
void StructuralRuleGEXP03::SideEffectChecker(Ptr<Node> node)
{
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            // find AssignExpr which can make side effect, throw error. Exp. count += num
            [this](const AssignExpr& assignExpr) {
                std::string name = GetOperandName(assignExpr);
                if (assignExpr.desugarExpr == nullptr || assignExpr.desugarExpr->astKind != ASTKind::CALL_EXPR) {
                    Diagnose(
                        assignExpr.begin, assignExpr.end, CodeCheckDiagKind::G_EXP_03_side_effect_information, name);
                    return VisitAction::SKIP_CHILDREN;
                }

                auto callExpr = StaticAs<ASTKind::CALL_EXPR>(assignExpr.desugarExpr.get());
                if (sideEffectSet.find(callExpr->resolvedFunction->identifier.Begin()) != sideEffectSet.end()) {
                    Diagnose(
                        assignExpr.begin, assignExpr.end, CodeCheckDiagKind::G_EXP_03_side_effect_information, name);
                }
                return VisitAction::SKIP_CHILDREN;
            },
            // find IncOrDecExpr which can make side effect, throw error. Exp. count++
            [this](const IncOrDecExpr& incOrDecExpr) {
                if (incOrDecExpr.desugarExpr != nullptr) {
                    return VisitAction::WALK_CHILDREN;
                }
                Diagnose(incOrDecExpr.begin, incOrDecExpr.end, CodeCheckDiagKind::G_EXP_03_side_effect_information,
                    incOrDecExpr.expr->ToString());
                return VisitAction::SKIP_CHILDREN;
            },
            // find RefExpr, if it is func and its name in sideEffectSet,throw a error.
            [this](const RefExpr& refExpr) {
                if (refExpr.isBaseFunc && refExpr.ref.target != nullptr) {
                    if (sideEffectSet.find(refExpr.ref.target->identifier.Begin()) != sideEffectSet.end()) {
                        Diagnose(refExpr.begin, refExpr.end,
                            CodeCheckDiagKind::G_EXP_03_side_effect_information, refExpr.ref.identifier.Val());
                    }
                }
                return VisitAction::WALK_CHILDREN;
            },
            // find memberAccess, if its name in sideEffectSet,throw a error.
            [this](const MemberAccess& memberAccess) {
                FindMemberAccess(memberAccess);
                return VisitAction::WALK_CHILDREN;
            },
            [this](const BinaryExpr& binaryExpr) {
                if (binaryExpr.desugarExpr != nullptr) {
                    SideEffectChecker(binaryExpr.desugarExpr.get());
                    return VisitAction::SKIP_CHILDREN;
                }
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

/*
 * The function can find the functions that include side effect and add its name to a side effect set.
 */
void StructuralRuleGEXP03::SideEffectFuncFinder(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        match (*node)([this](const FuncDecl& funcDecl) {
            if (!funcDecl.TestAttr(AST::Attribute::IN_CLASSLIKE) && !funcDecl.TestAttr(AST::Attribute::IN_STRUCT)) {
                GlobalRefChecker(
                    funcDecl.funcBody->body.get(), funcDecl.identifier.Begin(), funcDecl.identifier.Begin());
            } else {
                ClassRefChecker(
                    funcDecl.funcBody->body.get(), funcDecl.identifier.Begin(), funcDecl.identifier.Begin());
            }
        });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

/*
 * The function that help SideEffectFuncFinder func to find side effect expression in the func.
 */
void StructuralRuleGEXP03::SideEffectExprFinder(Ptr<Codira::AST::Node> node, const Codira::Position position)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this, position](Ptr<Node> node) -> VisitAction {
        match (*node)(
            [this, position](const RefExpr& refExpr) {
                if (refExpr.isBaseFunc) {
                    (void)sideEffectSet.insert(position);
                }
            },
            [this, position](const AssignExpr&) { (void)sideEffectSet.insert(position); },
            [this, position](const IncOrDecExpr&) { (void)sideEffectSet.insert(position); });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

bool StructuralRuleGEXP03::AddRefFuncInfo(Ptr<const Codira::AST::Node> node,
    const Codira::Position originFuncPosition, const Codira::Position refFuncPosition)
{
    if (node == nullptr) {
        return false;
    }
    // the beginning node
    if (std::find(callStack.begin(), callStack.end(), originFuncPosition) != callStack.end() &&
        std::find(callStack.begin(), callStack.end(), refFuncPosition) != callStack.end()) {
        return false;
    }
    // the stack include position that already checked and this stack could avoid function with infinite loop.
    callStack.push_back(refFuncPosition);
    return true;
}

void StructuralRuleGEXP03::ProcessRefFuncInfo(const RefExpr& refExpr, const Codira::Position originFuncPosition)
{
    for (auto& item : refExpr.ref.targets) {
        if (auto func = DynamicCast<FuncDecl*>(item.get()); func) {
            if (sideEffectSet.find(func->identifier.Begin()) != sideEffectSet.end()) {
                (void)sideEffectSet.insert(originFuncPosition);
            } else if (checkedFunc.find(func->identifier.Begin()) != checkedFunc.end()) {
                continue;
            } else {
                (void)checkedFunc.insert(func->identifier.Begin());
                GlobalRefChecker(func->funcBody->body.get(), originFuncPosition, func->identifier.Begin());
            }
        }
    }
}

/*
 * The function that help SideEffectFuncFinder func to find GlobalRef expression in the func.
 */
void StructuralRuleGEXP03::GlobalRefChecker(
    Ptr<Codira::AST::Node> node, const Codira::Position originFuncPosition, const Codira::Position refFuncPosition)
{
    if (node == nullptr || originFuncPosition == INVALID_POSITION ||
        !AddRefFuncInfo(node, originFuncPosition, refFuncPosition)) {
        return;
    }
    Ptr<Node> oNode = node;
    Walker walker(node, [this, originFuncPosition, oNode](Ptr<Node> node) -> VisitAction {
        match (*node)([this, originFuncPosition, oNode](const RefExpr& refExpr) {
            if (refExpr.ref.target) {
                // check ref of varDecl
                if (refExpr.ref.target->TestAttr(AST::Attribute::GLOBAL) && !refExpr.isBaseFunc) {
                    SideEffectExprFinder(oNode, originFuncPosition);
                } else if (!refExpr.ref.targets.empty()) { // check ref of functions
                    ProcessRefFuncInfo(refExpr, originFuncPosition);
                }
            }
        });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
    callStack.pop_back();
}

void StructuralRuleGEXP03::ClassRefCheckerDetail(const RefExpr& refExpr, const Codira::Position originFuncPosition)
{
    for (auto& item : refExpr.ref.targets) {
        if (auto func = DynamicCast<FuncDecl*>(item.get()); func) {
            if (sideEffectSet.find(func->identifier.Begin()) != sideEffectSet.end()) {
                (void)sideEffectSet.insert(originFuncPosition);
            } else if (checkedFunc.find(func->identifier.Begin()) != checkedFunc.end()) {
                continue;
            } else {
                (void)checkedFunc.insert(func->identifier.Begin());
                ClassRefChecker(func->funcBody->body.get(), originFuncPosition, func->identifier.Begin());
            }
        }
    }
}

/*
 * The function that help SideEffectFuncFinder func to find Global RefExpr and Class-level RefExpr in the func.
 */
void StructuralRuleGEXP03::ClassRefChecker(
    Ptr<Codira::AST::Node> node, const Codira::Position originFuncPosition, const Codira::Position refFuncPosition)
{
    if (node == nullptr || originFuncPosition == INVALID_POSITION ||
        !AddRefFuncInfo(node, originFuncPosition, refFuncPosition)) {
        return;
    }
    Ptr<Node> oNode = node;
    Walker walker(node, [this, originFuncPosition, oNode](Ptr<Node> node) -> VisitAction {
        match (*node)([this, originFuncPosition, oNode](const RefExpr& refExpr) {
            if (refExpr.ref.target) {
                // the condition that can select refExpr of var which is global, in the class and in the struct.
                if ((refExpr.ref.target->TestAttr(AST::Attribute::GLOBAL) ||
                        refExpr.ref.target->TestAttr(AST::Attribute::IN_CLASSLIKE) ||
                        refExpr.ref.target->TestAttr(AST::Attribute::IN_STRUCT)) &&
                    !refExpr.isBaseFunc) {
                    SideEffectExprFinder(oNode, originFuncPosition);
                } else if (!refExpr.ref.targets.empty()) {
                    ClassRefCheckerDetail(refExpr, originFuncPosition);
                }
            }
        });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
    callStack.pop_back();
}

void StructuralRuleGEXP03::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;

    // The node that could let Walker search file from beginning.
    originNode = node;
    SideEffectFuncFinder(node);
    RightOperandFinder(node);
}
} // namespace Codira::CodeCheck
