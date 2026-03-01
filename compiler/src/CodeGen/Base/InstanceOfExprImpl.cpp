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
 * This file implements codegen for CHIR InstanceOf.
 */

#include "Base/InstanceOfImpl.h"

#include "CGModule.h"
#include "IRBuilder.h"
#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/Type/ClassDef.h"
#include "Codira/CHIR/Type/Type.h"
#include "Codira/CHIR/Value.h"

using namespace Codira;
using namespace CodeGen;

llvm::Value* CodeGen::GenerateInstanceOf(IRBuilder2& irBuilder, const CHIR::InstanceOf& instanceOf)
{
    // Match pattern: type match.
    auto object = instanceOf.GetObject();
    auto targetCHIRType = instanceOf.GetType();
    auto targetTi = irBuilder.CreateTypeInfo(*targetCHIRType);
    auto objectCHIRType = DeRef(*object->GetType());
    auto objectVal = *(irBuilder.GetCGModule() | object);
    if (objectCHIRType->IsAny() || objectCHIRType->IsGeneric()) {
        auto instanceTi = irBuilder.GetTypeInfoFromObject(*objectVal);
        auto typeKind = irBuilder.GetTypeKindFromTypeInfo(instanceTi);
        auto isTuple = irBuilder.CreateICmpEQ(typeKind, irBuilder.getInt8(static_cast<uint8_t>(UGTypeKind::UG_TUPLE)));
        auto [isTupleBB, nonTupleBB, endBB] =
            Vec2Tuple<3>(irBuilder.CreateAndInsertBasicBlocks({"isTuple", "nonTuple", "end"}));
        irBuilder.CreateCondBr(isTuple, isTupleBB, nonTupleBB);

        irBuilder.SetInsertPoint(isTupleBB);
        auto nullPtr = llvm::Constant::getNullValue(irBuilder.getInt8PtrTy());
        auto isTupleRet = irBuilder.CallIntrinsicIsTupleTypeOf({*objectVal, nullPtr, targetTi});
        irBuilder.CreateBr(endBB);

        irBuilder.SetInsertPoint(nonTupleBB);
        auto nonTupleRet = irBuilder.CallIntrinsicIsSubtype({instanceTi, targetTi});
        irBuilder.CreateBr(endBB);

        irBuilder.SetInsertPoint(endBB);
        auto phi = irBuilder.CreatePHI(irBuilder.getInt1Ty(), 2U);
        phi->addIncoming(isTupleRet, isTupleBB);
        phi->addIncoming(nonTupleRet, nonTupleBB);
        return phi;
    } else if (objectCHIRType->IsClass()) {
        auto instanceTi = irBuilder.GetTypeInfoFromObject(*objectVal);
        return irBuilder.CallIntrinsicIsSubtype({instanceTi, targetTi});
    } else if (objectCHIRType->IsTuple()) {
        auto i8PtrTy = llvm::Type::getInt8PtrTy(irBuilder.GetLLVMContext());
        auto instanceTi = objectVal.GetCGType()->GetSize().has_value() ? irBuilder.CreateTypeInfo(objectCHIRType)
                                                                       : llvm::Constant::getNullValue(i8PtrTy);
        return irBuilder.CallIntrinsicIsTupleTypeOf({*objectVal, instanceTi, targetTi});
    }
    return irBuilder.CallIntrinsicIsSubtype({irBuilder.CreateTypeInfo(objectCHIRType), targetTi});
}
