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

#include "Base/CGTypes/CGCPointerType.h"

#include "CGModule.h"

namespace Codira::CodeGen {

llvm::Type* CGCPointerType::GenLLVMType()
{
    if (llvmType) {
        return llvmType;
    }
    auto& llvmCtx = cgCtx.GetLLVMContext();
    llvmType = llvm::Type::getInt8PtrTy(llvmCtx);

    layoutType = llvm::StructType::getTypeByName(llvmCtx, "CPointer.Type");
    if (!layoutType) {
        layoutType = llvm::StructType::create(llvmCtx, {llvmType}, "CPointer.Type");
    }
    return llvmType;
}

void CGCPointerType::GenContainedCGTypes()
{
    CODEC_ASSERT(chirType.GetTypeArgs().size() == 1);
    containedCGTypes.emplace_back(CGType::GetInt8CGType(cgMod));
}

llvm::Constant* CGCPointerType::GenSuperOfTypeInfo()
{
    auto cpointerElementType = DeRef(*static_cast<const CHIR::CPointerType&>(chirType).GetElementType());
    auto ti = CGType::GetOrCreate(cgMod, cpointerElementType)->GetOrCreateTypeInfo();
    return llvm::ConstantExpr::getBitCast(ti, CGType::GetOrCreateTypeInfoPtrType(cgMod.GetLLVMContext()));
}

llvm::Constant* CGCPointerType::GenTypeArgsNumOfTypeInfo()
{
    return llvm::ConstantInt::get(llvm::Type::getInt8Ty(cgMod.GetLLVMContext()), 1U);
}

llvm::Constant* CGCPointerType::GenTypeArgsOfTypeInfo()
{
    auto genericArg = StaticCast<const CHIR::CPointerType&>(chirType).GetElementType();
    auto typeInfoPtrTy = CGType::GetOrCreateTypeInfoPtrType(cgMod.GetLLVMContext());
    auto p0i8 = llvm::Type::getInt8PtrTy(cgMod.GetLLVMContext());

    auto elemCGType = CGType::GetOrCreate(cgMod, DeRef(*genericArg));
    std::vector<llvm::Constant*> constants{elemCGType->GetOrCreateTypeInfo()};
    if (elemCGType->IsStaticGI()) {
        cgCtx.AddDependentPartialOrderOfTypes(constants[0], this->typeInfo);
    }

    auto typeOfGenericArgsGV = llvm::ArrayType::get(typeInfoPtrTy, constants.size());
    auto typeInfoOfGenericArgs = llvm::cast<llvm::GlobalVariable>(cgMod.GetLLVMModule()->getOrInsertGlobal(
        CGType::GetNameOfTypeInfoGV(chirType) + ".typeArgs", typeOfGenericArgsGV));
    typeInfoOfGenericArgs->setInitializer(llvm::ConstantArray::get(typeOfGenericArgsGV, constants));
    typeInfoOfGenericArgs->setLinkage(llvm::GlobalValue::LinkageTypes::PrivateLinkage);
    typeInfoOfGenericArgs->addAttribute(CODETI_TYPE_ARGS_ATTR);
    typeInfoOfGenericArgs->setConstant(true);
    typeInfoOfGenericArgs->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
    return llvm::ConstantExpr::getBitCast(typeInfoOfGenericArgs, p0i8);
}

void CGCPointerType::CalculateSizeAndAlign()
{
    llvm::DataLayout layOut = cgMod.GetLLVMModule()->getDataLayout();
    size = layOut.getTypeAllocSize(llvmType);
    align = layOut.getABITypeAlignment(llvmType);
}
} // namespace Codira::CodeGen
