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

#include "Codira/CHIR/Transformation/GetRefToArrayElem.h"

#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/CHIR/Expression/Terminator.h"

using namespace Codira::CHIR;

void GetRefToArrayElem::RunOnPackage(const Package& package, CHIRBuilder& builder)
{
    for (auto func : package.GetGlobalFuncs()) {
        RunOnFunc(*func, builder);
    }
}

void GetRefToArrayElem::RunOnFunc(const Func& func, CHIRBuilder& builder)
{
    for (auto block : func.GetBody()->GetBlocks()) {
        for (auto expr : block->GetExpressions()) {
            if (expr->GetExprKind() != ExprKind::INTRINSIC) {
                continue;
            }
            auto intrinsic = StaticCast<Intrinsic*>(expr);
            if (intrinsic->GetIntrinsicKind() != CHIR::IntrinsicKind::ARRAY_GET_UNCHECKED) {
                continue;
            }
            auto users = intrinsic->GetResult()->GetUsers();
            if (!std::all_of(users.begin(), users.end(), [](auto e) { return e->GetExprKind() == ExprKind::FIELD; })) {
                continue;
            }
            auto callContext = IntrisicCallContext {
                .kind = IntrinsicKind::ARRAY_GET_REF_UNCHECKED,
                .args = intrinsic->GetOperands()
            };
            auto arrayGetRef = builder.CreateExpression<Intrinsic>(
                builder.GetType<RefType>(intrinsic->GetResult()->GetType()), callContext, intrinsic->GetParentBlock());
            arrayGetRef->CopyAnnotationMapFrom(*intrinsic);
            for (auto user : users) {
                auto field = StaticCast<Field*>(user);
                auto fieldTy = field->GetResult()->GetType();
                auto getElemRef = builder.CreateExpression<GetElementRef>(builder.GetType<RefType>(fieldTy),
                    arrayGetRef->GetResult(), field->GetPath(), field->GetParentBlock());
                getElemRef->CopyAnnotationMapFrom(*field);
                getElemRef->GetResult()->EnableAttr(Attribute::READONLY);
                auto load = builder.CreateExpression<Load>(fieldTy, getElemRef->GetResult(), field->GetParentBlock());
                load->CopyAnnotationMapFrom(*field);
                getElemRef->MoveBefore(user);
                field->ReplaceWith(*load);
            }
            intrinsic->ReplaceWith(*arrayGetRef);
        }
    }
}
