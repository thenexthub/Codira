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
 * This file implements codegen for CHIR Allocate.
 */

#include "Base/AllocateImpl.h"

#include "Base/CGTypes/CGClassType.h"
#include "Base/CHIRExprWrapper.h"
#include "IRBuilder.h"
#include "Codira/CHIR/Type/ClassDef.h"

using namespace Codira;
using namespace CodeGen;

llvm::Value* CodeGen::GenerateAllocate(IRBuilder2& irBuilder, const CHIRAllocateWrapper& alloca)
{
    auto allocaType = alloca.GetType();
    if (allocaType->IsClass()) {
        auto classType = StaticCast<CHIR::ClassType*>(allocaType);
        auto ret = irBuilder.CallClassIntrinsicAlloc(*classType);
        if (classType->IsAutoEnv()) {
            CODEC_ASSERT(!classType->IsAutoEnvBase());
            auto payload = irBuilder.GetPayloadFromObject(ret);
            auto methods = classType->GetClassDef()->GetMethods();
            for (size_t idx = 0; idx < methods.size(); ++idx) {
                auto virtualFunc = methods[idx];
                auto function = irBuilder.GetCGModule().GetOrInsertCGFunction(virtualFunc)->GetRawValue();
                auto addr = irBuilder.CreateConstInBoundsGEP1_32(
                    irBuilder.getInt8PtrTy(), payload, static_cast<unsigned>(idx));
                irBuilder.CreateStore(irBuilder.CreateBitCast(function, irBuilder.getInt8PtrTy()), addr);
            }
        }
        return ret;
    } else {
        CODEC_ASSERT_WITH_MSG(!allocaType->IsThis(), "CHIR should not try to allocate memory for `ThisType`.");
        auto cgType = CGType::GetOrCreate(irBuilder.GetCGModule(), allocaType);
        return irBuilder.CreateEntryAlloca(*cgType);
    }
}
