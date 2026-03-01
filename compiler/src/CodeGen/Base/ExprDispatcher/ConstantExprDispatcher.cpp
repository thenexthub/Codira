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

#include "CGModule.h"
#include "DIBuilder.h"
#include "IRBuilder.h"
#include "Codira/CHIR/Value.h"

namespace Codira::CodeGen {
llvm::Value* HandleConstantExpression(IRBuilder2& irBuilder, const CHIR::Constant& chirConst)
{
    return HandleLiteralValue(irBuilder, *chirConst.GetValue());
}

llvm::Value* HandleLiteralValue(IRBuilder2& irBuilder, const CHIR::LiteralValue& chirLiteral)
{
    CGModule& cgMod = irBuilder.GetCGModule();
    llvm::Value* literalValue = nullptr;
    // remove loc for constant instruction.
    auto curLoc = irBuilder.GetInsertFunction() && irBuilder.GetInsertFunction()->getSubprogram()
        ? cgMod.diBuilder->CreateDILoc(irBuilder.GetInsertFunction()->getSubprogram(), {0, 0})
        : llvm::DebugLoc().get();
    irBuilder.SetCurrentDebugLocation(curLoc);
    if (chirLiteral.IsUnitLiteral()) {
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
        literalValue = cgMod.GenerateUnitTypeValue();
#endif
    } else if (chirLiteral.IsNullLiteral()) {
        literalValue = irBuilder.CreateNullValue(*chirLiteral.GetType());
    } else if (chirLiteral.IsBoolLiteral()) {
        literalValue = irBuilder.getInt1(StaticCast<CHIR::BoolLiteral&>(chirLiteral).GetVal());
    } else if (chirLiteral.IsRuneLiteral()) {
        literalValue = irBuilder.getInt32(StaticCast<CHIR::RuneLiteral&>(chirLiteral).GetVal());
    } else if (chirLiteral.IsIntLiteral()) {
        auto type = CGType::GetOrCreate(cgMod, chirLiteral.GetType())->GetLLVMType();
        auto intConst = StaticCast<CHIR::IntLiteral*>(&chirLiteral);
        if (intConst->IsSigned()) {
            literalValue = llvm::ConstantInt::getSigned(type, intConst->GetSignedVal());
        } else {
            literalValue = llvm::ConstantInt::get(type, intConst->GetUnsignedVal());
        }
    } else if (chirLiteral.IsFloatLiteral()) {
        literalValue = llvm::ConstantFP::get(CGType::GetOrCreate(cgMod, chirLiteral.GetType())->GetLLVMType(),
            StaticCast<CHIR::FloatLiteral&>(chirLiteral).GetVal());
    } else if (chirLiteral.IsStringLiteral()) {
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
        literalValue = irBuilder.CreateStringLiteral(StaticCast<CHIR::StringLiteral&>(chirLiteral).GetVal());
#endif
    } else {
        CODEC_ABORT();
        return nullptr;
    }
    return literalValue;
}

} // namespace Codira::CodeGen
