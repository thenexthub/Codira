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

#include "Base/ExprDispatcher/ExprDispatcher.h"

#include <cinttypes>

#include "Base/CHIRExprWrapper.h"
#include "Base/OverflowDispatcher.h"
#include "IRBuilder.h"
#include "Codira/CHIR/Value.h"

namespace Codira::CodeGen {
llvm::Value* HandleNegExpression(IRBuilder2& irBuilder, llvm::Value* value)
{
    if (value->getType()->isFloatingPointTy()) {
        return irBuilder.CreateFNeg(value, "FNeg");
    } else {
        return irBuilder.CreateNeg(value, "Neg");
    }
}

llvm::Value* HandleNonOverflowUnaryExpression(IRBuilder2& irBuilder, const CHIRUnaryExprWrapper& chirExpr)
{
    auto value = **(irBuilder.GetCGModule() | chirExpr.GetOperand());
    switch (chirExpr.GetUnaryExprKind()) {
        case CHIR::ExprKind::NEG: {
            return HandleNegExpression(irBuilder, value);
        }
        case CHIR::ExprKind::NOT: {
            return irBuilder.CreateXor(value, 1, "not");
        }
        case CHIR::ExprKind::BITNOT: {
            return irBuilder.CreateNot(value, "bitNot");
        }
        default: {
            auto exprKindStr = std::to_string(static_cast<uint64_t>(chirExpr.GetUnaryExprKind()));
            CODEC_ASSERT_WITH_MSG(false, std::string("Unexpected CHIRUnaryExprKind: " + exprKindStr + "\n").c_str());
            return nullptr;
        }
    }
}

llvm::Value* HandleUnaryExpression(IRBuilder2& irBuilder, const CHIRUnaryExprWrapper& chirExpr)
{
    OverflowStrategy overflowStrategy = chirExpr.GetOverflowStrategy();
    const CHIR::ExprKind& kind = chirExpr.GetUnaryExprKind();
    if (OPERATOR_KIND_TO_OP_MAP.find(kind) == OPERATOR_KIND_TO_OP_MAP.end() ||
        overflowStrategy == OverflowStrategy::NA) {
        return HandleNonOverflowUnaryExpression(irBuilder, chirExpr);
    }

    const CHIR::Type* ty = chirExpr.GetResult()->GetType();
    // There is a possibility of integer overflow when the result of an arithmetic expression is an integer type.(spec)
    if (overflowStrategy == OverflowStrategy::WRAPPING || !ty->IsInteger()) {
        return HandleNonOverflowUnaryExpression(irBuilder, chirExpr);
    }
    const CHIR::IntType* intTy = StaticCast<const CHIR::IntType*>(ty);
    auto& cgMod = irBuilder.GetCGModule();
    auto cgValue = cgMod | chirExpr.GetOperand();
    irBuilder.EmitLocation(chirExpr);

    return GenerateOverflow(irBuilder, overflowStrategy, kind, std::make_pair(intTy, nullptr), {cgValue});
}
} // namespace Codira::CodeGen
