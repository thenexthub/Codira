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

#include "Codira/CHIR/Transformation/UselessAllocateElimination.h"

#include "Codira/CHIR/Analysis/Utils.h"
#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/CHIR/Expression/Terminator.h"

using namespace Codira::CHIR;

void UselessAllocateElimination::RunOnPackage(const Package& package, bool isDebug)
{
    for (auto func : package.GetGlobalFuncs()) {
        RunOnFunc(*func, isDebug);
    }
}

void UselessAllocateElimination::RunOnFunc(const Func& func, bool isDebug)
{
    for (auto block : func.GetBody()->GetBlocks()) {
        for (auto expr : block->GetExpressions()) {
            if (expr->GetExprKind() != ExprKind::ALLOCATE) {
                continue;
            }
            auto allocate = StaticCast<Allocate*>(expr);
            if (auto allocatedTy = allocate->GetType();
                allocatedTy->IsClass() && StaticCast<ClassType*>(allocatedTy)->GetClassDef()->GetFinalizer()) {
                continue;
            }
            auto res = allocate->GetResult();
            if (func.GetReturnValue() == res) {
                continue;
            }
            auto users = res->GetUsers();
            auto onlyBeenWritten = std::all_of(users.begin(), users.end(), [res](auto e) {
                return (e->GetExprKind() == ExprKind::STORE && StaticCast<Store*>(e)->GetLocation() == res) ||
                    (e->GetExprKind() == ExprKind::STORE_ELEMENT_REF &&
                        StaticCast<StoreElementRef*>(e)->GetLocation() == res) ||
                    e->GetExprKind() == ExprKind::DEBUGEXPR;
            });
            if (onlyBeenWritten) {
                allocate->RemoveSelfFromBlock();
                for (auto user : users) {
                    user->RemoveSelfFromBlock();
                }
                if (isDebug && !allocate->GetDebugLocation().GetBeginPos().IsZero()) {
                    std::string message = "[UselessAllocateElimination] Allocate" +
                        ToPosInfo(allocate->GetDebugLocation()) + " and its users have been deleted\n";
                    std::cout << message;
                }
            }
        }
    }
}
