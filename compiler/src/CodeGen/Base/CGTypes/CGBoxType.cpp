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

#include "Base/CGTypes/CGBoxType.h"

#include "CGModule.h"

namespace Codira::CodeGen {
CGBoxType::CGBoxType(CGModule& cgMod, CGContext& cgCtx, const CHIR::BoxType& chirType)
    : CGType(cgMod, cgCtx, chirType, CGTypeKind::CG_REF)
{
}

llvm::Type* CGBoxType::GenLLVMType()
{
    if (llvmType) {
        return llvmType;
    }
    return llvm::Type::getInt8Ty(cgCtx.GetLLVMContext());
}

void CGBoxType::GenContainedCGTypes()
{
    auto& boxType = StaticCast<const CHIR::BoxType&>(chirType);
    (void)containedCGTypes.emplace_back(CGType::GetOrCreate(cgMod, boxType.GetBaseType()));
}

void CGBoxType::CalculateSizeAndAlign()
{
    llvm::DataLayout layOut = cgMod.GetLLVMModule()->getDataLayout();
    size = layOut.getTypeAllocSize(llvmType);
    align = layOut.getABITypeAlignment(llvmType);
}
} // namespace Codira::CodeGen
