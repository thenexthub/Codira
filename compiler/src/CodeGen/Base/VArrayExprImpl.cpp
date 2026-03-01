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
 * This file implements codegen for CHIR Varray creation.
 */

#include "Base/VArrayExprImpl.h"

#include <optional>

#include "CGModule.h"
#include "IRBuilder.h"
#include "Utils/CGUtils.h"
#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/Type/Type.h"
#include "Codira/CHIR/Value.h"

using namespace Codira;
using namespace CodeGen;

namespace {
llvm::Value* GenerateConstantVArray(
    const IRBuilder2& irBuilder, const CHIR::VArray& varray, const std::string& serialized)
{
    auto chirType = varray.GetResult()->GetType();
    CODEC_ASSERT_WITH_MSG(chirType->IsVArray(), "Should not reach here.");
    auto varrayChirType = StaticCast<const CHIR::VArrayType*>(chirType);

    auto varrayCGType = CGType::GetOrCreate(irBuilder.GetCGModule(), varrayChirType);
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    std::vector<llvm::Constant*> params;
    for (size_t i = 0; i < varray.GetOperands().size(); ++i) {
        auto value = (irBuilder.GetCGModule() | varray.GetOperand(i))->GetRawValue();
        auto tmp = llvm::dyn_cast<llvm::GlobalVariable>(value);
        params.emplace_back(tmp ? tmp->getInitializer() : llvm::cast<llvm::Constant>(value));
    }
    auto arrayType = llvm::cast<llvm::ArrayType>(varrayCGType->GetLLVMType());
    auto constVal = llvm::ConstantArray::get(arrayType, params);
    return irBuilder.GetCGModule().GetOrCreateGlobalVariable(constVal, serialized, false);
#endif
}
} // namespace

llvm::Value* CodeGen::GenerateVArray(IRBuilder2& irBuilder, const CHIR::VArray& varray)
{
    // let arr1: VArray<Int64, $5> = [1,2,3,4,5]
    auto [isConstantVArray, serialized] = IsConstantVArray(varray);
    if (isConstantVArray) {
        return GenerateConstantVArray(irBuilder, varray, serialized);
    }
    auto chirType = varray.GetResult()->GetType();
    CODEC_ASSERT_WITH_MSG(chirType->IsVArray(), "Should not reach here.");
    auto varrayChirType = StaticCast<const CHIR::VArrayType*>(chirType);

    auto varrayCGType = CGType::GetOrCreate(irBuilder.GetCGModule(), varrayChirType);
    auto varrayType = varrayCGType->GetLLVMType();
    auto varrayPtr = irBuilder.CreateEntryAlloca(varrayType, nullptr, "varray");

    for (size_t i = 0; i < varrayChirType->GetSize(); ++i) {
        auto indexName = "varray.idx" + std::to_string(i) + "E";
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
        auto elementPtr =
            irBuilder.CreateGEP(varrayType, varrayPtr, {irBuilder.getInt64(0), irBuilder.getInt64(i)}, indexName);
#endif
        auto cGValue = (irBuilder.GetCGModule() | varray.GetOperand(i));
        auto& cgCtx = irBuilder.GetCGContext();
        auto& cgMod = irBuilder.GetCGModule();
        auto elementPtrCGType =
            CGType::GetOrCreate(cgMod, CGType::GetRefTypeOf(cgCtx.GetCHIRBuilder(), *varrayChirType->GetElementType()));
        (void)irBuilder.CreateStore(*cGValue, CGValue(elementPtr, elementPtrCGType));
    }
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    return varrayPtr;
#endif
}

llvm::Value* CodeGen::GenerateVArrayBuilder(IRBuilder2& irBuilder, const CHIR::VArrayBuilder& varrayBuilder)
{
    auto& cgMod = irBuilder.GetCGModule();
    auto varrayType = StaticCast<CHIR::VArrayType*>(varrayBuilder.GetResult()->GetType());
    auto varrayLen = (cgMod | varrayBuilder.GetSize())->GetRawValue();

    auto item = DynamicCast<CHIR::LocalVar*>(varrayBuilder.GetItem());
    CODEC_NULLPTR_CHECK(item);
    bool isInitedByItem = item && !item->GetExpr()->IsConstantNull();
    if (!isInitedByItem) {
        // VArrayBuilder(size, nullptr, initLambda: Class-$Auto_Env_Base_XXXX)
        auto autoEnvOfInitFunc = varrayBuilder.GetInitFunc();
        auto autoEnvType = DeRef(*autoEnvOfInitFunc->GetType());
        CODEC_ASSERT(autoEnvType->IsAutoEnvBase());
        auto cgValue = (cgMod | autoEnvOfInitFunc);
        return irBuilder.VArrayInitedByLambda(varrayLen, *cgValue, *varrayType);
    } else {
        // VArrayBuilder(size, value, initLambda: nullptr)
        auto cgValue = (cgMod | item);
        return irBuilder.VArrayInitedByItem(varrayLen, *cgValue, *varrayType);
    }
}
