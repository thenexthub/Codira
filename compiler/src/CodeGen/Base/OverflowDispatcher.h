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
 * This file declares generate overflow APIs for codegen.
 */

#ifndef CODIRA_OVDERFLOW_DISPATCHER_H
#define CODIRA_OVDERFLOW_DISPATCHER_H

#include "llvm/IR/Value.h"

#include "Base/CHIRExprWrapper.h"
#include "CGModule.h"
#include "IRBuilder.h"

namespace Codira {
namespace CHIR {
class Type;
enum class ExprKind : uint8_t;
} // namespace CHIR
namespace CodeGen {
class IRBuilder2;

llvm::Value* GenerateOverflowBinaryExpression(IRBuilder2& irBuilder, const CHIR::Expression& chirExpr);
llvm::Value* GenerateOverflowWrappingArithmeticOp(IRBuilder2& irBuilder, const CHIR::ExprKind& kind,
    const CHIR::Type* ty, const std::vector<CGValue*>& argGenValues);
llvm::Value* GenerateOverflow(IRBuilder2& irBuilder, const OverflowStrategy& strategy, const CHIR::ExprKind& kind,
    const std::pair<const CHIR::IntType*, const CHIR::Type*>& tys, const std::vector<CGValue*>& argGenValues);

llvm::Value* GenerateOverflowApply(IRBuilder2& irBuilder, const CHIRIntrinsicWrapper& intrinsic);
} // namespace CodeGen
} // namespace Codira
#endif // CODIRA_OVDERFLOW_DISPATCHER_H
