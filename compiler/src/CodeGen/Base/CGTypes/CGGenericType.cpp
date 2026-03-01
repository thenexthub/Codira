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

#include "Base/CGTypes/CGGenericType.h"

#include "CGModule.h"

namespace Codira::CodeGen {
llvm::Type* CGGenericType::GenLLVMType()
{
    if (llvmType) {
        return llvmType;
    }
    return CGType::GetRefType(cgMod.GetLLVMContext());
}

void CGGenericType::CalculateSizeAndAlign()
{
    size = std::nullopt;
    align = std::nullopt;
}

void CGGenericType::GenContainedCGTypes()
{
    // No contained types
}

llvm::Constant* CGGenericType::GenUpperBoundsOfGenericType(std::string& uniqueName)
{
    auto p0i8 = llvm::Type::getInt8PtrTy(cgMod.GetLLVMContext());
    if (upperBounds.empty()) {
        return llvm::ConstantPointerNull::get(p0i8);
    }

    std::vector<llvm::Constant*> constants;
    for (auto upperBound : upperBounds) {
        auto cgTypeOfUpperBound = CGType::GetOrCreate(cgMod, DeRef(*upperBound));
        constants.emplace_back(llvm::ConstantExpr::getBitCast(cgTypeOfUpperBound->GetOrCreateTypeInfo(), p0i8));
    }
    auto typeOfUpperBoundsGV = llvm::ArrayType::get(p0i8, constants.size());
    auto typeInfoOfUpperBounds = llvm::cast<llvm::GlobalVariable>(cgMod.GetLLVMModule()->getOrInsertGlobal(
        uniqueName + ".upperBounds", typeOfUpperBoundsGV));
    typeInfoOfUpperBounds->setInitializer(llvm::ConstantArray::get(typeOfUpperBoundsGV, constants));
    typeInfoOfUpperBounds->setLinkage(llvm::GlobalValue::LinkageTypes::PrivateLinkage);
    typeInfoOfUpperBounds->addAttribute(CODETI_UPPER_BOUNDS_ATTR);
    return llvm::ConstantExpr::getBitCast(typeInfoOfUpperBounds, p0i8);
}

llvm::GlobalVariable* CGGenericType::GetOrCreateTypeInfo()
{
    CODEC_ASSERT(chirType.IsGeneric());
    auto genericTypeName = StaticCast<const CHIR::GenericType&>(chirType).GetSrcCodeIdentifier();
    upperBounds = StaticCast<const CHIR::GenericType&>(chirType).GetUpperBounds();
    std::string uniqueName = cgMod.GetCGContext().GetGenericTypeUniqueName(genericTypeName, upperBounds);
    if (auto found = cgMod.GetLLVMModule()->getNamedGlobal(uniqueName + ".ti"); found) {
        return found;
    }

    auto genericTypeInfoType = GetOrCreateGenericTypeInfoType(cgMod.GetLLVMContext());
    auto genericTypeInfo = llvm::cast<llvm::GlobalVariable>(
        cgMod.GetLLVMModule()->getOrInsertGlobal(uniqueName + ".ti", genericTypeInfoType));
    unsigned typeInfoKind = UGTypeKind::UG_GENERIC;
    std::vector<llvm::Constant*> typeInfoVec(GENERIC_TYPE_INFO_FIELDS_NUM);
    typeInfoVec[static_cast<size_t>(GENERIC_TYPEINFO_NAME)] =
        cgMod.GenerateTypeNameConstantString(genericTypeName, false);
    typeInfoVec[static_cast<size_t>(GENERIC_TYPEINFO_TYPE_KIND)] =
        llvm::ConstantInt::get(llvm::Type::getInt8Ty(cgMod.GetLLVMContext()), typeInfoKind);
    typeInfoVec[static_cast<size_t>(GENERIC_TYPEINFO_UPPERBOUNDS_NUM)] =
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(cgMod.GetLLVMContext()), upperBounds.size());
    typeInfoVec[static_cast<size_t>(GENERIC_TYPEINFO_UPPERBOUNDS)] =
        GenUpperBoundsOfGenericType(uniqueName);
    genericTypeInfo->setInitializer(llvm::ConstantStruct::get(genericTypeInfoType, typeInfoVec));
    genericTypeInfo->setLinkage(llvm::GlobalValue::PrivateLinkage);
    genericTypeInfo->addAttribute(GENERIC_TYPEINFO_ATTR);
    return genericTypeInfo;
}
} // namespace Codira::CodeGen
