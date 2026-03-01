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

#include "Codira/CHIR/Transformation/UnitUnify.h"
#include "Codira/CHIR/Analysis/Utils.h"
#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/CHIR/Visitor/Visitor.h"

using namespace Codira::CHIR;
namespace {
bool NeedUnify(const Expression& expr)
{
    auto result = expr.GetResult();
    if (result == nullptr) {
        return false;
    }
    if (!result->GetType()->IsUnit()) {
        return false;
    }
    if (result->GetUsers().empty()) {
        return false;
    }
    if (auto constant = Codira::DynamicCast<const Constant*>(&expr)) {
        if (constant->GetValue()->IsNullLiteral() || constant->GetValue()->IsUnitLiteral()) {
            return false;
        }
    }
    return true;
}
}

UnitUnify::UnitUnify(CHIRBuilder& builder) : builder(builder)
{
}

void UnitUnify::RunOnPackage(const Ptr<const Package>& package, bool isDebug)
{
    for (auto func : package->GetGlobalFuncs()) {
        RunOnFunc(func, isDebug);
    }
}

void UnitUnify::RunOnFunc(const Ptr<Func>& func, bool isDebug)
{
    Ptr<Constant> optUnit;
    auto preAcation = [this, isDebug, &optUnit](Expression& expr) {
        if (Is<GetRTTI>(expr) || Is<GetRTTIStatic>(expr)) {
            return VisitResult::CONTINUE;
        }
        if (NeedUnify(expr)) {
            LoadOrCreateUnit(optUnit, expr.GetParentBlockGroup());
            expr.GetResult()->ReplaceWith(*optUnit->GetResult(), expr.GetParentBlockGroup());
            if (isDebug) {
                std::cout << "[UnitUnify] unit unify" << ToPosInfo(expr.GetDebugLocation()) << ".\n";
            }
        }
        return VisitResult::CONTINUE;
    };
    Visitor::Visit(*func, preAcation);
}

void UnitUnify::LoadOrCreateUnit(Ptr<Constant>& constant, const Ptr<BlockGroup>& group)
{
    if (constant != nullptr) {
        return;
    }
    auto entryBlock = group->GetEntryBlock();
    constant = builder.CreateConstantExpression<UnitLiteral>(builder.GetUnitTy(), entryBlock);
    constant->MoveBefore(entryBlock->GetExpressions()[0]);
}
