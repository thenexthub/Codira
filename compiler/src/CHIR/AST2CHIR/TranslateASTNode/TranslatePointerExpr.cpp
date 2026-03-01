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

#include "Codira/CHIR/AST2CHIR/TranslateASTNode/Translator.h"

#include "Codira/CHIR/AST2CHIR/Utils.h"

using namespace Codira::CHIR;
using namespace Codira;

Ptr<Value> Translator::Visit(const AST::PointerExpr& expr)
{
    auto ty = TranslateType(*expr.ty);
    CHIR::IntrinsicKind intrinsicKind;
    std::vector<Value*> args{};

    if (expr.arg) {
        intrinsicKind = IntrinsicKind::CPOINTER_INIT1;
        auto loc = TranslateLocation(*expr.arg);
        Value* argVal = nullptr;
        if (expr.arg->withInout) {
            auto argLeftValInfo = TranslateExprAsLeftValue(*expr.arg->expr);
            argVal = argLeftValInfo.base;
            // polish this
            if (!argLeftValInfo.path.empty()) {
                auto lhsCustomType = StaticCast<CustomType*>(argVal->GetType()->StripAllRefs());
                if (argVal->GetType()->IsRef()) {
                    argVal = CreateGetElementRefWithPath(TranslateLocation(expr), argVal,
                        argLeftValInfo.path, currentBlock, *lhsCustomType);
                } else {
                    auto memberType = GetInstMemberTypeByName(*lhsCustomType, argLeftValInfo.path, builder);
                    auto getMember = CreateAndAppendExpression<FieldByName>(
                        TranslateLocation(expr), memberType, argVal, argLeftValInfo.path, currentBlock);
                    argVal = getMember->GetResult();
                }
            }
            auto ty1 = TranslateType(*expr.arg->ty);
            auto callContext = IntrisicCallContext {
                .kind = IntrinsicKind::INOUT_PARAM,
                .args = std::vector<Value*>{argVal}
            };
            argVal = CreateAndAppendExpression<Intrinsic>(loc, ty1, callContext, currentBlock)->GetResult();
        } else {
            argVal = TranslateASTNode(*expr.arg, *this);
        }
        CODEC_NULLPTR_CHECK(argVal);
        argVal = GenerateLoadIfNeccessary(*argVal, false, false, expr.arg->withInout, loc);
        args.emplace_back(argVal);
    } else {
        intrinsicKind = IntrinsicKind::CPOINTER_INIT0;
    }
    const auto& loc = TranslateLocation(expr);
    auto callContext = IntrisicCallContext {
        .kind = intrinsicKind,
        .args = args
    };
    return CreateAndAppendExpression<Intrinsic>(loc, ty, callContext, currentBlock)->GetResult();
}
