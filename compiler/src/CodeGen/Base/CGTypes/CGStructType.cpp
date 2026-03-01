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

#include "Base/CGTypes/CGStructType.h"

#include "Base/CGTypes/CGTupleType.h"
#include "CGContext.h"
#include "CGModule.h"
#include "Utils/CGUtils.h"
#include "Codira/CHIR/Type/StructDef.h"
#include "Codira/Option/Option.h"

namespace Codira::CodeGen {
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
llvm::Type* CGStructType::GenLLVMType()
{
    auto& llvmCtx = cgCtx.GetLLVMContext();
    auto& structType = StaticCast<const CHIR::StructType&>(chirType);

    if (!IsSized()) {
        return llvm::Type::getInt8Ty(llvmCtx);
    }

    auto typeName = STRUCT_TYPE_PREFIX;
    if (auto defType = structType.GetStructDef()->GetType();
        &chirType != defType && CGType::GetOrCreate(cgMod, defType)->GetSize()) {
        typeName += GetTypeQualifiedName(*defType);
    } else {
        typeName += GetTypeQualifiedName(structType);
    }

    llvmType = llvm::StructType::getTypeByName(llvmCtx, typeName);
    if (llvmType && cgCtx.IsGeneratedStructType(typeName)) {
        layoutType = llvm::cast<llvm::StructType>(llvmType);
        return llvmType;
    } else if (!llvmType) {
        llvmType =  llvm::StructType::create(llvmCtx, typeName);
    }
    layoutType = llvm::cast<llvm::StructType>(llvmType);
    cgCtx.AddGeneratedStructType(typeName);

    std::vector<llvm::Type*> fieldTypes;
    std::vector<size_t> litStructPtrIdx;
    size_t idx = 0;
    auto& nonConstStructType = const_cast<CHIR::StructType&>(structType);
    const auto memberVarTypes = nonConstStructType.GetInstantiatedMemberTys(cgMod.GetCGContext().GetCHIRBuilder());
    for (auto& memberVarType : memberVarTypes) {
        auto cgType = CGType::GetOrCreate(cgMod, memberVarType);
        auto& fieldType = fieldTypes.emplace_back(cgType->GetLLVMType());
        if (IsCFunc(*memberVarType) && IsLitStructPtrType(fieldType)) {
            litStructPtrIdx.emplace_back(idx);
        }
        idx++;
    }
    SetStructTypeBody(llvm::cast<llvm::StructType>(llvmType), fieldTypes);

    for (auto i : litStructPtrIdx) {
        fieldTypes[i] = CGType::GetOrCreate(cgMod, memberVarTypes[i])->GetLLVMType();
    }

    SetStructTypeBody(llvm::cast<llvm::StructType>(llvmType), fieldTypes);

    return llvmType;
}
#endif

void CGStructType::GenContainedCGTypes()
{
    auto& structType = StaticCast<const CHIR::StructType&>(chirType);
    auto& nonConstStructType = const_cast<CHIR::StructType&>(structType);
    for (auto& type : nonConstStructType.GetInstantiatedMemberTys(cgMod.GetCGContext().GetCHIRBuilder())) {
        (void)containedCGTypes.emplace_back(CGType::GetOrCreate(cgMod, type));
    }
}

llvm::Constant* CGStructType::GenFieldsNumOfTypeInfo()
{
    auto structDef = StaticCast<const CHIR::StructType&>(chirType).GetStructDef();
    return llvm::ConstantInt::get(llvm::Type::getInt16Ty(cgMod.GetLLVMContext()), structDef->GetAllInstanceVarNum());
}

void CGStructType::CalculateSizeAndAlign()
{
    if (auto structType = llvm::dyn_cast<llvm::StructType>(llvmType)) {
        auto layOut = cgMod.GetLLVMModule()->getDataLayout();
        size = layOut.getTypeAllocSize(structType);
        align = layOut.getABITypeAlignment(structType);
    }
}

llvm::Constant* CGStructType::GenFieldsOfTypeInfo()
{
    auto& structType = StaticCast<const CHIR::StructType&>(chirType);
    auto& nonConstStructType = const_cast<CHIR::StructType&>(structType);
    auto fieldConstants = GenTypeInfoConstantVectorForTypes(cgMod, nonConstStructType.GetInstantiatedMemberTys(cgMod.GetCGContext().GetCHIRBuilder()));
    return GenTypeInfoArray(cgMod, CGType::GetNameOfTypeInfoGV(chirType) + ".fields", fieldConstants, CODETI_FIELDS_ATTR);
}

} // namespace Codira::CodeGen
