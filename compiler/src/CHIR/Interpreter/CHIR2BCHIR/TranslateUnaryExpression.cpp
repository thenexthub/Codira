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
#include "Codira/CHIR/Interpreter/Utils.h"

using namespace Codira::CHIR;
using namespace Interpreter;

void CHIR2BCHIR::TranslateUnaryExpression(Context& ctx, const Expression& expr)
{
    CODEC_ASSERT(expr.GetNumOfOperands() == Bchir::FLAG_ONE);
    auto opCode = Codira::CHIR::Interpreter::UnExprKind2OpCode(expr.GetExprKind());
    auto unaryExpression = StaticCast<const UnaryExpression*>(&expr);
    auto typeKind = expr.GetResult()->GetType()->GetTypeKind();
    auto overflow = static_cast<Bchir::ByteCodeContent>(unaryExpression->GetOverflowStrategy());
    PushOpCodeWithAnnotations<false, true>(ctx, opCode, expr, typeKind, overflow);
}
