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

#ifndef CODIRA_ARITHMETICOP_IMPL_H
#define CODIRA_ARITHMETICOP_IMPL_H

#include "llvm/IR/Value.h"

#include "Codira/CHIR/Expression/Terminator.h"

namespace Codira {
namespace CodeGen {
class IRBuilder2;
class CGValue;
class CHIRBinaryExprWrapper;
llvm::Value* GenerateArithmeticOperation(IRBuilder2& irBuilder, const CHIRBinaryExprWrapper& binExpr);
llvm::Value* GenerateArithmeticOperation(IRBuilder2& irBuilder, CHIR::ExprKind exprKind, const CHIR::Type* ty,
    const CGValue* valLeft, const CGValue* valRight);
llvm::Value* GenerateBitwiseOperation(IRBuilder2& irBuilder, const CHIRBinaryExprWrapper& binExpr);
llvm::Value* GenerateBinaryExpOperation(IRBuilder2& irBuilder, const CHIRBinaryExprWrapper& binExpr);
llvm::Value* GenerateBinaryExpOperation(IRBuilder2& irBuilder, CGValue* valLeft, CGValue* valRight);
} // namespace CodeGen
} // namespace Codira
#endif // CODIRA_ARITHMETICOP_IMPL_H
