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

#include "Base/ArrayImpl.h"

#include "Base/CHIRExprWrapper.h"
#include "IRBuilder.h"

using namespace Codira;
using namespace CodeGen;

llvm::Value* CodeGen::GenerateRawArrayInitByValue(
    IRBuilder2& irBuilder, const CHIR::RawArrayInitByValue& rawArrayInitByValue)
{
    auto& cgMod = irBuilder.GetCGModule();

    auto valueOperand = rawArrayInitByValue.GetInitValue();
    auto elemValue = *(cgMod | valueOperand);
    auto sizeVal = **(cgMod | rawArrayInitByValue.GetSize());
    auto arrTy = static_cast<CHIR::RawArrayType*>(rawArrayInitByValue.GetRawArray()->GetType()->GetTypeArgs()[0]);

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    auto array = **(cgMod | rawArrayInitByValue.GetRawArray());
    bool isNullValue = valueOperand->IsLocalVar()
        ? StaticCast<CHIR::LocalVar*>(valueOperand)->GetExpr()->IsConstantNull()
        : false;
    if (isNullValue) {
        return array;
    }

    irBuilder.CallArrayInit(array, sizeVal, elemValue.GetRawValue(), *arrTy);
    return array;
#endif
}

llvm::Value* CodeGen::GenerateRawArrayAllocate(IRBuilder2& irBuilder, const CHIRRawArrayAllocateWrapper& rawArray)
{
    // Sized array must have 2 arguments
    CODEC_ASSERT_WITH_MSG(rawArray.GetOperands().size() == 1, "RawArrayAllocate's argument size is not equal to 1.");
    auto& cgMod = irBuilder.GetCGModule();
    auto arrTy = StaticCast<CHIR::RawArrayType*>(rawArray.GetResult()->GetType()->GetTypeArgs()[0]);
    auto length = **(cgMod | rawArray.GetOperand(0));
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    // If we already know the length of rawArray is greater than or equal to 0, we can remove the throw branch.
    if (rawArray.GetOperand(0)->IsLocalVar() &&
        StaticCast<CHIR::LocalVar*>(rawArray.GetOperand(0))->GetExpr()->IsConstant()) {
        auto constExpr = StaticCast<CHIR::Constant*>(StaticCast<CHIR::LocalVar*>(rawArray.GetOperand(0))->GetExpr());
        if (constExpr->GetSignedIntLitVal() >= 0) {
            return irBuilder.AllocateArray(*arrTy, length);
        }
    }
    auto [throwBB, bodyBB] = Vec2Tuple<2>(
        irBuilder.CreateAndInsertBasicBlocks({GenNameForBB("arr.alloc.throw"), GenNameForBB("arr.alloc.body")}));

    auto zeroVal = llvm::ConstantInt::get(length->getType(), 0);
    // Check whether size is greater than zero.
    auto cmpValid = irBuilder.CreateIntrinsic(llvm::Intrinsic::expect,
        {irBuilder.getInt1Ty()},
        {irBuilder.CreateICmpSGE(length, zeroVal, "arr.alloc.size.valid"), irBuilder.getTrue()});
    (void)irBuilder.CreateCondBr(cmpValid, bodyBB, throwBB);
    irBuilder.SetInsertPoint(throwBB);
    irBuilder.CreateNegativeArraySizeException();
    irBuilder.CreateUnreachable();
    irBuilder.SetInsertPoint(bodyBB);
    return irBuilder.AllocateArray(*arrTy, length);
#endif
}
