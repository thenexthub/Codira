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

#include "StructuralRuleGVAR02.h"
#include <algorithm>
#include <set>

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

const size_t FIRST_LEVEL = 1;
const size_t SECOND_LEVEL = 2;

static std::string UpdateScopeName(std::string scopeName, size_t level)
{
    for (size_t i = 0; i < level; i++) {
        size_t pos = scopeName.find_last_of('0');
        if (pos != std::string::npos) {
            scopeName = scopeName.substr(0, pos);
        } else {
            break;
        }
    }
    return scopeName;
}

void StructuralRuleGVAR02::CollectVarDeclMap(Ptr<Node> node, size_t level)
{
    if (!node) {
        return;
    }

    auto preVisit = [this, &level](Ptr<Node> node) -> VisitAction {
        if (node->astKind == ASTKind::CALL_EXPR && scopeName == "") {
            scopeName = node->scopeName;
            position = node->begin;
        }
        if (node->astKind == ASTKind::REF_EXPR) {
            auto refExpr = As<ASTKind::REF_EXPR>(node);
            auto target = refExpr->ref.target;
            if (!target || target->astKind != ASTKind::VAR_DECL) {
                return VisitAction::SKIP_CHILDREN;
            }
            auto var = As<ASTKind::VAR_DECL>(target);
            if (var->TestAttr(Attribute::GLOBAL) && var->TestAttr(Attribute::PUBLIC)) {
                return VisitAction::SKIP_CHILDREN;
            }
            if (var->TestAttr(Attribute::IS_CHECK_VISITED) && !var->TestAttr(Attribute::INITIALIZATION_CHECKED)) {
                return VisitAction::SKIP_CHILDREN;
            }
            if (var->TestAnyAttr(Attribute::IN_CLASSLIKE, Attribute::IN_STRUCT, Attribute::IN_ENUM)) {
                return VisitAction::SKIP_CHILDREN;
            }
            if (inSpawnLevel > 0 && var->TestAttr(Attribute::GLOBAL) && var->isVar) {
                varDeclSet.insert(var);
                return VisitAction::SKIP_CHILDREN;
            }
            auto realscopeName = scopeName == "" ? UpdateScopeName(refExpr->scopeName, level): scopeName;
            if (varDeclMap.count(var) == 0) {
                varDeclMap[var] = {realscopeName};
            } else {
                varDeclMap[var].insert(realscopeName);
            }
        }
        if (node->astKind == ASTKind::INTERPOLATION_EXPR) {
            return VisitAction::SKIP_CHILDREN;
        }
        if (node->astKind == ASTKind::IF_EXPR) {
            auto ie = As<ASTKind::IF_EXPR>(node);
            CollectVarDeclMap(ie->condExpr, FIRST_LEVEL);
            CollectVarDeclMap(ie->thenBody);
            if (ie->hasElse) {
                CollectVarDeclMap(ie->elseBody);
            }
            return VisitAction::SKIP_CHILDREN;
        }
        if (node->astKind == ASTKind::WHILE_EXPR) {
            auto we = As<ASTKind::WHILE_EXPR>(node);
            CollectVarDeclMap(we->condExpr, FIRST_LEVEL);
            CollectVarDeclMap(we->body);
            return VisitAction::SKIP_CHILDREN;
        }
        if (node->astKind == ASTKind::DO_WHILE_EXPR) {
            auto we = As<ASTKind::DO_WHILE_EXPR>(node);
            CollectVarDeclMap(we->condExpr, FIRST_LEVEL);
            CollectVarDeclMap(we->body);
            return VisitAction::SKIP_CHILDREN;
        }
        if (node->astKind == ASTKind::VAR_DECL) {
            auto var = As<ASTKind::VAR_DECL>(node);
            if (var->identifier.Val() == ".mtx" && var->ty && var->ty->name == "ReentrantMutex") {
                CollectVarDeclMap(var->initializer, SECOND_LEVEL);
                return VisitAction::SKIP_CHILDREN;
            }
        }
        if (node->astKind == ASTKind::SPAWN_EXPR) {
            inSpawnLevel++;
            auto spawnExpr = As<ASTKind::SPAWN_EXPR>(node);
            CollectVarDeclMap(spawnExpr->futureObj, SECOND_LEVEL);
            CollectVarDeclMap(spawnExpr->arg, SECOND_LEVEL);
            CollectVarDeclMap(spawnExpr->task, SECOND_LEVEL);
            return VisitAction::SKIP_CHILDREN;
        }
        return VisitAction::WALK_CHILDREN;
    };

    auto postVisit = [this](Ptr<Node> node) -> VisitAction {
        if (node->astKind == ASTKind::SPAWN_EXPR) {
            inSpawnLevel--;
        }
        if (node->astKind == ASTKind::CALL_EXPR && node->begin == position) {
            scopeName = "";
            position = {0, 0, 0};
        }
        return VisitAction::SKIP_CHILDREN;
    };

    Walker walker(node, preVisit, postVisit);
    walker.Walk();
}

static std::string FindMinParentScope(std::set<std::string>& strings)
{
    if (strings.empty()) {
        return "";
    }

    auto it = strings.begin();
    std::string commonPrefix = *it;
    ++it;

    while (it != strings.end()) {
        const std::string& currentString = *it;
        size_t minLength = std::min(commonPrefix.size(), currentString.size());
        size_t i = 0;
        for (; i < minLength; ++i) {
            if (commonPrefix[i] != currentString[i]) {
                break;
            }
        }
        commonPrefix = commonPrefix.substr(0, i);
        if (commonPrefix.empty()) {
            return "";
        }
        ++it;
    }
    if (!commonPrefix.empty() && commonPrefix.back() == '0') {
        return commonPrefix.substr(0, commonPrefix.size() - 1);
    }
    return commonPrefix;
}

void StructuralRuleGVAR02::CheckVarDeclMap()
{
    for (auto& element : varDeclMap) {
        auto varDecl = element.first;
        if (varDeclSet.count(varDecl) != 0) {
            continue;
        }
        auto declScope = varDecl->scopeName;
        auto refScopeSet = element.second;
        auto item =
            std::find_if(refScopeSet.begin(), refScopeSet.end(), [&declScope](auto& it) { return it == declScope; });
        if (item != refScopeSet.end()) {
            continue;
        }
        auto minParentScope = FindMinParentScope(refScopeSet);
        if (declScope.size() >= minParentScope.size()) {
            continue;
        }
        Diagnose(varDecl->begin, varDecl->end, CodeCheckDiagKind::G_VAR_02_min_scope, varDecl->identifier.Val());
    }
}

void StructuralRuleGVAR02::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    CollectVarDeclMap(node);
    CheckVarDeclMap();
}
}
