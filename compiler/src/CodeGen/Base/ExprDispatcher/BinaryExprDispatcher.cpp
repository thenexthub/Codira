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

#include "Base/ArithmeticOpImpl.h"
#include "Base/LogicalOpImpl.h"
#include "Base/OverflowDispatcher.h"
#include "CGModule.h"
#include "IRBuilder.h"
#include "Codira/CHIR/Value.h"

using namespace Codira::CHIR;

namespace Codira::CodeGen {
llvm::Value* HandleNonOverflowBinaryExpression(IRBuilder2& irBuilder, const CHIRBinaryExprWrapper& chirExpr)
{
    switch (chirExpr.GetBinaryExprKind()) {
        case CHIR::ExprKind::ADD:
        case CHIR::ExprKind::SUB:
        case CHIR::ExprKind::MUL:
        case CHIR::ExprKind::DIV:
        case CHIR::ExprKind::MOD: {
            return GenerateArithmeticOperation(irBuilder, chirExpr);
        }
        case CHIR::ExprKind::EXP: {
            return GenerateBinaryExpOperation(irBuilder, chirExpr);
        }
        case CHIR::ExprKind::LSHIFT:
        case CHIR::ExprKind::RSHIFT:
        case CHIR::ExprKind::BITAND:
        case CHIR::ExprKind::BITOR:
        case CHIR::ExprKind::BITXOR: {
            return GenerateBitwiseOperation(irBuilder, chirExpr);
        }
        case CHIR::ExprKind::LT:
        case CHIR::ExprKind::GT:
        case CHIR::ExprKind::LE:
        case CHIR::ExprKind::GE:
        case CHIR::ExprKind::EQUAL:
        case CHIR::ExprKind::NOTEQUAL: {
            return GenerateBooleanOperation(irBuilder, chirExpr);
        }
        // AND and OR are short circuit operators. They are transformed to simple branches by CHIR
        // thus we are free from generating code for these expression kinds.
        case CHIR::ExprKind::AND:
        case CHIR::ExprKind::OR:
        default: {
            auto exprKindStr = std::to_string(static_cast<uint64_t>(chirExpr.GetBinaryExprKind()));
            CODEC_ASSERT_WITH_MSG(false, std::string("Unexpected CHIRBinaryExprKind: " + exprKindStr + "\n").c_str());
            return nullptr;
        }
    }
}

llvm::Value* HandleBinaryExpression(IRBuilder2& irBuilder, const CHIRBinaryExprWrapper& chirExpr)
{
    const CHIR::Type* ty = chirExpr.GetResult()->GetType();
    const CHIR::ExprKind& kind = chirExpr.GetBinaryExprKind();
    OverflowStrategy overflowStrategy = chirExpr.GetOverflowStrategy();
    if (!ty) {
        return nullptr;
    }
    if (OPERATOR_KIND_TO_OP_MAP.find(kind) == OPERATOR_KIND_TO_OP_MAP.end()) {
        return HandleNonOverflowBinaryExpression(irBuilder, chirExpr);
    }
    if ((overflowStrategy == OverflowStrategy::NA || overflowStrategy == OverflowStrategy::WRAPPING) &&
        kind != CHIR::ExprKind::DIV && kind != CHIR::ExprKind::MOD) {
        return HandleNonOverflowBinaryExpression(irBuilder, chirExpr);
    }
    // There is a possibility of integer overflow when the result of an arithmetic expression is an integer type.(spec)
    if (!ty->IsInteger()) {
        return HandleNonOverflowBinaryExpression(irBuilder, chirExpr);
    }
    const CHIR::IntType* intTy = StaticCast<const CHIR::IntType*>(ty);
    auto& cgMod = irBuilder.GetCGModule();
    CGValue* valLeft = cgMod | chirExpr.GetLHSOperand();
    CGValue* valRight = cgMod | chirExpr.GetRHSOperand();
    irBuilder.EmitLocation(chirExpr);
    return GenerateOverflow(irBuilder, overflowStrategy, kind, std::make_pair(intTy, nullptr), {valLeft, valRight});
}

} // namespace Codira::CodeGen
