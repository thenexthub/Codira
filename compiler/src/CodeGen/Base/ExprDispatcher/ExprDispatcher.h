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

#ifndef CODIRA_EXPRDISPATCHER_H
#define CODIRA_EXPRDISPATCHER_H

#include "llvm/IR/Value.h"

#include "Codira/CHIR/Expression/Terminator.h"

namespace Codira {
namespace CodeGen {
class IRBuilder2;
class CGValue;
class CHIRUnaryExprWrapper;
class CHIRBinaryExprWrapper;

llvm::Value* HandleConstantExpression(IRBuilder2& irBuilder, const CHIR::Constant& chirConst);
llvm::Value* HandleLiteralValue(IRBuilder2& irBuilder, const CHIR::LiteralValue& chirLiteral);
llvm::Value* HandleTerminatorExpression(IRBuilder2& irBuilder, const CHIR::Expression& chirExpr);
llvm::Value* HandleNegExpression(IRBuilder2& irBuilder, llvm::Value* value);
llvm::Value* HandleUnaryExpression(IRBuilder2& irBuilder, const CHIRUnaryExprWrapper& chirExpr);
llvm::Value* HandleBinaryExpression(IRBuilder2& irBuilder, const CHIRBinaryExprWrapper& chirExpr);
llvm::Value* HandleMemoryExpression(IRBuilder2& irBuilder, const CHIR::Expression& chirExpr);
llvm::Value* HandleStructedControlFlowExpression(IRBuilder2& irBuilder, const CHIR::Expression& chirExpr);
llvm::Value* HandleOthersExpression(IRBuilder2& irBuilder, const CHIR::Expression& chirExpr);
} // namespace CodeGen
} // namespace Codira
#endif // CODIRA_EXPRDISPATCHER_H
