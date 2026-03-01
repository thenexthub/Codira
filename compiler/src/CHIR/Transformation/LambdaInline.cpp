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

#include "Codira/CHIR/Transformation/LambdaInline.h"

namespace Codira::CHIR {

namespace {
std::vector<Expression*> GetNonDebugUsers(const Value& val)
{
    std::vector<Expression*> res;
    for (auto expr: val.GetUsers()) {
        if (expr->GetExprKind() != CHIR::ExprKind::DEBUGEXPR) {
            res.emplace_back(expr);
        }
    }
    return res;
}
}  // namespace

bool CheckLambdaUsingForMultiThread(const Lambda& lambda)
{
    auto returnTy = lambda.GetReturnType();
    if (returnTy == nullptr) {
        return false;
    }
    returnTy = returnTy->StripAllRefs();
    if (!returnTy->IsClass()) {
        return false;
    }
    auto classDef = StaticCast<ClassType*>(returnTy)->GetClassDef();
    return classDef->GetPackageName() == "std.core" && classDef->GetSrcCodeIdentifier() == "Future";
}

LambdaInline::LambdaInline(CHIRBuilder& builder, const GlobalOptions& opts)
    : opts(opts), inlinePass(builder, GlobalOptions::OptimizationLevel::O2, opts.chirDebugOptimizer)
{
}

void LambdaInline::InlineLambda(const std::vector<Lambda*>& funcs)
{
    if (!opts.IsOptimizationExisted(GlobalOptions::OptimizationFlag::FUNC_INLINING)) {
        return;
    }
    for (auto func : funcs) {
        if (func->GetFuncType()->IsCFunc()) {
            // skip c func type
            continue;
        }
        RunOnLambda(*func);
    }
}

/*
 * easy func can be inline:
 *   1. lambda func as a parameter.
 *   2. lambda call once in this easy function.
 *   3. lambda will not escape in this function.
 *   func g(x: LambdaType) {
 *     // ...
 *     x(<args>)
 *     // ...
 *   }
 */
bool LambdaInline::IsLambdaPassToEasyFunc(const Lambda& lambda) const
{
    // 0. lambda is not using for multi-thread
    if (CheckLambdaUsingForMultiThread(lambda)) {
        return false;
    }

    // 1. judge if lambda is a parameter of an apply.
    auto users = lambda.GetResult()->GetUsers();
    CODEC_ASSERT(users.size() == 1 && users[0]->IsApply());
    auto apply = StaticCast<Apply*>(users[0]);
    size_t index = apply->GetArgs().size();
    auto args = apply->GetArgs();
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == lambda.GetResult()) {
            index = i;
            break;
        }
    }
    if (index == args.size() || !apply->GetCallee()->IsFuncWithBody()) {
        return false;
    }
    // 2. judge lambda is only used in one callee apply.
    auto func = VirtualCast<Func*>(apply->GetCallee());
    CODEC_ASSERT(index < func->GetParams().size());
    auto lambdaArg = func->GetParams()[index];
    auto lambdaArgUsers = GetNonDebugUsers(*lambdaArg);
    if (lambdaArgUsers.size() != 1) {
        return false;
    }
    auto onlyUser = lambdaArgUsers[0];
    if (onlyUser != nullptr && onlyUser->GetExprKind() == ExprKind::APPLY &&
        StaticCast<Apply*>(onlyUser)->GetCallee() == lambdaArg) {
        return true;
    }
    return false;
}

void LambdaInline::RunOnLambda(Lambda& lambda)
{
    auto users = lambda.GetResult()->GetUsers();
    if (GetNonDebugUsers(*lambda.GetResult()).empty() && !opts.enableCompileDebug) {
        for (auto user : users) {
            user->RemoveSelfFromBlock();
        }
        lambda.RemoveSelfFromBlock();
        PrintOptInfo(lambda, opts.chirDebugOptimizer, "EraseUselessLambdas");
        return;
    }
    if (users.size() != 1U || users[0]->GetExprKind() != ExprKind::APPLY) {
        return;
    }
    if (StaticCast<Apply*>(users[0])->GetCallee() == lambda.GetResult()) {
        // If a lambda can be inlined and then removed, it should meet the requirements:
        // 1) only has one user which is an `APPLY` instruction;
        // 2) is the callee of `APPLY` instruction.
        // In other words, if a lambda is called only once and has no other uses, it can be inlined and then removed.
        inlinePass.DoFunctionInline(*StaticCast<Apply*>(users[0]), "functionInlineForLambda");
        if (!opts.enableCompileDebug) {
            lambda.RemoveSelfFromBlock();
        }
        return;
    }
    if (IsLambdaPassToEasyFunc(lambda)) {
        // only have one consumer as a parameter to apply expression, which will not escape in new function
        inlinePass.DoFunctionInline(*StaticCast<Apply*>(users[0]), "functionInlineForLambda");
        auto newUsers = lambda.GetResult()->GetUsers();
        if (newUsers[0]->GetExprKind() == ExprKind::APPLY &&
            StaticCast<Apply*>(newUsers[0])->GetCallee() == lambda.GetResult()) {
            inlinePass.DoFunctionInline(*StaticCast<Apply*>(newUsers[0]), "functionInlineForLambda");
        }

        if (!opts.enableCompileDebug) {
            lambda.RemoveSelfFromBlock();
        }
    }
}
}  // namespace Codira::CHIR
