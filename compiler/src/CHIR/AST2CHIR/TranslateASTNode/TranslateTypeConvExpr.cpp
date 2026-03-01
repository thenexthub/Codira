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

using namespace Codira::CHIR;
using namespace Codira;

Ptr<Value> Translator::Visit(const AST::TypeConvExpr& typeConvExpr)
{
    const auto& loc = TranslateLocation(typeConvExpr);
    auto chirType = TranslateType(*typeConvExpr.ty);
    auto operand = TranslateExprArg(*typeConvExpr.expr);

    auto srcTy = typeConvExpr.expr->ty;
    auto targetTy = typeConvExpr.ty.get();
    if (srcTy->IsFunc() || srcTy->IsTuple()) {
        return CreateWrappedTypeCast(loc, chirType, operand, currentBlock)->GetResult();
    }
    auto ofs = typeConvExpr.overflowStrategy;
    auto noException = targetTy->IsInteger() && ofs != OverflowStrategy::THROWING;
    auto opLoc = TranslateLocation(*typeConvExpr.expr);
    auto newNode = TryCreateCastWithOV(currentBlock, !noException, ofs, loc, chirType, operand);
    return newNode->GetResult();
}
