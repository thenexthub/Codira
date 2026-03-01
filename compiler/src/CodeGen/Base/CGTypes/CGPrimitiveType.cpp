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

#include "Base/CGTypes/CGPrimitiveType.h"

#include "CGModule.h"
#include "Codira/CHIR/CHIRContext.h"

namespace Codira::CodeGen {
llvm::Type* CGPrimitiveType::GenLLVMType()
{
    if (llvmType) {
        return llvmType;
    }

    auto& llvmCtx = cgCtx.GetLLVMContext();
    switch (chirType.GetTypeKind()) {
        case CHIR::Type::TypeKind::TYPE_INT8:
        case CHIR::Type::TypeKind::TYPE_UINT8:
            llvmType = llvm::Type::getInt8Ty(llvmCtx);
            break;
        case CHIR::Type::TypeKind::TYPE_INT16:
        case CHIR::Type::TypeKind::TYPE_UINT16:
            llvmType = llvm::Type::getInt16Ty(llvmCtx);
            break;
        case CHIR::Type::TypeKind::TYPE_INT32:
        case CHIR::Type::TypeKind::TYPE_UINT32:
        case CHIR::Type::TypeKind::TYPE_RUNE:
            llvmType = llvm::Type::getInt32Ty(llvmCtx);
            break;
        case CHIR::Type::TypeKind::TYPE_INT64:
        case CHIR::Type::TypeKind::TYPE_UINT64:
            llvmType = llvm::Type::getInt64Ty(llvmCtx);
            break;
        case CHIR::Type::TypeKind::TYPE_INT_NATIVE:
        case CHIR::Type::TypeKind::TYPE_UINT_NATIVE: {
            auto archType = cgCtx.GetCompileOptions().target.GetArchType();
            if (archType == Triple::ArchType::X86_64 || archType == Triple::ArchType::AARCH64) {
                llvmType = llvm::Type::getInt64Ty(llvmCtx);
                break;
            }
            if (archType == Triple::ArchType::ARM32) {
                llvmType = llvm::Type::getInt32Ty(llvmCtx);
                break;
            }
            CODEC_ASSERT_WITH_MSG(false, "Unsupported ArchType.");
            return nullptr;
        }
        case CHIR::Type::TypeKind::TYPE_FLOAT16:
            llvmType = llvm::Type::getHalfTy(llvmCtx);
            break;
        case CHIR::Type::TypeKind::TYPE_FLOAT32:
            llvmType = llvm::Type::getFloatTy(llvmCtx);
            break;
        case CHIR::Type::TypeKind::TYPE_FLOAT64:
            llvmType = llvm::Type::getDoubleTy(llvmCtx);
            break;
        case CHIR::Type::TypeKind::TYPE_BOOLEAN:
            llvmType = llvm::Type::getInt1Ty(llvmCtx);
            break;
        case CHIR::Type::TypeKind::TYPE_UNIT:
        case CHIR::Type::TypeKind::TYPE_NOTHING:
            llvmType = CGType::GetUnitType(llvmCtx);
            break;
        case CHIR::Type::TypeKind::TYPE_VOID:
            llvmType = llvm::Type::getVoidTy(llvmCtx);
            layoutType = llvm::StructType::getTypeByName(llvmCtx, UNIT_TYPE_STR);
            if (!layoutType) {
                layoutType = llvm::StructType::create(llvmCtx, {}, UNIT_TYPE_STR);
            }
            return llvmType;
        default:
            CODEC_ASSERT_WITH_MSG(false, "Should not reach here: unsupported chir type");
            return nullptr;
    }

    auto layoutName = chirType.ToString() + ".Type";
    layoutType = llvm::StructType::getTypeByName(llvmCtx, layoutName);
    if (!layoutType) {
        layoutType = llvm::StructType::create(llvmCtx, {llvmType}, layoutName);
    }
    return llvmType;
}

void CGPrimitiveType::GenContainedCGTypes()
{
    // For primitive type, no contained types.
}

void CGPrimitiveType::CalculateSizeAndAlign()
{
    if (auto t = DynamicCast<const CHIR::NumericType*>(&chirType)) {
        size= t->GetBitness() / 8U;
    } else if (chirType.IsRune()) {
        size = 4U;
    } else if (chirType.IsBoolean()) {
        size = 1U;
    } else if (chirType.IsUnit() || chirType.IsNothing() || chirType.IsVoid()) {
        size = 0U;
    } else {
        CODEC_ASSERT_WITH_MSG(false, "Unsupported type");
    }
    align = size == 0 ? 1 : size;
}
} // namespace Codira::CodeGen
