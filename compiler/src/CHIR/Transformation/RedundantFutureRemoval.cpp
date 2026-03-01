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

#include "Codira/CHIR/Transformation/RedundantFutureRemoval.h"

#include "Codira/CHIR/Analysis/Utils.h"
#include "Codira/CHIR/Utils.h"
#include "Codira/CHIR/Visitor/Visitor.h"

namespace Codira::CHIR {

RedundantFutureRemoval::RedundantFutureRemoval(const Package& pkg, bool isDebug)
    : package(pkg), isDebug(isDebug)
{
}

void RedundantFutureRemoval::RunOnPackage()
{
    for (auto func : package.GetGlobalFuncs()) {
        RunOnFunc(*func);
    }
}

void RedundantFutureRemoval::RunOnFunc(const Func& func)
{
    auto visitExitAction = [this](Expression& expr) {
        auto [future, apply] = CheckSpawnWithFuture(expr);
        if (future != nullptr) {
            auto spawnExpr = StaticCast<Spawn*>(&expr);
            RewriteSpawnWithOutFuture(*spawnExpr, *future, *apply);
            if (isDebug) {
                std::string message = "[RedundantFutureRemoval] The call to Spawn" +
                    ToPosInfo(expr.GetDebugLocation()) +
                    " has been optimised due to redundant future in spawn.\n";
                std::cout << message;
            }
        }
        return VisitResult::CONTINUE;
    };
    Visitor::Visit(func, visitExitAction);
}

FuncBase* RedundantFutureRemoval::GetExecureClosureFunc() const
{
    for (auto def : package.GetAllCustomTypeDef()) {
        if (!IsCoreFuture(*def)) {
            continue;
        }
        for (auto method : def->GetMethods()) {
            if (method->GetSrcCodeIdentifier() == "executeClosure") {
                return method;
            }
        }
        return nullptr;
    }
    return nullptr;
}

void RedundantFutureRemoval::RewriteSpawnWithOutFuture(Spawn& spawnExpr, LocalVar& futureValue, Apply& apply)
{
    /* change from:
        %a : future = Allocate()
        %b : funcType = Lambda()
        %c : Apply(Future, %a, %b)
        %d : Spawn(%a)
        change to:
        %b : funcType = Lambda()
        %d : Spwan(%b)
    */
    // 1. Get Lambda from apply expression
    auto lambda = apply.GetOperand(2U);
    CODEC_ASSERT(lambda->GetType()->IsFunc());

    // 2. Replace spawn and remove useless node
    auto futureExpression = futureValue.GetExpr();
    apply.RemoveSelfFromBlock();
    futureValue.ReplaceWith(*lambda, spawnExpr.GetParentBlock()->GetParentBlockGroup());
    futureExpression->RemoveSelfFromBlock();
    if (executeClosure == nullptr) {
        executeClosure = GetExecureClosureFunc();
        CODEC_NULLPTR_CHECK(executeClosure);
    }
    spawnExpr.SetExecuteClosure(*executeClosure);
}

std::pair<LocalVar*, Apply*> RedundantFutureRemoval::CheckSpawnWithFuture(Expression& expr) const
{
    if (expr.GetExprKind() != ExprKind::SPAWN) {
        return {nullptr, nullptr};
    }
    auto spawnExpr = StaticCast<Spawn*>(&expr);
    if (spawnExpr->IsExecuteClosure()) {
        return {nullptr, nullptr};
    }
    auto spawnOperand = spawnExpr->GetFuture();
    if (!spawnOperand->IsLocalVar()) {
        return {nullptr, nullptr};
    }
    auto localFuture = StaticCast<LocalVar*>(spawnOperand);
    auto users = localFuture->GetUsers();
    std::unordered_set<Expression*> usersSet(users.begin(), users.end());
    if (usersSet.size() == 3U) {
        // if spawn and future debug is only users of future, then optimize.
        // future would have exactly three users: apply future, as a paramter in spawn and debug
        usersSet.erase(localFuture->GetDebugExpr());
    }
    if (usersSet.size() == 2U) {
        // if spawn is only user of future, then optimize.
        // future would have exactly two users: apply future and use in spawn
        usersSet.erase(spawnExpr);
    }
    if (usersSet.size() != 1) {
        return {nullptr, nullptr};
    }
    // optimize spawn if only apply is left
    auto apply = *usersSet.begin();
    CODEC_ASSERT(apply->GetExprKind() == ExprKind::APPLY);
    return {localFuture, StaticCast<Apply*>(apply)};
}

} // namespace Codira::CHIR
