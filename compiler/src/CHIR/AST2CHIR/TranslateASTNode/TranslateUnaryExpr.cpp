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

Ptr<Value> Translator::Visit(const AST::UnaryExpr& unaryExpr)
{
    auto chirType = TranslateType(*unaryExpr.ty);
    ExprKind kd = ExprKind::INVALID;
    if (unaryExpr.op == Codira::TokenKind::NOT) {
        kd = unaryExpr.ty->IsBoolean() ? ExprKind::NOT : ExprKind::BITNOT;
    } else if (unaryExpr.op == Codira::TokenKind::SUB) {
        kd = ExprKind::NEG;
    } else {
        CODEC_ASSERT(false && "Visit UnaryExpr: invalid unary operation!");
    }
    auto chirExpr = TranslateExprArg(*unaryExpr.expr);
    const auto& loc = TranslateLocation(unaryExpr.begin, unaryExpr.end);

    auto ofs = unaryExpr.overflowStrategy;
    bool mayHaveException = OverloadableExprMayThrowException(unaryExpr, *chirType);
    auto opLoc = TranslateLocation(*unaryExpr.expr);
    const auto& operatorLoc = GetOperatorLoc(unaryExpr);
    return TryCreateWithOV<UnaryExpression>(
        currentBlock, mayHaveException, ofs, operatorLoc, loc, chirType, kd, chirExpr)
        ->GetResult();
}
