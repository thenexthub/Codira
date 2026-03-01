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

/**
 * @file
 *
 * This file implements a translation from CHIR to BCHIR.
 */
#include "Codira/CHIR/Interpreter/CHIR2BCHIR.h"
#include "Codira/CHIR/Type/ClassDef.h"
#include "Codira/CHIR/Type/StructDef.h"

using namespace Codira::CHIR;
using namespace Interpreter;

void CHIR2BCHIR::TranslateMemoryExpression(Context& ctx, const Expression& expr)
{
    switch (expr.GetExprKind()) {
        case ExprKind::ALLOCATE: {
            CODEC_ASSERT(expr.GetNumOfOperands() == 0U);
            TranslateAllocate(ctx, expr);
            break;
        }
        case ExprKind::LOAD: {
            CODEC_ASSERT(expr.GetNumOfOperands() == 1U);
            PushOpCodeWithAnnotations(ctx, OpCode::DEREF, expr);
            break;
        }
        case ExprKind::STORE: {
            CODEC_ASSERT(expr.GetNumOfOperands() == Bchir::FLAG_TWO);
            PushOpCodeWithAnnotations(ctx, OpCode::ASG, expr);
            break;
        }
        case ExprKind::GET_ELEMENT_REF: {
            CODEC_ASSERT(expr.GetNumOfOperands() == 1U);
            auto getElementRefExpr = StaticCast<const GetElementRef*>(&expr);
            PushOpCodeWithAnnotations(
                ctx, OpCode::GETREF, expr, static_cast<unsigned>(getElementRefExpr->GetPath().size()));
            for (auto i : getElementRefExpr->GetPath()) {
                CODEC_ASSERT(i <= Bchir::BYTECODE_CONTENT_MAX);
                ctx.def.Push(static_cast<Bchir::ByteCodeContent>(i));
            }
            break;
        }
        case ExprKind::STORE_ELEMENT_REF: {
            CODEC_ASSERT(expr.GetNumOfOperands() == 2U);
            auto storeElementRefExpr = StaticCast<const StoreElementRef*>(&expr);
            PushOpCodeWithAnnotations(
                ctx, OpCode::STOREINREF, expr, static_cast<unsigned>(storeElementRefExpr->GetPath().size()));
            for (auto i : storeElementRefExpr->GetPath()) {
                CODEC_ASSERT(i <= Bchir::BYTECODE_CONTENT_MAX);
                ctx.def.Push(static_cast<Bchir::ByteCodeContent>(i));
            }
            break;
        }
        default: {
            // unreachable
            CODEC_ASSERT(false);
            PushOpCodeWithAnnotations(ctx, OpCode::ABORT, expr);
        }
    }
}
