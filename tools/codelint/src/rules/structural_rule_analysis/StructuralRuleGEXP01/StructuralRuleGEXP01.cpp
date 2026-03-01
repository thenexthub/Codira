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

#include "StructuralRuleGEXP01.h"
#include "Codira/AST/Types.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

void StructuralRuleGEXP01::FindMatchExpr(Ptr<Codira::AST::Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        match (*node)([this](const MatchExpr &matchExpr) { CheckMatchExpr(matchExpr); });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

static bool CheckMatchExprHelper(const std::vector<AST::Pattern *> &patternVec)
{
    auto iter = std::find_if(patternVec.begin(), patternVec.end(),
        [](const auto &pattern) { return pattern->astKind == ASTKind::TYPE_PATTERN; });
    if (iter == patternVec.end()) {
        return false;
    }
    return std::find_if(patternVec.begin(), patternVec.end(), [](const auto &pattern) {
        return pattern->astKind == ASTKind::CONST_PATTERN || pattern->astKind == ASTKind::VAR_PATTERN ||
            pattern->astKind == ASTKind::VAR_OR_ENUM_PATTERN || pattern->astKind == ASTKind::ENUM_PATTERN ||
            pattern->astKind == ASTKind::TUPLE_PATTERN;
        }
    ) != patternVec.end();
}

static bool IsIrrefutablePattern(const Pattern &pattern)
{
    switch (pattern.astKind) {
        case AST::ASTKind::INVALID_PATTERN:
        case AST::ASTKind::CONST_PATTERN:
        case AST::ASTKind::TYPE_PATTERN:
            return false;
        case AST::ASTKind::WILDCARD_PATTERN:
        case AST::ASTKind::VAR_PATTERN:
            return true;
        case AST::ASTKind::TUPLE_PATTERN: {
            auto &tuplePattern = static_cast<const TuplePattern &>(pattern);
            return std::all_of(tuplePattern.patterns.cbegin(), tuplePattern.patterns.cend(),
                [](const auto &p) { return IsIrrefutablePattern(*p); });
        }
        case AST::ASTKind::VAR_OR_ENUM_PATTERN: {
            if (!pattern.ty) {
                return false;
            }
            // For variable mode, true is returned
            if (!pattern.ty->IsEnum()) {
                return true;
            }
            auto varOrEnumPattern = dynamic_cast<const AST::VarOrEnumPattern *>(&pattern);
            if (varOrEnumPattern == nullptr) {
                return false;
            }
            auto identifier = varOrEnumPattern->identifier;
            auto varOrEnumTy = static_cast<EnumTy *>(pattern.ty.get());
            if (varOrEnumTy && varOrEnumTy->declPtr) {
                auto &constructors = varOrEnumTy->declPtr->constructors;
                auto iter = std::find_if(constructors.begin(), constructors.end(),
                    [&identifier](const auto &c) { return c->identifier == identifier; });
                if (iter == constructors.end()) {
                    return true;
                }
            }
            // For Enum mode, determine if it is an IrrefutablePattern
            if (varOrEnumPattern->pattern && varOrEnumPattern->pattern->ty) {
                auto &enumPattern = static_cast<const EnumPattern &>(*varOrEnumPattern->pattern);
                auto enumTy = RawStaticCast<EnumTy *>(varOrEnumPattern->pattern->ty);
                return enumTy && enumTy->declPtr && enumTy->declPtr->constructors.size() == 1 &&
                    std::all_of(enumPattern.patterns.cbegin(), enumPattern.patterns.cend(),
                    [](const OwnedPtr<Pattern> &p) { return IsIrrefutablePattern(*p); });
            }
            return false;
        }
        case AST::ASTKind::ENUM_PATTERN: {
            if (!pattern.ty || !pattern.ty->IsEnum()) {
                return false;
            }
            auto &enumPattern = static_cast<const EnumPattern &>(pattern);
            auto enumTy = RawStaticCast<EnumTy *>(pattern.ty);
            return enumTy && enumTy->declPtr && enumTy->declPtr->constructors.size() == 1 &&
                std::all_of(enumPattern.patterns.cbegin(), enumPattern.patterns.cend(),
                [](const OwnedPtr<Pattern> &p) { return IsIrrefutablePattern(*p); });
        }
        default:
            return false;
    }
}

void StructuralRuleGEXP01::CheckMatchExpr(const Codira::AST::MatchExpr &matchExpr)
{
    std::vector<AST::Pattern *> patternVec;
    for (auto &matchCase : matchExpr.matchCases) {
        if (matchCase->ty->String() == "UnknownType") {
            continue;
        }
        for (auto &pattern : matchCase->patterns) {
            patternVec.emplace_back(pattern.get());
        }
    }
    // The case of irrefutable pattern needs to be placed after all the cases of refutable pattern
    auto firstIrr = std::find_if(patternVec.begin(), patternVec.end(),
        [](auto &pattern) { return IsIrrefutablePattern(*pattern); });
    if (firstIrr != patternVec.end()) {
        bool flag = std::all_of(firstIrr + 1, patternVec.end(),
            [](const auto &pattern) { return IsIrrefutablePattern(*pattern); });
        if (!flag) {
            Diagnose((*firstIrr)->begin, (*firstIrr)->end,
                CodeCheckDiagKind::G_EXP_01_avoid_mixed_patterns_in_match_expr_01, "");
        }
    }
    // Avoid mixing patterns with different matching dimensions within the same level of a match expression
    if (CheckMatchExprHelper(patternVec)) {
        Diagnose(matchExpr.begin, matchExpr.end, CodeCheckDiagKind::G_EXP_01_avoid_mixed_patterns_in_match_expr_02, "");
    }
}

void StructuralRuleGEXP01::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FindMatchExpr(node);
}
} // namespace Codira::CodeCheck
